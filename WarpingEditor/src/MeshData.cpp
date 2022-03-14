#include "MeshData.h"
#include "imgui.h"
#include "GuiFunc.h"
#include "Icon.h"
#include "ofxBlendScreen.h"
#include "SaveData.h"
#include "AppFunc.h"

#pragma mark - IO

namespace {
template<typename T>
void writeTo(std::ostream& os, const T& t) {
	os.write(reinterpret_cast<const char*>(&t), sizeof(T));
}
template<typename T>
void readFrom(std::istream& is, T& t) {
	is.read(reinterpret_cast<char*>(&t), sizeof(T));
}
}

void DataContainerBase::save(const std::filesystem::path &filepath, glm::vec2 scale) const
{
	ofFile file(filepath, ofFile::WriteOnly);
	pack(file, scale);
	file.close();
}
void DataContainerBase::load(const std::filesystem::path &filepath, glm::vec2 scale)
{
	clear();
	ofFile file(filepath);
	unpack(file, scale);
	file.close();
}

template<typename Data>
void DataContainer<Data>::update() {
	for(auto &&d : data_) {
		d.second->update();
	}
}
template<typename Data>
bool DataContainer<Data>::remove(const std::string &name) {
	auto found = find(data_, name);
	if(found == std::end(data_)) {
		return false;
	}
	data_.erase(found);
	return true;
}
template<typename Data>
bool DataContainer<Data>::remove(const std::shared_ptr<Data> data) {
	auto found = std::find_if(begin(data_), end(data_), [data](const std::pair<std::string, std::shared_ptr<Data>> d) {
		return d.second == data;
	});
	if(found == std::end(data_)) {
		return false;
	}
	data_.erase(found);
	return true;
}

template<typename Data>
bool DataContainer<Data>::isVisible(std::shared_ptr<Data> data) const
{
	return std::any_of(begin(data_), end(data_), [](const std::pair<std::string, std::shared_ptr<Data>> d) {
		return d.second->is_solo;
	}) ? data->is_solo && !data->is_hidden : !data->is_hidden;
}
template<typename Data>
bool DataContainer<Data>::isEditable(std::shared_ptr<Data> data, bool include_hidden) const
{
	return !data->is_locked && (include_hidden || isVisible(data));
}

template<typename Data>
typename DataContainer<Data>::DataMap DataContainer<Data>::getVisibleData() const
{
	DataMap ret;
	auto isVisible = std::any_of(begin(data_), end(data_), [](const std::pair<std::string, std::shared_ptr<Data>> d) {
		return d.second->is_solo;
	}) ? [](std::shared_ptr<Data> d) {
		return d->is_solo && !d->is_hidden;
	} : [](std::shared_ptr<Data> d) {
		return !d->is_hidden;
	};
	for(auto &&d : data_) {
		if(isVisible(d.second)) {
			ret.push_back(d);
		}
	}
	return ret;
}
template<typename Data>
typename DataContainer<Data>::DataMap DataContainer<Data>::getEditableData(bool include_hidden) const
{
	DataMap ret;
	auto isVisible = std::any_of(begin(data_), end(data_), [](const std::pair<std::string, std::shared_ptr<Data>> d) {
		return d.second->is_solo;
	}) ? [](std::shared_ptr<Data> d) {
		return d->is_solo && !d->is_hidden;
	} : [](std::shared_ptr<Data> d) {
		return !d->is_hidden;
	};
	for(auto &&d : data_) {
		if(!d.second->is_locked && (include_hidden || isVisible(d.second))) {
			ret.push_back(d);
		}
	}
	return ret;
}

