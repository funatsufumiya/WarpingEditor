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
void UVEditor::forEachPoint(const Data::Mesh &data, std::function<void(const PointType&, IndexType)> func) const
{
	auto mesh = getMeshType(data);
	for(int i = 0; i < mesh->size(); ++i) {
		func((*mesh)[i], i);
	}
}

template<>
std::pair<std::weak_ptr<MeshType>, IndexType> UVEditor::getNearestPoint(std::shared_ptr<DataType> data, const glm::vec2 &pos, float &distance2)
{
	std::pair<std::weak_ptr<MeshType>, int> ret;
	glm::vec2 tex_uv{tex_.getTextureData().tex_t, tex_.getTextureData().tex_u};
	distance2 = std::numeric_limits<float>::max();
	auto p = getIn(pos);
	auto uv = getScaled(*data->uv_quad, tex_uv);
	for(int i = 0; i < uv.size(); ++i) {
		auto &&point = uv[i];
		float d2 = glm::distance2(point, p);
		if(d2 < distance2) {
			std::pair<std::weak_ptr<MeshType>, IndexType> tmp{data->uv_quad, i};
			swap(ret, tmp);
			distance2 = d2;
		}
	};
	return ret;
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
	auto vert = getScaled(mesh, {tex_.getTextureData().tex_t, tex_.getTextureData().tex_t});
	auto coord = getScaled(mesh, {tex_.getWidth(),tex_.getHeight()});
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
	auto vert = getScaled(mesh, {tex_.getTextureData().tex_t, tex_.getTextureData().tex_t});
	auto coord = getScaled(mesh, {tex_.getWidth(),tex_.getHeight()});
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
ofMesh UVEditor::makeMeshFromPoint(const PointType &point, const ofColor &color, float point_size) const
{
	auto p = point*glm::vec2{tex_.getWidth(),tex_.getHeight()};
	ofMesh ret;
	ret.setMode(OF_PRIMITIVE_TRIANGLES);
	const int resolution = 16;
	float angle = TWO_PI/(float)resolution;
	for(int i = 0; i < resolution; ++i) {
		ret.addVertex(glm::vec3(p+glm::vec2(cos(angle*i), sin(angle*i))*point_size,0));
		ret.addVertex(glm::vec3(p,0));
		ret.addVertex(glm::vec3(p+glm::vec2(cos(angle*(i+1)), sin(angle*(i+1)))*point_size,0));
		ret.addColor(color);
		ret.addColor(color);
		ret.addColor(color);
	}
	return ret;
}

template<>
ofMesh UVEditor::makeBackground() const
{
	MeshType mesh{1,1};
	ofMesh ret;
	ret.setMode(OF_PRIMITIVE_TRIANGLES);
	auto vert = getScaled(mesh, {tex_.getTextureData().tex_t, tex_.getTextureData().tex_t});
	auto coord = getScaled(mesh, {tex_.getWidth(),tex_.getHeight()});
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
