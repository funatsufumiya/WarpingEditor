#include "BlendingEditor.h"
#include "Quad.h"

using DataType = BlendingEditor::DataType;
using MeshType = BlendingEditor::MeshType;
using IndexType = BlendingEditor::IndexType;
using PointType = BlendingEditor::PointType;

PointType BlendingEditor::getPoint(const MeshType &mesh, const IndexType &index) const
{
	return mesh.quad[index.first][index.second];
}
void BlendingEditor::moveMesh(MeshType &mesh, const glm::vec2 &delta)
{
	for(int i = 0; i < MeshType::size(); ++i) {
		geom::translate(mesh.quad[i], delta);
	}
}
void BlendingEditor::movePoint(MeshType &mesh, IndexType index, const glm::vec2 &delta)
{
	mesh.quad[index.first][index.second] += delta;
}
std::shared_ptr<MeshType> BlendingEditor::getMeshType(const DataType &data) const
{
	return data.mesh;
}
void BlendingEditor::forEachPoint(const DataType &data, std::function<void(const PointType&, IndexType)> func) const
{
	auto &quads = data.mesh;
	for(int i = 0; i < quads->size(); ++i) {
		auto &&q = quads->quad[i];
		for(int j = 0; j < q.size(); ++j) {
			func(q[j], {i,j});
		}
	}
}
ofMesh BlendingEditor::makeMeshFromMesh(const DataType &data, const ofColor &color) const
{
	const float min_interval = 100;
	float mesh_resample_interval = std::max<float>(min_interval, (getIn({min_interval,0})-getIn({0,0})).x);
	auto viewport = getRegion();
	ofRectangle viewport_in{getIn(viewport.getTopLeft()), getIn(viewport.getBottomRight())};
	auto tex_data = tex_.getTextureData();
	glm::vec2 tex_scale = tex_data.textureTarget == GL_TEXTURE_RECTANGLE_ARB
	? glm::vec2(1,1)
	: glm::vec2(1/tex_data.tex_w, 1/tex_data.tex_h);
	ofMesh ret = data.getMesh(mesh_resample_interval, tex_scale, &viewport_in);
	auto &colors = ret.getColors();
	for(auto &&c : colors) {
		c = c*color;
	}
	return ret;
}
ofMesh BlendingEditor::makeWireFromMesh(const DataType &data, const ofColor &color) const
{
//	return makeMeshFromMesh(data, color);
	auto tex_data = tex_.getTextureData();
	glm::vec2 tex_scale = tex_data.textureTarget == GL_TEXTURE_RECTANGLE_ARB
	? glm::vec2(1,1)
	: glm::vec2(1/tex_data.tex_w, 1/tex_data.tex_h);
	ofMesh ret = data.getWireframe(tex_scale);
	return ret;
}
ofMesh BlendingEditor::makeBackground() const
{
	return ofMesh();
}

std::shared_ptr<MeshType> BlendingEditor::getIfInside(std::shared_ptr<DataType> data, const glm::vec2 &pos, float &distance)
{
	bool found = false;
	auto &quads = data->mesh;
	auto p = getIn(pos);
	for(int i = 0; i < quads->size(); ++i) {
		auto &&q = quads->quad[i];
		if(!inside(q, p)) {
			continue;
		}
		distance = std::min(getDistanceFromCenterOfGravity(q, p), distance);
		found = true;
	}
	return found ? getMeshType(*data) : nullptr;
}

void BlendingEditor::draw() const
{
	pushScissor();
	pushMatrix();
	if(grid_.is_show) {
		drawGrid();
	}
	shader_.begin(tex_);
	{
		ofMesh mesh;
		mesh.setMode(OF_PRIMITIVE_TRIANGLES);
		mesh.append(makeBackground());
		auto meshes = data_->getVisibleData();
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
		}
		mesh.draw();
	}
	shader_.end();
	drawWire();
	drawPoint(!is_enabled_hovering_uneditable_point_);
	popMatrix();
	if(is_enabled_rect_selection_) {
		drawDragRect();
	}
	popScissor();
}

