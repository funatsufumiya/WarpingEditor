#pragma once

#include "ofxEditorFrame.h"
#include "Models.h"
#include "ofTexture.h"
#include "GuiFunc.h"
#include "imgui.h"
#include "imgui_internal.h"

class EditorBase : public ofxEditorFrame
{
public:
	void setup();
	virtual void update();
	virtual void draw() const{}
	virtual void gui() {}

	void setTexture(ofTexture tex) { tex_ = tex; }

	void handleMouse(const ofxEditorFrame::MouseEventArg &arg) { mouse_.set(arg); }
	void setEnableViewportEditByMouse(bool enable) { is_viewport_editable_by_mouse_ = enable; }
	void setEnableMeshEditByMouse(bool enable) { is_mesh_editable_by_mouse_ = enable; }
	
	virtual bool isPreventMeshInterpolation() const { return false; }

	virtual void moveSelectedOnScreenScale(const glm::vec2 &delta){}

protected:
	ofTexture tex_;

	bool is_viewport_editable_by_mouse_=true;
	bool is_mesh_editable_by_mouse_=true;

	class MouseEvent : public ofxEditorFrame::MouseEventArg {
	public:
		bool isFrameNew() const { return is_frame_new_; }
		void set(const ofxEditorFrame::MouseEventArg &src) {
			*(ofxEditorFrame::MouseEventArg*)(this) = src;
			has_new_frame_ = true;
		}
		void update() {
			is_frame_new_ = has_new_frame_;
			has_new_frame_ = false;
		}
	private:
		bool is_frame_new_=false, has_new_frame_=false;
	} mouse_;
	virtual void procNewMouseEvent(const MouseEvent &mouse){}
};

template<typename MeshType, typename IndexType, typename PointType=glm::vec2>
class Editor : public EditorBase
{
public:
	virtual void draw() const override;
	
	void setEnabledHoveringUneditablePoint(bool enable) { is_enabled_hovering_uneditable_point_ = enable; }
	void moveSelectedOnScreenScale(const glm::vec2 &delta) override { moveSelected(delta/getScale()); }

protected:
	void procNewMouseEvent(const MouseEvent &mouse) override;

	bool is_enabled_hovering_uneditable_point_=false;
	float mouse_near_distance_ = 10;
	struct OpHover {
		std::weak_ptr<MeshType> mesh;
		std::pair<std::weak_ptr<MeshType>, IndexType> point;
	} op_hover_;
	struct OpSelection {
		std::set<std::weak_ptr<MeshType>, std::owner_less<std::weak_ptr<MeshType>>> mesh;
		std::map<std::weak_ptr<MeshType>, std::set<IndexType>, std::owner_less<std::weak_ptr<MeshType>>> point;
		bool is_grabbing = false;
	} op_selection_;
	bool isOpAdd() const;
	bool isOpAlt() const;
	bool isOpDefault() const;
	
	void drawMesh() const;
	void drawWire() const;
	void drawPoint(bool only_editable_point) const;
	
	OpHover getHover(const glm::vec2 &screen_pos, bool only_editable_point);
	OpSelection updateSelection(const OpSelection &selection, const OpHover &hover);
	void moveSelected(const glm::vec2 &delta);
	virtual void moveMesh(MeshType &mesh, const glm::vec2 &delta) {}
	virtual void movePoint(MeshType &mesh, IndexType index, const glm::vec2 &delta){}
	void moveMeshOnScreenScale(MeshType &mesh, const glm::vec2 &delta) { moveMesh(mesh, delta/getScale()); }
	void movePointOnScreenScale(MeshType &mesh, IndexType index, const glm::vec2 &delta) { movePoint(mesh, delta/getScale()); }
	
	virtual std::shared_ptr<MeshType> getMeshType(const Data::Mesh &data) const { return nullptr; }
	