template<typename Data>
void DataContainer<Data>::gui(std::function<bool(DataType&)> is_selected, std::function<void(DataType&, bool)> set_selected, std::function<void()> create_new)
{
	using namespace ImGui;
	
	bool update_mesh_name = false;
	std::weak_ptr<DataType> mesh_delete;
	
	const char *dnd_id = "meshD&D";
	auto &meshes = getData();
	int move_from = -1, move_to = -1;
	auto cursor_start = GetCursorPos();
	std::map<std::string, std::shared_ptr<DataType>> selected_meshes = [is_selected, meshes]() {
		std::map<std::string, std::shared_ptr<DataType>> ret;
		for(auto &&m : meshes) {
			if(is_selected(*m.second)) {
				ret.insert(m);
			}
		}
		return ret;
	}();
	auto clearSelection = [&]() {
		for(auto &&m : meshes) {
			set_selected(*m.second, false);
		}
		selected_meshes.clear();
	};
	auto addSelection = [&](std::pair<std::string, std::shared_ptr<DataType>> p) {
		set_selected(*p.second, true);
		selected_meshes.insert(p);
	};
	auto removeSelection = [&](std::pair<std::string, std::shared_ptr<DataType>> p) {
		set_selected(*p.second, false);
		selected_meshes.erase(p.first);
	};
	bool is_any_solo = std::any_of(begin(meshes), end(meshes), [](const std::pair<std::string, std::shared_ptr<DataType>> p) {
		return p.second->is_solo;
	});
	auto switches = [is_any_solo](std::shared_ptr<DataType> m, bool &hidden, bool &locked, bool &solo, bool *editable) {
		bool ret = false;
		ret |= ToggleButton("##hide", hidden, Icon::HIDE, Icon::SHOW, {17,17}, 0);	SameLine();
		ret |= ToggleButton("##lock", locked, Icon::LOCK, Icon::UNLOCK, {17,17}, 0);	SameLine();
		ret |= ToggleButton("##solo", solo, Icon::FLAG, Icon::BLANK, {17,17}, 0);
		if(editable) {
			*editable = (!is_any_solo || solo) && !hidden && !locked;
		}
		return ret;
	};
	for(int i = 0; i < meshes.size(); ++i) {
		auto &&m = meshes[i];
		PushID(m.first.c_str());
		bool editable = false;
		switches(m.second, m.second->is_hidden, m.second->is_locked, m.second->is_solo, &editable);
		if(!editable) {
			set_selected(*m.second, false);
		}
		SameLine();
		if(mesh_edit_.second.lock() == m.second) {
			if(need_keyboard_focus_) SetKeyboardFocusHere();
			need_keyboard_focus_ = false;
			update_mesh_name = EditText("###change name", mesh_name_buf_, 256, ImGuiInputTextFlags_EnterReturnsTrue|ImGuiInputTextFlags_AutoSelectAll);
		}
		else {
			bool selected = is_selected(*m.second);
			if(Selectable(m.first.c_str(), &selected)) {
				if(app::isOpDefault()) {
					clearSelection();
					addSelection(m);
				}
				if(app::isOpAlt()) {
					selected ? addSelection(m) : removeSelection(m);
				}
				if(app::isOpAdd() && selected) {
					addSelection(m);
				}
			}
			if(IsItemClicked(ImGuiPopupFlags_MouseButtonLeft)) {
				mesh_edit_.second.reset();
			}
			else if(IsItemClicked(ImGuiPopupFlags_MouseButtonMiddle)
					|| (IsItemClicked(ImGuiPopupFlags_MouseButtonRight) && IsModKeyDown(ImGuiKeyModFlags_Alt))
				) {
				mesh_delete = m.second;
			}
			else if(IsItemClicked(ImGuiPopupFlags_MouseButtonRight)) {
				mesh_name_buf_ = m.first;
				mesh_edit_ = m;
				need_keyboard_focus_ = true;
			}
		}
		PopID();
		
		ImGuiDragDropFlags src_flags = 0;
		src_flags |= ImGuiDragDropFlags_SourceNoDisableHover;
		src_flags |= ImGuiDragDropFlags_SourceNoHoldToOpenOthers;
		//src_flags |= ImGuiDragDropFlags_SourceNoPreviewTooltip; // Hide the tooltip
		if (ImGui::BeginDragDropSource(src_flags)) {
			if (!(src_flags & ImGuiDragDropFlags_SourceNoPreviewTooltip))
				ImGui::Text("Moving \"%s\"", m.first.c_str());
			ImGui::SetDragDropPayload(dnd_id, &i, sizeof(int));
			ImGui::EndDragDropSource();
		}
		if (ImGui::BeginDragDropTarget()) {
			ImGuiDragDropFlags target_flags = 0;
			target_flags |= ImGuiDragDropFlags_AcceptBeforeDelivery;
//			target_flags |= ImGuiDragDropFlags_AcceptNoDrawDefaultRect; // Don't display the yellow rectangle
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(dnd_id, target_flags)) {
				move_from = *(const int*)payload->Data;
				move_to = i;
			}
			ImGui::EndDragDropTarget();
		}
	}
	if(update_mesh_name) {
		auto found = find(meshes, mesh_edit_.first);
		assert(found != end(meshes));
		if(mesh_name_buf_ != "" && insert(meshes, {mesh_name_buf_, found->second}).second) {
			meshes.erase(found);
		}
		mesh_edit_.second.reset();
	}
	if(auto mesh_to_delete = mesh_delete.lock()) {
		remove(mesh_to_delete);
	}
	if(Button("create new")) {
		create_new();
	}
	if(mesh_edit_.second.expired() && !selected_meshes.empty()) {
		SameLine();
		if(Button("duplicate selected")) {
			for(auto &&s : selected_meshes) {
				createCopy(s.first, s.second);
			}
		}
	}
	if (move_from != -1 && move_to != -1) {
		swap(meshes[move_to], meshes[move_from]);
		ImGui::SetDragDropPayload(dnd_id, &move_to, sizeof(int));
	}
	if(!selected_meshes.empty()) {
		auto cursor_end = GetCursorPos();
		SetCursorPos(cursor_start);
		if(InvisibleButton("clear selction", GetContentRegionAvail())) {
			clearSelection();
		}
		SetCursorPos(cursor_end);
	}
}