void BlendingEditor::gui()
{
	using namespace ImGui;
	auto &&data = *data_;
	const auto names = std::vector<std::string>{"lt", "rt", "lb", "rb"};
	const auto quad_names = std::vector<std::string>{"outer", "inner"};

	struct GuiMesh {
		std::string label;
		std::shared_ptr<MeshType> mesh;
	};
	struct GuiPoint {
		std::string label;
		std::shared_ptr<MeshType> mesh;
		IndexType index;
	};

	float v_min[2] = {0,0};
	float v_max[2] = {tex_.getWidth(),tex_.getHeight()};
	std::vector<std::pair<std::string, std::vector<ImGui::DragScalarAsParam>>> params{
		{"px", {
			{glm::ivec2{0, tex_.getWidth()}, 1, "%d"},
			{glm::ivec2{0, tex_.getHeight()}, 1, "%d"}
		}},
		{"%", {
			{glm::vec2{0, 100}, 0.1f, "%.02f%%"}
		}},
		{"rate", {
			{glm::vec2{0, 1}, 0.001f, "%.03f"}
		}},
	};

	auto guiPoint = [&](const GuiPoint &point) {
		auto &p = point.mesh->quad[point.index.first][point.index.second];
		return DragFloatNAs(point.label, &p.x, 2, v_min, v_max, nullptr, nullptr, params, ImGuiSliderFlags_NoRoundToFormat);
	};
	auto guiMesh = [&](const GuiMesh &mesh) {
		bool ret = false;
		if(TreeNode(mesh.label.c_str())) {
			auto d = data.find(mesh.mesh).second;
			Checkbox("Left", &d->blend_l); SameLine();
			Checkbox("Right", &d->blend_r); SameLine();
			Checkbox("Top", &d->blend_t); SameLine();
			Checkbox("Bottom", &d->blend_b);
			for(int i = 0; i < mesh.mesh->size(); ++i) {
				if(TreeNode(quad_names[i].c_str())) {
					auto &&q = mesh.mesh->quad[i];
					for(int j = 0; j < q.size(); ++j) {
						ret |= guiPoint({names[j], mesh.mesh, {i,j}});
					}
					TreePop();
				}
			}
			TreePop();
		}
		return ret;
	};
	std::vector<GuiMesh> meshes;
	std::vector<GuiPoint> points;

	if(Begin("Blending")) {
		if(TreeNode("shader")) {
			auto &p = shader_.getParams();
			SliderFloat("blend_power", &p.blend_power, 0, 2);
			SliderFloat("luminance_control", &p.luminance_control, 0, 1);
			SliderFloat3("gamma", &p.gamma[0], 0, 3);
			ColorEdit3("base_color", &p.base_color[0]);
			TreePop();
		}
		if(BeginTabBar("#tab")) {
			if(BeginTabItem("selected")) {
				for(auto &&weak : op_selection_.mesh) {
					auto mesh = data.find(weak.lock());
					if(mesh.second) {
						auto m = getMeshType(*mesh.second);
						meshes.push_back({mesh.first, m});
					}
				}
				for(auto &&point : op_selection_.point) {
					auto mesh = data.find(point.first.lock());
					if(mesh.second) {
						auto m = getMeshType(*mesh.second);
						for(IndexType i : point.second) {
							points.emplace_back(GuiPoint{mesh.first+"/"+quad_names[i.first]+"/"+names[i.second], getMeshType(*mesh.second), i});
						}
					}
				}
				EndTabItem();
			}
			if(BeginTabItem("editable")) {
				for(auto &&mesh : data.getEditableData()) {
					if(mesh.second) {
						auto m = getMeshType(*mesh.second);
						meshes.push_back({mesh.first, m});
					}
				}
				EndTabItem();
			}
			if(BeginTabItem("all")) {
				for(auto &&mesh : data.getData()) {
					if(mesh.second) {
						auto m = getMeshType(*mesh.second);
						meshes.push_back({mesh.first, m});
					}
				}
				EndTabItem();
			}
			EndTabBar();
		};
		for(auto &&m : meshes) {
			if(guiMesh(m)) {
				data.find(m.mesh).second->setDirty();
			}
		}
		for(auto &&p : points) {
			if(guiPoint(p)) {
				data.find(p.mesh).second->setDirty();
			}
		}
	}
	End();
	if(Begin("Move Selected Together")) {
		auto result = gui2DPanel("panel", v_min, v_max, params);
		if(result.first) {
			moveSelected(result.second);
		}
	}
	End();
	if(Begin("Grid")) {
		Checkbox("show", &grid_.is_show);
		Checkbox("snap", &grid_.enabled_snap);
		DragFloatNAs("offset", &grid_.offset.x, 2, v_min, v_max, nullptr, nullptr, params, ImGuiSliderFlags_NoRoundToFormat);
		bool clamp_min[] = {true, true};
		DragFloatNAs("size", &grid_.size.x, 2, v_min, v_max, clamp_min, nullptr, params, ImGuiSliderFlags_NoRoundToFormat);
	}
	End();
}