	virtual bool isEditableMesh(const Data::Mesh &data) const;
	virtual bool isEditablePoint(const Data::Mesh &data, IndexType index) const { return true; }
	virtual bool isHoveredMesh(const Data::Mesh &data) const;
	virtual bool isHoveredPoint(const Data::Mesh &data, IndexType index) const;
	virtual bool isSelectedMesh(const Data::Mesh &data) const;
	virtual bool isSelectedPoint(const Data::Mesh &data, IndexType index) const;
	
	virtual void forEachMesh(std::function<void(std::shared_ptr<Data::Mesh>)> func) const;
	virtual void forEachPoint(const Data::Mesh &data, std::function<void(const PointType&, IndexType)> func, bool scale_for_inner_world=true) const {}
	
	virtual ofMesh makeMeshFromMesh(const Data::Mesh &mesh, const ofColor &color) const { return ofMesh(); }
	virtual ofMesh makeWireFromMesh(const Data::Mesh &mesh, const ofColor &color) const { return ofMesh(); }
	ofMesh makeMeshFromPoint(const PointType &point, const ofColor &color, float point_size) const;
	virtual ofMesh makeBackground() const { return ofMesh(); }

	virtual std::pair<std::weak_ptr<MeshType>, IndexType> getNearestPoint(std::shared_ptr<Data::Mesh> data, const glm::vec2 &pos, float &distance2, bool filter_by_if_editable=true);
	virtual std::shared_ptr<MeshType> getIfInside(std::shared_ptr<Data::Mesh> data, const glm::vec2 &pos, float &distance) { return nullptr; }
	
	std::pair<bool, glm::vec2> gui2DPanel(const std::string &label, const float v_min[2], const float v_max[2], const std::vector<std::pair<std::string, std::vector<ImGui::DragScalarAsParam>>> &params) const;
};

template<typename MeshType, typename IndexType, typename PointType>
inline bool Editor<MeshType, IndexType, PointType>::isOpAdd() const
{
	return ImGui::IsModKeyDown(ImGuiKeyModFlags_Shift);
}
template<typename MeshType, typename IndexType, typename PointType>
inline bool Editor<MeshType, IndexType, PointType>::isOpAlt() const
{
#ifdef TARGET_OSX
	return ImGui::IsModKeyDown(ImGuiKeyModFlags_Super);
#else
	 return ImGui::IsModKeyDown(ImGuiKeyModFlags_Ctrl);
#endif
}
template<typename MeshType, typename IndexType, typename PointType>
inline bool Editor<MeshType, IndexType, PointType>::isOpDefault() const
{
	return !(isOpAdd() || isOpAlt());
}


template<typename MeshType, typename IndexType, typename PointType>
typename Editor<MeshType, IndexType, PointType>::OpSelection Editor<MeshType, IndexType, PointType>::updateSelection(const OpSelection &selection, const OpHover &hover)
{
	OpSelection ret = selection;
	bool is_grabbing = false;
	if(hover.point.first.expired()) {
		ret.point.clear();
	}
	else {
		is_grabbing = true;
		auto &point = ret.point[hover.point.first];
		auto result = point.insert(hover.point.second);
		bool is_new = result.second;
		if(!is_new) {
			if(isOpAlt()) {
				point.erase(result.first);
				is_grabbing = false;
			}
		}
		else if(isOpDefault()) {
			ret.point.clear();
			ret.point.insert(std::make_pair(hover.point.first, std::set<IndexType>{hover.point.second}));
		}
	}
	if(hover.mesh.expired()) {
		ret.mesh.clear();
	}
	else {
		is_grabbing = true;
		auto result = ret.mesh.insert(hover.mesh);
		bool is_new = result.second;
		if(!is_new) {
			if(isOpAlt()) {
				ret.mesh.erase(result.first);
				is_grabbing = false;
			}
		}
		else if(isOpDefault()) {
			ret.mesh = {hover.mesh};
		}
	}
	ret.is_grabbing = is_grabbing;
	return ret;
}