#pragma mark - IO

template<typename Data>
void DataContainer<Data>::pack(std::ostream &stream, const glm::vec2 &scale) const
{
	writeTo(stream, data_.size());
	const int name_alignemt = 4;
	for(auto &&d : data_) {
		auto name = d.first;
		std::size_t name_size = name.size();
		std::size_t pad_size = std::ceil(name_size/(float)name_alignemt) * name_alignemt;
		writeTo(stream, name.size());
		stream.write(name.c_str(), pad_size);
		d.second->pack(stream, scale);
	}
}

template<typename Data>
void DataContainer<Data>::unpack(std::istream &stream, const glm::vec2 &scale)
{
	const int name_alignemt = 4;
	data_.clear();
	std::size_t num;
	readFrom(stream, num);
	while(num-->0) {
		std::size_t name_size;
		readFrom(stream, name_size);
		std::size_t pad_size = std::ceil(name_size/(float)name_alignemt) * name_alignemt;
		std::string name;
		name.resize(pad_size);
		stream.read(const_cast<char*>(name.data()), pad_size);
		name.resize(name_size);
		auto data = std::make_shared<Data>();
		data->unpack(stream, scale);
		insert(data_, {name, data});
	}
}

template<typename Data>
std::pair<std::string, std::shared_ptr<Data>> DataContainer<Data>::createCopy(const std::string &name, std::shared_ptr<DataType> src)
{
	auto d = std::make_shared<Data>();
	*d = *src;
	auto n = name;
	int index = 0;
	while(!insert(data_, {n, d}).second) {
		n = name+ofToString(index++);
	}
	return std::make_pair(n, d);
}

