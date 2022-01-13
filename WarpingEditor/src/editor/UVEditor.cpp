#include "UVEditor.h"
#include "ofGraphics.h"

namespace {
	using DataType = Data::Mesh;
	using MeshType = geom::Quad;
	using IndexType = int;
	using PointType = glm::vec2;
}

template<>
std::shared_ptr<MeshType> UVEditor::getMeshType(const Data::Mesh &data) const
{
	return data.uv_quad;
}

template<>
bool UVEditor::isEditablePoint(const Data::Mesh &data, IndexType index) const
{
	return true;
}

template<>
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

template<>
std::shared_ptr<MeshType> UVEditor::getIfInside(std::shared_ptr<DataType> data, const glm::vec2 &pos, float &distance)
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


template<>
void UVEditor::moveMesh(MeshType &mesh, const glm::vec2 &delta)
{
	glm::vec2 tex_uv{tex_.getTextureData().tex_t, tex_.getTextureData().tex_u};
	auto d = delta/getScale()/tex_uv;
	mesh = geom::getTranslated(mesh, d);
}
template<>
void UVEditor::movePoint(MeshType &mesh, IndexType index, const glm::vec2 &delta)
{
	glm::vec2 tex_uv{tex_.getTextureData().tex_t, tex_.getTextureData().tex_u};
	auto d = delta/getScale()/tex_uv;
	mesh[index] += d;
}

template<>
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

template<>
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

template<>
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