template<typename MeshType, typename IndexType, typename PointType>
void Editor<MeshType, IndexType, PointType>::moveSelected(const glm::vec2 &delta)
{
	auto &&data = Data::shared();
	if(!op_selection_.point.empty()) {
		for(auto &&qp : op_selection_.point) {
			if(auto ptr = qp.first.lock()) {
				for(auto i : qp.second) {
					movePoint(*ptr, i, delta);
				}
				data.find(ptr).second->setDirty();
			}
		}
	}
	if(!op_selection_.mesh.empty()) {
		for(auto &&q : op_selection_.mesh) {
			if(auto ptr = q.lock()) {
				moveMesh(*ptr, delta);
				data.find(ptr).second->setDirty();
			}
		}
	}
}

template<typename MeshType, typename IndexType, typename PointType>
void Editor<MeshType, IndexType, PointType>::procNewMouseEvent(const MouseEvent &mouse)
{
	bool used = false;
	if(!used && is_mesh_editable_by_mouse_) {
		if(mouse.isPressed(OF_MOUSE_BUTTON_LEFT)) {
			op_selection_ = updateSelection(op_selection_, op_hover_);
			op_hover_ = OpHover();
			used = true;
		}
		else if(!mouse.isPressing(OF_MOUSE_BUTTON_LEFT)) {
			op_hover_ = getHover(mouse.pos, !is_enabled_hovering_uneditable_point_);
		}
		else if(mouse.isDragged(OF_MOUSE_BUTTON_LEFT)) {
			if(op_selection_.is_grabbing) {
				moveSelectedOnScreenScale(mouse.delta);
				used = true;
			}
		}
	}
	if(!used && is_viewport_editable_by_mouse_) {
		if(mouse.isDragged(OF_MOUSE_BUTTON_LEFT)) {
			translate(mouse.delta);
			used = true;
		}
		if(mouse.isScrolledY()) {
			scale(pow(2, mouse.scroll.y/10.f), mouse.pos);
			used = true;
		}
	}
}

template<typename MeshType, typename IndexType, typename PointType>
void Editor<MeshType, IndexType, PointType>::drawMesh() const
{
	ofMesh mesh;
	mesh.setMode(OF_PRIMITIVE_TRIANGLES);
	mesh.append(makeBackground());
	auto meshes = Data::shared().getVisibleMesh();
	for(auto &&mm : meshes) {
		auto m = mm.second;
		if(isSelectedMesh(*m)) {
			mesh.append(makeMeshFromMesh(*m, ofColor::white));
		}
		else if(isHoveredMesh(*m)) {
			mesh.append(makeMeshFromMesh(*m, {ofColor::yellow, 128}));
		}
		else {
			mesh.append(makeMeshFromMesh(*m, {ofColor::gray, 128}));
		}
	};
	tex_.bind();
	mesh.draw();
	tex_.unbind();
}
template<typename MeshType, typename IndexType, typename PointType>
void Editor<MeshType, IndexType, PointType>::drawWire() const
{
	ofMesh mesh;
	mesh.setMode(OF_PRIMITIVE_LINES);
	auto meshes = Data::shared().getVisibleMesh();
	for(auto &&mm : meshes) {
		auto m = mm.second;
		mesh.append(makeWireFromMesh(*m, ofColor::white));
	};
	mesh.draw();
}
template<typename MeshType, typename IndexType, typename PointType>
void Editor<MeshType, IndexType, PointType>::drawPoint(bool only_editable_point) const
{
	float point_size = mouse_near_distance_/getScale();
	ofMesh mesh;
	mesh.setMode(OF_PRIMITIVE_TRIANGLES);
	auto meshes = Data::shared().getVisibleMesh();
	for(auto &&mm : meshes) {
		auto m = mm.second;
		forEachPoint(*m, [&](const PointType &point, IndexType i) {
			if(only_editable_point && !isEditablePoint(*m, i)) {
				return;
			}
			if(isSelectedPoint(*m, i)) {
				mesh.append(makeMeshFromPoint(point, ofColor::white, point_size));
			}
			if(isHoveredPoint(*m, i)) {
				mesh.append(makeMeshFromPoint(point, {ofColor::yellow, 128}, point_size));
			}
			mesh.append(makeMeshFromPoint(point, {ofColor::gray, 128}, point_size));
		});
	};
	mesh.draw();
}