template<typename Data>
bool DataContainer<Data>::isDirtyAny() const
{
	return any_of(begin(data_), end(data_), [](std::pair<std::string, std::shared_ptr<DataType>> d) {
		return d.second->isDirty();
	});
}

// -------------

ofMesh WarpingMesh::getMesh(float resample_min_interval, const glm::vec2 &remap_coord, const ofRectangle *use_area) const {
	if(is_dirty_ || ofIsFloatEqual(cached_resample_interval_, resample_min_interval) || (use_area && *use_area != cached_valid_viewport_)) {
		cache_ = createMesh(resample_min_interval, remap_coord, use_area);
		is_dirty_ = false;
		cached_resample_interval_ = resample_min_interval;
		if(use_area) {
			cached_valid_viewport_ = *use_area;
		}
	}
	return cache_;
}

ofMesh WarpingMesh::createMesh(float resample_min_interval, const glm::vec2 &remap_coord, const ofRectangle *use_area) const
{
	ofMesh ret = ofx::mapper::UpSampler().proc(*mesh, resample_min_interval, use_area);
	auto uv = geom::getScaled(*uv_quad, remap_coord);
	for(auto &t : ret.getTexCoords()) {
		t = geom::rescalePosition(uv, t);
	}
	return ret;
}

std::pair<std::string, std::shared_ptr<WarpingData::DataType>> WarpingData::create(const std::string &name, const glm::ivec2 &num_cells, const ofRectangle &vert_rect, const ofRectangle &coord_rect) {
	std::string n = name;
	int index=0;
	auto d = std::make_shared<DataType>();
	while(!insert(data_, {n, d}).second) {
		n = name+ofToString(index++);
	}
	d->init(num_cells, vert_rect, coord_rect);
	return std::make_pair(n, d);
}

void WarpingData::uvRescale(const glm::vec2 &scale)
{
	for(auto &&d : data_) {
		*d.second->uv_quad = getScaled(*d.second->uv_quad, scale);
	}
}

std::pair<std::string, std::shared_ptr<WarpingData::DataType>> WarpingData::find(std::shared_ptr<UVType> quad)
{
	auto found = std::find_if(begin(data_), end(data_), [quad](const std::pair<std::string, std::shared_ptr<DataType>> d) {
		return d.second->uv_quad == quad;
	});
	if(found == std::end(data_)) {
		return {"", nullptr};
	}
	return *found;
}

std::pair<std::string, std::shared_ptr<WarpingData::DataType>> WarpingData::find(std::shared_ptr<MeshType> mesh)
{
	auto found = std::find_if(begin(data_), end(data_), [mesh](const std::pair<std::string, std::shared_ptr<DataType>> d) {
		return d.second->mesh == mesh;
	});
	if(found == std::end(data_)) {
		return {"", nullptr};
	}
	return *found;
}

#pragma mark - IO


void WarpingData::exportMesh(const std::filesystem::path &filepath, float resample_min_interval, const glm::vec2 &coord_size, bool only_visible) const
{
	getMeshForExport(resample_min_interval, coord_size).save(filepath);
}

ofMesh WarpingData::getMeshForExport(float resample_min_interval, const glm::vec2 &coord_size, bool only_visible) const
{
	ofMesh ret;
	for(auto &&d : only_visible ? getVisibleData() : data_) {
		ret.append(d.second->createMesh(resample_min_interval, coord_size));
	}
	return ret;
}

ofMesh WarpingData::getMesh(float resample_min_interval, const glm::vec2 &coord_size, ofRectangle *viewport, bool only_visible) const
{
	ofMesh ret;
	for(auto &&d : only_visible ? getVisibleData() : data_) {
		ret.append(d.second->getMesh(resample_min_interval, coord_size, viewport));
	}
	return ret;
}

