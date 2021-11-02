#include "CroppingEditor.h"

namespace {

CroppingEditor::FrameID NEXT_ID() {
	static CroppingEditor::FrameID identifier;
	return identifier++;
}

}

CroppingEditor::~CroppingEditor()
{
	while(!frame_.empty()) {
		removeFrame(frame_.begin()->first);
	}
}
CroppingEditor::FrameID CroppingEditor::addFrame(const ofRectangle &rect, glm::vec2 h_range, glm::vec2 v_range, const geom::Quad &uv)
{
	auto frame = std::make_shared<Frame>();
	frame->vertex_frame = rect;
	frame->texture_uv_for_frame = uv;
	auto points = std::make_shared<ofx::mapper::Mesh>();
	points->init({1,1}, rect);
	points->divideRow(0, {v_range[0], v_range[1]});
	points->divideCol(0, {h_range[0], h_range[1]});
	frame->points = points;
	frame->picker.setMesh(points);
	frame->picker.setSelectable({{0,0},{3,0},{1,1},{2,1},{1,2},{2,2},{0,3},{3,3}});
	ofAddListener(on_point_hover_, &frame->picker, &MeshPicker::onPointHover);
	ofAddListener(on_point_selection_, &frame->picker, &MeshPicker::onPointSelection);
	ofAddListener(on_rect_selection_, &frame->picker, &MeshPicker::onRectSelection);
	auto identifier = NEXT_ID();
	frame_.insert(std::make_pair(identifier, frame));
	return identifier;
}
void CroppingEditor::removeFrame(FrameID identifier)
{
	auto found = frame_.find(identifier);
	if(found == end(frame_)) {
		return;
	}
	auto &frame = found->second;
	ofRemoveListener(on_point_hover_, &frame->picker, &MeshPicker::onPointHover);
	ofRemoveListener(on_point_selection_, &frame->picker, &MeshPicker::onPointSelection);
	ofRemoveListener(on_rect_selection_, &frame->picker, &MeshPicker::onRectSelection);
	frame_.erase(found);
}
void CroppingEditor::clear()
{
	frame_.clear();
}
ofMesh CroppingEditor::getResult() const
{
	ofMesh ret;
	for(auto &&f : frame_) {
		ret.append(getResult(f.first));
	}
	return ret;
}
ofMesh CroppingEditor::getResult(FrameID identifier) const
{
	auto found = frame_.find(identifier);
	if(found == end(frame_)) {
		return {};
	}
	auto &&frame = found->second;
	auto &&points = frame->points;
	return ofxBlendScreen::createMesh(
	  {*points->getPoint(0, 0).v, *points->getPoint(3, 0).v, *points->getPoint(0, 3).v, *points->getPoint(3, 3).v},
	  {*points->getPoint(1, 1).v, *points->getPoint(2, 1).v, *points->getPoint(1, 2).v, *points->getPoint(2, 2).v},
	  frame->vertex_frame, frame->texture_uv_for_frame
	);
}

geom::Quad* CroppingEditor::getUVQuad(FrameID identifier)
{
	auto found = frame_.find(identifier);
	return found == end(frame_) ? nullptr : &found->second->texture_uv_for_frame;
}

std::vector<ofx::mapper::Mesh::PointRef> CroppingEditor::getHover()
{
	std::vector<ofx::mapper::Mesh::PointRef> ret;
	for(auto &&f : frame_) {
		auto &&picked = f.second->picker.getHover();
		for(auto &&p : picked) {
			ret.push_back(f.second->points->getPoint(p.x, p.y));
		}
	}
	return ret;
}
std::vector<ofx::mapper::Mesh::PointRef> CroppingEditor::getSelected()
{
	std::vector<ofx::mapper::Mesh::PointRef> ret;
	for(auto &&f : frame_) {
		auto &&picked = f.second->picker.getSelected();
		for(auto &&p : picked) {
			ret.push_back(f.second->points->getPoint(p.x, p.y));
		}
	}
	return ret;
}
std::vector<ofx::mapper::Mesh::PointRef> CroppingEditor::getHover() const
{
	std::vector<ofx::mapper::Mesh::PointRef> ret;
	for(auto &&f : frame_) {
		auto &&picked = f.second->picker.getHover();
		for(auto &&p : picked) {
			ret.push_back(f.second->points->getPoint(p.x, p.y));
		}
	}
	return ret;
}
std::vector<ofx::mapper::Mesh::PointRef> CroppingEditor::getSelected() const
{
	std::vector<ofx::mapper::Mesh::PointRef> ret;
	for(auto &&f : frame_) {
		auto &&picked = f.second->picker.getSelected();
		for(auto &&p : picked) {
			ret.push_back(f.second->points->getPoint(p.x, p.y));
		}
	}
	return ret;
}