template<typename MeshType, typename IndexType, typename PointType>
void Editor<MeshType, IndexType, PointType>::draw() const
{
	pushMatrix();
	pushScissor();
	drawMesh();
	drawWire();
	drawPoint(!is_enabled_hovering_uneditable_point_);
	popScissor();
	popMatrix();
}

template<typename MeshType, typename IndexType, typename PointType>
std::pair<std::weak_ptr<MeshType>, IndexType> Editor<MeshType, IndexType, PointType>::getNearestPoint(std::shared_ptr<Data::Mesh> data, const glm::vec2 &pos, float &distance2, bool filter_by_if_editable)
{
	std::pair<std::weak_ptr<MeshType>, IndexType> ret;
	distance2 = std::numeric_limits<float>::max();
	auto p = getIn(pos);
	forEachPoint(*data, [&](const PointType &point, IndexType index) {
		if(filter_by_if_editable && !isEditablePoint(*data, index)) {
			return;
		}
		float d2 = glm::distance2(point, p);
		if(d2 < distance2) {
			decltype(ret) tmp{getMeshType(*data), index};
			swap(ret, tmp);
			distance2 = d2;
		}
	});
	return ret;
}

template<typename MeshType, typename IndexType, typename PointType>
typename Editor<MeshType, IndexType, PointType>::OpHover Editor<MeshType, IndexType, PointType>::getHover(const glm::vec2 &screen_pos, bool only_editable_point)
{
	auto &&data = Data::shared();
	auto &&meshes = data.getMesh();
	glm::vec2 tex_uv{tex_.getTextureData().tex_t, tex_.getTextureData().tex_u};
	OpHover ret;
	
	float max_distance = std::numeric_limits<float>::max();
	for(auto &&m : meshes) {
		if(!isEditableMesh(*m.second)) {
			continue;
		}
		const float threshold = pow(mouse_near_distance_/getScale(), 2);
		float distance;
		auto nearest = getNearestPoint(m.second, mouse_.pos, distance, only_editable_point);
		if(max_distance > distance && distance < threshold) {
			ret.point = nearest;
			max_distance = distance;
		}
	}
	max_distance = std::numeric_limits<float>::max();
	if(ret.point.first.expired()) {
		for(auto &&m : meshes) {
			if(!isEditableMesh(*m.second)) {
				continue;
			}
			float distance;
			auto mesh = getIfInside(m.second, mouse_.pos, distance);
			if(mesh && max_distance > distance) {
				ret.mesh = mesh;
				max_distance = distance;
			}
		}
	}
	return ret;
}

template<typename MeshType, typename IndexType, typename PointType>
bool Editor<MeshType, IndexType, PointType>::isEditableMesh(const Data::Mesh &data) const
{
	auto shared = Data::shared();
	auto d = shared.find(data.mesh);
	return shared.isEditable(d.second);
}

template<typename MeshType, typename IndexType, typename PointType>
bool Editor<MeshType, IndexType, PointType>::isHoveredMesh(const Data::Mesh &data) const
{
	return getMeshType(data) == op_hover_.mesh.lock();
}
template<typename MeshType, typename IndexType, typename PointType>
bool Editor<MeshType, IndexType, PointType>::isHoveredPoint(const Data::Mesh &data, IndexType index) const
{
	return getMeshType(data) == op_hover_.point.first.lock() && index == op_hover_.point.second;
}
template<typename MeshType, typename IndexType, typename PointType>
bool Editor<MeshType, IndexType, PointType>::isSelectedMesh(const Data::Mesh &data) const
{
	return any_of(begin(op_selection_.mesh), end(op_selection_.mesh), [&](std::weak_ptr<MeshType> ptr) {
		return getMeshType(data) == ptr.lock();
	});
}
template<typename MeshType, typename IndexType, typename PointType>
bool Editor<MeshType, IndexType, PointType>::isSelectedPoint(const Data::Mesh &data, IndexType index) const
{
	return any_of(begin(op_selection_.point), end(op_selection_.point), [&](std::pair<std::weak_ptr<MeshType>, std::set<IndexType>> ptr) {
		return getMeshType(data) == ptr.first.lock() && ptr.second.find(index) != ptr.second.end();
	});
}

