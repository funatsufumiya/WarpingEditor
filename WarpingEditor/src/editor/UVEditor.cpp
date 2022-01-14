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
	auto data = Data::shared();
	using namespace ImGui;
	if(Begin("UV")) {
		for(auto &&mesh : data.getMesh()) {
			if(data.isEditable(mesh.second)) {
				if(TreeNode(mesh.first.c_str())) {
					auto m = getMeshType(*mesh.second);
					const auto names = std::vector<std::string>{"lt", "rt", "lb", "rb"};
					float v_min[2] = {0,0};
					float v_max[2] = {1,1};
					for(int i = 0; i < m->size(); ++i) {
						DragFloatNAs(names[i], &m->operator[](i).x, 2, v_min, v_max, nullptr, nullptr, {
							{"px", {
								{glm::ivec2{0, tex_.getWidth()}, 1, "%d"},
								{glm::ivec2{0, tex_.getHeight()}, 1, "%d"}
							}},
							{"%", {
								{glm::vec2{0, 100}, 0.1f, "%.02f%"}
							}},
							{"rate", {
								{glm::vec2{0, 1}, 0.001f, "%.03f%"}
							}},
						});
					}
					TreePop();
				}
			}
			else {
				Text("%s", mesh.first.c_str());
			}
		}
	}
	End();
}
