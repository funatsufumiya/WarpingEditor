#include "UVEditor.h"
#include "ofGraphics.h"

std::shared_ptr<UVEditor::MeshType> UVEditor::getMeshType(const Data::Mesh &data) const
{
	return data.uv_quad;
}

UVEditor::PointType UVEditor::getPoint(const MeshType &mesh, const IndexType &index) const
{
	return mesh[index];
}

bool UVEditor::isEditablePoint(const Data::Mesh &data, IndexType index) const
{
	return true;
}


void UVEditor::forEachPoint(const Data::Mesh &data, std::function<void(const PointType&, IndexType)> func) const
{
	auto mesh = *getMeshType(data);
	for(int i = 0; i < mesh.size(); ++i) {
		func(mesh[i], i);
	}
}


std::shared_ptr<UVEditor::MeshType> UVEditor::getIfInside(std::shared_ptr<DataType> data, const glm::vec2 &pos, float &distance)
{
	auto uv = *data->uv_quad;
	auto p = getIn(pos);
	if(!inside(uv, p)) {
		return nullptr;
	}
	distance = getDistanceFromCenterOfGravity(uv, p);
	return data->uv_quad;
}



void UVEditor::moveMesh(MeshType &mesh, const glm::vec2 &delta)
{
	mesh = geom::getTranslated(mesh, delta);
}

void UVEditor::movePoint(MeshType &mesh, IndexType index, const glm::vec2 &delta)
{
	mesh[index] += delta;
}


ofMesh UVEditor::makeMeshFromMesh(const DataType &data, const ofColor &color) const
{
	auto mesh = *data.uv_quad;
	ofMesh ret;
	ret.setMode(OF_PRIMITIVE_TRIANGLES);
	auto vert = mesh;
	auto tex_data = tex_.getTextureData();
	auto coord = tex_data.textureTarget == GL_TEXTURE_RECTANGLE_ARB ? mesh : getScaled(mesh, {1/tex_data.tex_w, 1/tex_data.tex_h});
	for(int i = 0; i < mesh.size(); ++i) {
		ret.addTexCoord(coord[i]);
		ret.addVertex(glm::vec3(vert[i],0));
		ret.addColor(color);
	}
	for(auto i : {0,2,1,1,2,3}) {
		ret.addIndex(i);
	}
	return ret;
}


ofMesh UVEditor::makeWireFromMesh(const DataType &data, const ofColor &color) const
{
	auto mesh = *data.uv_quad;
	ofMesh ret;
	ret.setMode(OF_PRIMITIVE_LINES);
	auto vert = mesh;
	auto tex_data = tex_.getTextureData();
	auto coord = tex_data.textureTarget == GL_TEXTURE_RECTANGLE_ARB ? mesh : getScaled(mesh, {1/tex_data.tex_w, 1/tex_data.tex_h});
	for(int i = 0; i < mesh.size(); ++i) {
		ret.addTexCoord(coord[i]);
		ret.addVertex(glm::vec3(vert[i],0));
		ret.addColor(color);
	}
	for(auto i : {0,1,1,3,3,2,2,0}) {
		ret.addIndex(i);
	}
	return ret;
}


ofMesh UVEditor::makeBackground() const
{
	MeshType mesh{tex_.getWidth(),tex_.getHeight()};
	ofMesh ret;
	ret.setMode(OF_PRIMITIVE_TRIANGLES);
	auto vert = mesh;
	auto tex_data = tex_.getTextureData();
	auto coord = tex_data.textureTarget == GL_TEXTURE_RECTANGLE_ARB ? mesh : getScaled(mesh, {1/tex_data.tex_w, 1/tex_data.tex_h});
	for(int i = 0; i < mesh.size(); ++i) {
		ret.addTexCoord(coord[i]);
		ret.addVertex(glm::vec3(vert[i],0));
		ret.addColor(ofColor::gray);
	}
	for(auto i : {0,2,1,1,2,3}) {
		ret.addIndex(i);
	}
	return ret;
}


void UVEditor::gui()
{
	using namespace ImGui;
	auto data = Data::shared();
	const auto names = std::vector<std::string>{"lt", "rt", "lb", "rb"};

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
		auto &p = point.mesh->operator[](point.index);
		return DragFloatNAs(point.label, &p.x, 2, v_min, v_max, nullptr, nullptr, params, ImGuiSliderFlags_NoRoundToFormat);
	};
	auto guiMesh = [&](const GuiMesh &mesh) {
		bool ret = false;
		if(TreeNode(mesh.label.c_str())) {
			for(int i = 0; i < mesh.mesh->size(); ++i) {
				ret |= guiPoint({names[i], mesh.mesh, i});
			}
			TreePop();
		}
		return ret;
	};
	std::vector<GuiMesh> meshes;
	std::vector<GuiPoint> points;

	if(Begin("UV")) {
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
							points.emplace_back(GuiPoint{mesh.first+"/"+names[i], getMeshType(*mesh.second), i});
						}
					}
				}
				EndTabItem();
			}
			if(BeginTabItem("editable")) {
				for(auto &&mesh : data.getEditableMesh()) {
					if(mesh.second) {
						auto m = getMeshType(*mesh.second);
						meshes.push_back({mesh.first, m});
					}
				}
				EndTabItem();
			}
			if(BeginTabItem("all")) {
				for(auto &&mesh : data.getMesh()) {
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
		Checkbox("show", &is_show_grid_);
		Checkbox("snap", &is_snap_enabled_);
		DragFloatNAs("offset", &snap_grid_.position.x, 2, v_min, v_max, nullptr, nullptr, params, ImGuiSliderFlags_NoRoundToFormat);
		glm::vec2 size{snap_grid_.width, snap_grid_.height};
		if(DragFloatNAs("size", &size.x, 2, v_min, v_max, nullptr, nullptr, params, ImGuiSliderFlags_NoRoundToFormat)) {
			snap_grid_.width = size.x;
			snap_grid_.height = size.y;
		}
	}
	End();
}
