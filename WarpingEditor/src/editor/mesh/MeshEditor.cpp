#include "MeshEditor.h"
#include "ofGraphics.h"

std::shared_ptr<MeshEditor::MeshType> MeshEditor::getMeshType(const DataType &data) const
{
	return data.mesh;
}

MeshEditor::PointType MeshEditor::getPoint(const MeshType &mesh, const IndexType &index) const
{
	return *mesh.getPoint(index.first, index.second).v;
}

void MeshEditor::forEachPoint(const DataType &data, std::function<void(const PointType&, IndexType)> func) const
{
	auto &mesh = *data.mesh;
	for(int r = 0; r <= mesh.getNumRows(); ++r) {
		for(int c = 0; c <= mesh.getNumCols(); ++c) {
			func(glm::vec2(*mesh.getPoint(c, r).v), {c,r});
		}
	}
}

bool MeshEditor::isEditablePoint(const DataType &data, IndexType index) const
{
	return data.interpolator->isSelected(index.first, index.second);
}

std::shared_ptr<MeshEditor::MeshType> MeshEditor::getIfInside(std::shared_ptr<DataType> data, const glm::vec2 &pos, float &distance)
{
	auto tex_data = tex_.getTextureData();
	glm::vec2 tex_uv = tex_data.textureTarget == GL_TEXTURE_RECTANGLE_ARB
	? glm::vec2{tex_data.tex_w, tex_data.tex_h}
	: glm::vec2{tex_data.tex_t, tex_data.tex_u};
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

void MeshEditor::moveMesh(MeshType &mesh, const glm::vec2 &delta)
{
	glm::vec3 d = {delta, 0};
	for(int r = 0; r <= mesh.getNumRows(); ++r) {
		for(int c = 0; c <= mesh.getNumCols(); ++c) {
			*mesh.getPoint(c, r).v += d;
		}
	}
}

void MeshEditor::movePoint(MeshType &mesh, IndexType index, const glm::vec2 &delta)
{
	glm::vec3 d = {delta, 0};
	*mesh.getPoint(index.first, index.second).v += d;
}

void MeshEditor::moveSelectedCoord(const glm::vec2 &delta)
{
	auto &&data = *data_;
	if(!op_selection_.point.empty()) {
		for(auto &&qp : op_selection_.point) {
			if(auto ptr = qp.first.lock()) {
				for(auto i : qp.second) {
					movePointCoord(*ptr, i, delta);
				}
				data.find(ptr).second->setDirty();
			}
		}
	}
	if(!op_selection_.mesh.empty()) {
		for(auto &&q : op_selection_.mesh) {
			if(auto ptr = q.lock()) {
				moveMeshCoord(*ptr, delta);
				data.find(ptr).second->setDirty();
			}
		}
	}
}

void MeshEditor::moveMeshCoord(MeshType &mesh, const glm::vec2 &delta)
{
	for(int r = 0; r <= mesh.getNumRows(); ++r) {
		for(int c = 0; c <= mesh.getNumCols(); ++c) {
			*mesh.getPoint(c, r).t += delta;
		}
	}
}
void MeshEditor::movePointCoord(MeshType &mesh, IndexType index, const glm::vec2 &delta)
{
	*mesh.getPoint(index.first, index.second).t += delta;
}


ofMesh MeshEditor::makeMeshFromMesh(const DataType &data, const ofColor &color) const
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

ofMesh MeshEditor::makeWireFromMesh(const DataType &data, const ofColor &color) const
{
	ofMesh ret = data.mesh->getMesh();
	ret.setMode(OF_PRIMITIVE_LINES);
	int rows = data.mesh->getNumRows()+1;
	int cols = data.mesh->getNumCols()+1;
	ret.clearIndices();
	for(int y = 0; y < rows; y++) {
		for(int x = 0; x < cols; x++) {
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


std::set<MeshEditor::IndexType> MeshEditor::getIndices(std::shared_ptr<MeshType> mesh) const
{
	auto selector = data_->find(mesh).second->interpolator;
	auto selected = selector->getSelectedIndices();
	std::set<IndexType> ret;
	for(auto &&s : selected) {
		ret.insert({s[0],s[1]});
	}
	return ret;
}
