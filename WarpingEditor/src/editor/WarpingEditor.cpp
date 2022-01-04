#include "WarpingEditor.h"
#include "ofGraphics.h"

namespace {
	using DataType = Data::Mesh;
	using MeshType = ofx::mapper::Mesh;
	using IndexType = std::pair<int,int>;
	using PointType = glm::vec2;
}

template<>
std::shared_ptr<MeshType> WarpingEditor::getMeshType(const Data::Mesh &data) const
{
	return data.mesh;
}

template<>
void WarpingEditor::forEachPoint(const Data::Mesh &data, std::function<void(const PointType&, IndexType)> func, bool scale_for_inner_world) const
{
	auto &mesh = *data.mesh;
	for(int r = 0; r <= mesh.getNumRows(); ++r) {
		for(int c = 0; c <= mesh.getNumCols(); ++c) {
			func(glm::vec2(*mesh.getPoint(c, r).v), {c,r});
		}
	}
}

template<>
bool WarpingEditor::isEditablePoint(const Data::Mesh &data, IndexType index) const
{
	return data.interpolator->isSelected(index.first, index.second);
}

template<>
std::shared_ptr<MeshType> WarpingEditor::getIfInside(std::shared_ptr<Data::Mesh> data, const glm::vec2 &pos, float &distance)
{
	glm::vec2 tex_uv{tex_.getTextureData().tex_t, tex_.getTextureData().tex_u};
	auto uv = getScaled(*data->uv_quad, tex_uv);
	auto p = getIn(pos);
	auto &mesh = *data->mesh;
	if(!mesh.isInside(p)) {
		return nullptr;
	}
	glm::vec2 average(0,0);
	float num = mesh.getNumRows()*mesh.getNumCols();
	for(int r = 0; r <= mesh.getNumRows(); ++r) {
		for(int c = 0; c <= mesh.getNumCols(); ++c) {
			average += glm::vec2(*mesh.getPoint(c, r).v);
		}
	}
	distance = glm::distance(pos, average/num);
	return data->mesh;
}


template<>
void WarpingEditor::moveMesh(MeshType &mesh, const glm::vec2 &delta)
{
	glm::vec3 d = {delta/getScale(), 0};
	for(int r = 0; r <= mesh.getNumRows(); ++r) {
		for(int c = 0; c <= mesh.getNumCols(); ++c) {
			*mesh.getPoint(c, r).v += d;
		}
	}
}
template<>
void WarpingEditor::movePoint(MeshType &mesh, IndexType index, const glm::vec2 &delta)
{
	glm::vec3 d = {delta/getScale(), 0};
	*mesh.getPoint(index.first, index.second).v += d;
}


template<>
ofMesh WarpingEditor::makeMeshFromMesh(const DataType &data, const ofColor &color) const
{
	ofMesh ret = data.getMesh(50, {tex_.getTextureData().tex_t, tex_.getTextureData().tex_u});
	auto &colors = ret.getColors();
	colors.assign(colors.size(), color);
	return ret;
}

template<>
ofMesh WarpingEditor::makeWireFromMesh(const DataType &data, const ofColor &color) const
{
	ofMesh ret = data.mesh->getMesh();
	ret.setMode(OF_PRIMITIVE_LINES);
	int rows = data.mesh->getNumRows()+1;
	int cols = data.mesh->getNumCols()+1;
	ret.clearIndices();
	for(int y = 0; y < rows-1; y++) {
		for(int x = 0; x < cols-1; x++) {
			if(y < rows-1) {
				ret.addIndex((x)*rows + y);
				ret.addIndex((x)*rows + y+1);
			}
			if(x < cols-1) {
				ret.addIndex((x)*rows + y);
				ret.addIndex((x+1)*rows + y);
			}
		}
	}
	return ret;
}