std::pair<std::string, std::shared_ptr<BlendingData::DataType>> BlendingData::create(const std::string &name, const ofRectangle &frame, const float &default_inner_ratio)
{
	std::string n = name;
	int index=0;
	auto d = std::make_shared<DataType>();
	while(!insert(data_, {n, d}).second) {
		n = name+ofToString(index++);
	}
	d->init(frame, default_inner_ratio);
	return std::make_pair(n, d);
}

std::pair<std::string, std::shared_ptr<BlendingData::DataType>> BlendingData::find(std::shared_ptr<MeshType> mesh)
{
	auto found = std::find_if(begin(data_), end(data_), [mesh](const std::pair<std::string, std::shared_ptr<DataType>> d) {
		return d.second->mesh == mesh;
	});
	if(found == std::end(data_)) {
		return {"", nullptr};
	}
	return *found;
}

void BlendingData::exportMesh(const std::filesystem::path &filepath, float resample_min_interval, const glm::vec2 &coord_size, bool only_visible) const
{
	getMeshForExport(resample_min_interval, coord_size).save(filepath);
}

ofMesh BlendingData::getMeshForExport(float resample_min_interval, const glm::vec2 &coord_size, bool only_visible) const
{
	ofMesh ret;
	for(auto &&d : only_visible ? getVisibleData() : data_) {
		ret.append(d.second->createMesh(resample_min_interval, coord_size));
	}
	return ret;
}

ofMesh BlendingData::getMesh(float resample_min_interval, const glm::vec2 &coord_size, ofRectangle *viewport, bool only_visible) const
{
	ofMesh ret;
	for(auto &&d : only_visible ? getVisibleData() : data_) {
		ret.append(d.second->getMesh(resample_min_interval, coord_size, viewport));
	}
	return ret;
}

void BlendingData::pack(std::ostream &stream, const glm::vec2 &scale) const
{
	SaveData::pack(stream, shader_->getParams());
	DataContainer::pack(stream, scale);
	
}
void BlendingData::unpack(std::istream &stream, const glm::vec2 &scale)
{
	SaveData::unpack(stream, shader_->getParams());
	DataContainer::unpack(stream, scale);
}

void BlendingMesh::init(const ofRectangle &frame, float default_inner_ratio)
{
	mesh->quad[0] = frame;
	auto inner = frame;
	inner.scaleFromCenter(default_inner_ratio);
	mesh->quad[1] = inner;
}

ofMesh BlendingMesh::getMesh(float resample_min_interval, const glm::vec2 &remap_coord, const ofRectangle *use_area) const
{
	if(is_dirty_ || !ofIsFloatEqual(cached_resample_interval_, resample_min_interval) || (use_area && *use_area != cached_valid_viewport_)) {
		cache_ = createMesh(resample_min_interval, remap_coord, use_area);
		is_dirty_ = false;
		cached_resample_interval_ = resample_min_interval;
		if(use_area) {
			cached_valid_viewport_ = *use_area;
		}
	}
	return cache_;
}


ofMesh BlendingMesh::createMesh(float resample_min_interval, const glm::vec2 &remap_coord, const ofRectangle *use_area) const
{
	auto outer_uv = getScaled(mesh->quad[0], remap_coord);
	auto src_mesh = ofxBlendScreen::createMesh(mesh->quad[0], mesh->quad[1], outer_uv
											   ,(blend_l?ofxBlendScreen::BLEND_LEFT:0)
											   |(blend_r?ofxBlendScreen::BLEND_RIGHT:0)
											   |(blend_t?ofxBlendScreen::BLEND_TOP:0)
											   |(blend_b?ofxBlendScreen::BLEND_BOTTOM:0)
											   );
	ofx::mapper::Mesh mm;
	mm.init(src_mesh, {3,3});
	ofMesh ret = ofx::mapper::UpSampler().proc(mm, resample_min_interval, use_area);
	auto *v = ret.getVerticesPointer();
	auto *t = ret.getTexCoordsPointer();
	for(int i = 0; i < ret.getNumVertices(); ++i) {
		t[i] = v[i]*remap_coord;
	}
	return ret;
}

