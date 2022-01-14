#include "WarpingEditor.h"
#include "ofGraphics.h"

namespace {
ofMesh makeCross(const WarpingEditor::PointType &point, const ofColor &color, float line_length, float stroke_width, float degree)
{
	ofMesh ret;
	ret.setMode(OF_PRIMITIVE_TRIANGLES);
	glm::vec3 p{point, 0};
	ret.addVertex(p);
	float hl = line_length/2.f;
	float hs = stroke_width/2.f;
	for(int i = 0; i < 4; ++i) {
		ofIndexType index = ret.getNumVertices();
		float rot = ofDegToRad(i*90+degree);
		ret.addVertex(glm::vec3(glm::rotate(glm::vec2{hl,hs}, rot),0)+p);
		ret.addVertex(glm::vec3(glm::rotate(glm::vec2{hs,hs}, rot),0)+p);
		ret.addVertex(glm::vec3(glm::rotate(glm::vec2{hs,hl}, rot),0)+p);
		ret.addIndices({
			0,index+1,index,
			0,index+2,index+1,
			0,index+3,index+2,
		});
	}
	ret.addVertex(ret.getVertex(1));
	ret.getColors().assign(ret.getNumVertices(), color);
	
	return ret;
}
bool isCorner(const WarpingEditor::MeshType &mesh, WarpingEditor::IndexType index) {
	int x = index.first, y = index.second;
	return (x==0||x==mesh.getNumCols()) && (y==0||y==mesh.getNumRows());
}
}
std::shared_ptr<WarpingEditor::MeshType> WarpingEditor::getMeshType(const Data::Mesh &data) const
{
	return data.mesh;
}

void WarpingEditor::forEachPoint(const Data::Mesh &data, std::function<void(const PointType&, IndexType)> func, bool scale_for_inner_world) const
{
	auto &mesh = *data.mesh;
	for(int r = 0; r <= mesh.getNumRows(); ++r) {
		for(int c = 0; c <= mesh.getNumCols(); ++c) {
			func(glm::vec2(*mesh.getPoint(c, r).v), {c,r});
		}
	}
}

bool WarpingEditor::isEditablePoint(const Data::Mesh &data, IndexType index) const
{
	return data.interpolator->isSelected(index.first, index.second);
}

std::shared_ptr<WarpingEditor::MeshType> WarpingEditor::getIfInside(std::shared_ptr<Data::Mesh> data, const glm::vec2 &pos, float &distance)
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

void WarpingEditor::moveMesh(MeshType &mesh, const glm::vec2 &delta)
{
	if(mode_ != MODE_MESH) {
		return;
	}
	glm::vec3 d = {delta/getScale(), 0};
	for(int r = 0; r <= mesh.getNumRows(); ++r) {
		for(int c = 0; c <= mesh.getNumCols(); ++c) {
			*mesh.getPoint(c, r).v += d;
		}
	}
}

void WarpingEditor::movePoint(MeshType &mesh, IndexType index, const glm::vec2 &delta)
{
	if(mode_ != MODE_MESH) {
		return;
	}
	glm::vec3 d = {delta/getScale(), 0};
	*mesh.getPoint(index.first, index.second).v += d;
}

ofMesh WarpingEditor::makeMeshFromMesh(const DataType &data, const ofColor &color) const
{
	ofMesh ret = data.getMesh(10, {tex_.getTextureData().tex_t, tex_.getTextureData().tex_u});
	auto &colors = ret.getColors();
	colors.assign(colors.size(), color);
	return ret;
}

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

void WarpingEditor::update()
{
	auto mode_prev = mode_;
	if(ofGetKeyPressed('d')) {
		mode_ = MODE_DIVISION;
		setEnabledHoveringUneditablePoint(true);
	}
	if(ofGetKeyPressed('m')) {
		mode_ = MODE_MESH;
		setEnabledHoveringUneditablePoint(false);
	}
	if(mode_ != mode_prev) {
		op_selection_ = OpSelection();
		op_hover_ = OpHover();
	}
	Editor::update();
	if(mode_ == MODE_DIVISION) {
		if(mouse_.isFrameNew()) {
			if(mouse_.isClicked(OF_MOUSE_BUTTON_LEFT)) {
				auto pos = getIn(mouse_.pos);
				for(auto weak : op_selection_.quad) {
					if(auto data = Data::shared().find(weak.lock())) {
						if(!isEditableMesh(*data)) {
							continue;
						}
						auto mesh = data->mesh;
						glm::vec2 dst_findex;
						glm::vec2 result;
						bool is_row;
						if(mesh->getNearestPointOnLine(pos, dst_findex, result, is_row, mouse_near_distance_/getScale())) {
							int col = dst_findex.x;
							int row = dst_findex.y;
							if(is_row) {
								mesh->divideCol(col, dst_findex.x - col);
								data->interpolator->selectPoint(col+1, row);
							}
							else {
								mesh->divideRow(row, dst_findex.y - row);
								data->interpolator->selectPoint(col, row+1);
							}
						}
						else if(mesh->getIndexOfPoint(pos, dst_findex)) {
							int col = dst_findex.x;
							int row = dst_findex.y;
							mesh->divideCol(col, dst_findex.x - col);
							mesh->divideRow(row, dst_findex.y - row);
							data->interpolator->selectPoint(col+1, row+1);
						}
					}
				}
				for(auto selection : op_selection_.point) {
					if(auto data = Data::shared().find(selection.first.lock())) {
						auto mesh = data->mesh;
						for(auto index : selection.second) {
							if(isCorner(*mesh, index)) {
								continue;
							}
							data->interpolator->togglePoint(index.first, index.second);
						}
					}
				}
				op_selection_.point.clear();
				op_hover_ = getHover(mouse_.pos, !is_enabled_hovering_uneditable_point_);
			}
		}
	}
}


void WarpingEditor::draw() const
{
	switch(mode_) {
		case MODE_MESH:
			Editor::draw();
			return;
		case MODE_DIVISION: {
			pushMatrix();
			pushScissor();
			drawMesh();
			drawWire();
			float point_size = mouse_near_distance_/getScale();
			float cross_size = point_size*4;
			float cross_width = point_size/2.f;
			ofMesh mesh;
			mesh.setMode(OF_PRIMITIVE_TRIANGLES);
			forEachMesh([&](std::shared_ptr<Data::Mesh> m) {
				forEachPoint(*m, [&](const PointType &point, IndexType i) {
					if(isCorner(*m->mesh, i)) {
						mesh.append(makeMeshFromPoint(point, ofColor::black, point_size/2.f));
					}
					else if(isEditablePoint(*m, i)) {
						mesh.append(makeMeshFromPoint(point, {ofColor::white, 128}, point_size));
						if(isHoveredPoint(*m, i)) {
							mesh.append(makeCross(point, ofColor::red, cross_size, cross_width, 45));
						}
					}
					else {
						mesh.append(makeMeshFromPoint(point, {ofColor::green, 128}, point_size/2.f));
						if(isHoveredPoint(*m, i)) {
							mesh.append(makeCross(point, ofColor::green, cross_size, cross_width, 0));
						}
					}
				});
			});
			mesh.draw();
			popScissor();
			popMatrix();
		}	break;
	}
}

void WarpingEditor::gui()
{
}
