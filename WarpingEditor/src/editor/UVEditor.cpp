#include "UVEditor.h"
#include "ofGraphics.h"

std::shared_ptr<UVEditor::MeshType> UVEditor::getMeshType(const Data::Mesh &data) const
{
	return data.uv_quad;
}


bool UVEditor::isEditablePoint(const Data::Mesh &data, IndexType index) const
{
	return true;
}


void UVEditor::forEachPoint(const Data::Mesh &data, std::function<void(const PointType&, IndexType)> func, bool scale_for_inner_world) const
{
	auto mesh = *getMeshType(data);
	if(scale_for_inner_world) {
		mesh = getScaled(mesh, {tex_.getTextureData().tex_t, tex_.getTextureData().tex_u});
	}
	for(int i = 0; i < mesh.size(); ++i) {
		func(mesh[i], i);
	}
}


std::shared_ptr<UVEditor::MeshType> UVEditor::getIfInside(std::shared_ptr<DataType> data, const glm::vec2 &pos, float &distance)
{
	glm::vec2 tex_uv{tex_.getTextureData().tex_t, tex_.getTextureData().tex_u};
	auto uv = getScaled(*data->uv_quad, tex_uv);
	auto p = getIn(pos);
	if(!inside(uv, p)) {
		return nullptr;
	}
	distance = getDistanceFromCenterOfGravity(uv, p);
	return data->uv_quad;
}



void UVEditor::moveMesh(MeshType &mesh, const glm::vec2 &delta)
{
	glm::vec2 tex_uv{tex_.getTextureData().tex_t, tex_.getTextureData().tex_u};
	auto d = delta/getScale()/tex_uv;
	mesh = geom::getTranslated(mesh, d);
}

void UVEditor::movePoint(MeshType &mesh, IndexType index, const glm::vec2 &delta)
{
	glm::vec2 tex_uv{tex_.getTextureData().tex_t, tex_.getTextureData().tex_u};
	auto d = delta/getScale()/tex_uv;
	mesh[index] += d;
}


ofMesh UVEditor::makeMeshFromMesh(const DataType &data, const ofColor &color) const
{
	auto mesh = *data.uv_quad;
	ofMesh ret;
	ret.setMode(OF_PRIMITIVE_TRIANGLES);
	auto vert = getScaled(mesh, {tex_.getTextureData().tex_t, tex_.getTextureData().tex_u});
	auto coord = getScaled(mesh, {tex_.getTextureData().tex_t, tex_.getTextureData().tex_u});
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
	auto vert = getScaled(mesh, {tex_.getTextureData().tex_t, tex_.getTextureData().tex_u});
	auto coord = getScaled(mesh, {tex_.getTextureData().tex_t, tex_.getTextureData().tex_u});
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
	MeshType mesh{1,1};
	ofMesh ret;
	ret.setMode(OF_PRIMITIVE_TRIANGLES);
	auto vert = getScaled(mesh, {tex_.getTextureData().tex_t, tex_.getTextureData().tex_u});
	auto coord = getScaled(mesh, {tex_.getTextureData().tex_t, tex_.getTextureData().tex_u});
	for(int i = 0; i < mesh.size(); ++i) {
		ret.addTexCoord(coord[i]);
		ret.addVertex(glm::vec3(vert[i],0));
		ret.addColor(ofColor::white);
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

	auto guiPoint = [&](const std::string &label, PointType &p) {
		float v_min[2] = {0,0};
		float v_max[2] = {1,1};
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
		return DragFloatNAs(label, &p.x, 2, v_min, v_max, nullptr, nullptr, params, ImGuiSliderFlags_NoRoundToFormat);
	};
	auto guiMesh = [&](const std::string &label, MeshType &mesh) {
		bool ret = false;
		if(TreeNode(label.c_str())) {
			for(int i = 0; i < mesh.size(); ++i) {
				ret |= guiPoint(names[i], mesh[i]);
			}
			TreePop();
		}
		return ret;
	};
	struct GuiMesh {
		std::string label;
		std::shared_ptr<MeshType> mesh;
	};
	struct GuiPoint {
		std::string label;
		std::shared_ptr<MeshType> mesh;
		IndexType index;
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
			if(guiMesh(m.label, *m.mesh)) {
				data.find(m.mesh).second->setDirty();
			}
		}
		for(auto &&p : points) {
			if(guiPoint(p.label, p.mesh->operator[](p.index))) {
				data.find(p.mesh).second->setDirty();
			}
		}
	}
	End();
}