template<typename MeshType, typename IndexType, typename PointType>
void Editor<MeshType, IndexType, PointType>::forEachMesh(std::function<void(std::shared_ptr<Data::Mesh>)> func) const
{
	auto &&data = Data::shared();
	auto &&meshes = data.getMesh();
	for(auto &&m : meshes) {
		func(m.second);
	}
}


template<typename MeshType, typename IndexType, typename PointType>
ofMesh Editor<MeshType, IndexType, PointType>::makeMeshFromPoint(const PointType &point, const ofColor &color, float point_size) const
{
	ofMesh ret;
	ret.setMode(OF_PRIMITIVE_TRIANGLES);
	const int resolution = 16;
	float angle = TWO_PI/(float)resolution;
	for(int i = 0; i < resolution; ++i) {
		ret.addIndex(ret.getNumVertices());
		ret.addVertex(glm::vec3(point+glm::vec2(cos(angle*i), sin(angle*i))*point_size,0));
		ret.addIndex(ret.getNumVertices());
		ret.addVertex(glm::vec3(point,0));
		ret.addIndex(ret.getNumVertices());
		ret.addVertex(glm::vec3(point+glm::vec2(cos(angle*(i+1)), sin(angle*(i+1)))*point_size,0));
		ret.addColor(color);
		ret.addColor(color);
		ret.addColor(color);
	}
	return ret;
}

template<typename MeshType, typename IndexType, typename PointType>
std::pair<bool, glm::vec2> Editor<MeshType, IndexType, PointType>::gui2DPanel(const std::string &label_str, const float v_min[2], const float v_max[2], const std::vector<std::pair<std::string, std::vector<ImGui::DragScalarAsParam>>> &params) const
{
	using namespace ImGui;
	glm::vec2 diff{0,0};
	bool edited = false;
	
	const char *label = label_str.c_str();
	static ImVec2 diff_drag{0,0};
	auto diff_prev = diff_drag;
	if(DragFloatNAs(label, &diff_drag.x, 2, v_min, v_max, nullptr, nullptr, params, ImGuiSliderFlags_NoRoundToFormat)) {
		diff += diff_drag - diff_prev;
		edited |= true;
	}
	if(!IsItemActive()) {
		diff_drag = {0,0};
	}
	
	// getting data_type declared in DragFloatNAs 
	PushID(label);
	auto storage = ImGui::GetStateStorage();
	ImGuiID type_index_id = ImGui::GetCurrentWindow()->GetID("data_type");
	PopID();
	int type_index = storage->GetInt(type_index_id, 0);
	if(type_index >= params.size()) {
		type_index = 0;
	}
	auto p = params[type_index].second;
	assert(!p.empty());
	if(p.size() < 2) {
		p.push_back(p[0]);
	}
	for(int i = 0; i < 2; ++i) {
		ofStringReplace(p[i].format, "%d", "%.0f");
	}

	static ImVec2 diff_button{0,0};
	diff_prev = diff_button;
	if(Drag2DButton(label_str+"_dragbutton", diff_button, {p[0].speed, p[1].speed}, {p[0].format, p[1].format})) {
		for(int i = 0; i < 2; ++i) {
			diff[i] += ofMap(diff_button[i] - diff_prev[i], p[i].getMin<float>(), p[i].getMax<float>(), v_min[i], v_max[i]);
		}
		edited |= true;
	}
	if(!IsItemActive()) {
		diff_button = {0,0};
	}
	return {edited, diff};
}