ofMesh BlendingMesh::getWireframe(const glm::vec2 &remap_coord, const ofFloatColor &color) const
{
	auto outer_uv = getScaled(mesh->quad[0], remap_coord);
	auto ret = ofxBlendScreen::createMesh(mesh->quad[0], mesh->quad[1], outer_uv);
	ret.getColors().assign(ret.getNumVertices(), color);
	ret.clearIndices();
	auto makeIndexVector = [](std::vector<bool> enabler) {
		std::vector<ofIndexType> ret;
		ret.reserve(enabler.size());
		for(ofIndexType i = 0; i < enabler.size(); ++i) {
			if(enabler[i]) ret.push_back(i);
		}
		return ret;
	};
	auto cols = makeIndexVector({true, blend_l, blend_r, true});
	auto rows = makeIndexVector({true, blend_t, blend_b, true});
	for(int r = 0; r < rows.size(); ++r) {
		for(int c = 0; c < cols.size(); ++c) {
			ofIndexType lt = rows[r]*4+cols[c];
			if(c < cols.size()-1) {
				ofIndexType rt = rows[r]*4+cols[c+1];
				ret.addIndices({lt,rt});
			}
			if(r < rows.size()-1) {
				ofIndexType lb = rows[r+1]*4+cols[c];
				ret.addIndices({lt,lb});
			}
		}
	}
	return ret;
}

#pragma mark - IO


namespace geom {
void pack(const Quad &quad, std::ostream &stream, glm::vec2 scale) {
	for(int i = 0; i < quad.size(); ++i) {
		writeTo(stream, quad.pt[i].x*scale.x);
		writeTo(stream, quad.pt[i].y*scale.y);
	}
}
void unpack(Quad &quad, std::istream &stream, glm::vec2 scale) {
	for(int i = 0; i < quad.size(); ++i) {
		readFrom(stream, quad.pt[i].x); quad.pt[i].x *= scale.x;
		readFrom(stream, quad.pt[i].y); quad.pt[i].y *= scale.y;
	}
}

}
void MeshData::pack(std::ostream &stream, glm::vec2 scale) const
{
	writeTo(stream, is_hidden);
	writeTo(stream, is_locked);
	writeTo(stream, is_solo);
}
void MeshData::unpack(std::istream &stream, glm::vec2 scale)
{
	readFrom(stream, is_hidden);
	readFrom(stream, is_locked);
	readFrom(stream, is_solo);
	setDirty();
}

void WarpingMesh::pack(std::ostream &stream, glm::vec2 scale) const
{
	MeshData::pack(stream, scale);
	geom::pack(*uv_quad, stream, scale);
	mesh->pack(stream, interpolator.get());
}
void WarpingMesh::unpack(std::istream &stream, glm::vec2 scale)
{
	MeshData::unpack(stream, scale);
	geom::unpack(*uv_quad, stream, scale);
	mesh->unpack(stream, interpolator.get());
}


void BlendingMesh::pack(std::ostream &stream, glm::vec2 scale) const
{
	MeshData::pack(stream, scale);
	writeTo(stream, blend_l);
	writeTo(stream, blend_r);
	writeTo(stream, blend_t);
	writeTo(stream, blend_b);
	for(int i = 0; i < MeshType::size(); ++i) {
		geom::pack(mesh->quad[i], stream, scale);
	}
}
void BlendingMesh::unpack(std::istream &stream, glm::vec2 scale)
{
	MeshData::unpack(stream, scale);
	readFrom(stream, blend_l);
	readFrom(stream, blend_r);
	readFrom(stream, blend_t);
	readFrom(stream, blend_b);
	for(int i = 0; i < MeshType::size(); ++i) {
		geom::unpack(mesh->quad[i], stream, scale);
	}
}


template class DataContainer<WarpingMesh>;
template class DataContainer<BlendingMesh>;
