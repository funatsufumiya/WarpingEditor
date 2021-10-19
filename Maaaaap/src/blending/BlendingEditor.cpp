#include "BlendingEditor.h"

MeshPicker::MeshPicker(ofx::mapper::Mesh &mesh)
:mesh_(mesh)
{
	picker_.setSelectionMargin(10);
	hover_.resize(4,4);
	selection_.resize(4,4);
	selectable_.resize(4,4);
}
MeshPicker::~MeshPicker()
{
}

void MeshPicker::setSelectable(std::initializer_list<std::pair<int,int>> selectables)
{
	selectable_.clearAll();
	for(auto &&s : selectables) {
		selectable_.selectPoint(s.first, s.second);
	}
}

void MeshPicker::onPointHover(const ofxEditorFrame::PointHoverArg &arg)
{
	auto picked = picker_.pickPoints(mesh_, arg.pos);
	ofx::mapper::Selector selected;
	selected.resize(hover_.numCols(), hover_.numRows());
	selected.clearAll();
	for(auto &&p : picked) {
		selected.selectPoint(p.index.x, p.index.y);
	}
	hover_ = makeAnd(selectable_, selected);
}
void MeshPicker::onPointSelection(const ofxEditorFrame::PointSelectionArg &arg)
{
	if(!arg.finished) {
		return;
	}
	auto picked = picker_.pickPoints(mesh_, arg.pos);
	ofx::mapper::Selector selected;
	selected.resize(selection_.numCols(), selection_.numRows());
	selected.clearAll();
	for(auto &&p : picked) {
		selected.selectPoint(p.index.x, p.index.y);
	}
	selected = makeAnd(selectable_, selected);
	switch(mode_) {
		case REPLACE:
			selection_ = selected;
			break;
		case ADD:
			selection_ = makeOr(selection_, selected);
			break;
		case TOGGLE:
			selection_ = makeXor(selection_, selected);
			break;
	}
}
void MeshPicker::onRectSelection(const ofxEditorFrame::RectSelectionArg &arg)
{
}

std::vector<glm::ivec2> MeshPicker::getHover()
{
	return hover_.getSelectedIndices();
}
std::vector<glm::ivec2> MeshPicker::getSelected()
{
	return selection_.getSelectedIndices();
}

namespace {

BlendingEditor::FrameID NEXT_ID() {
	static BlendingEditor::FrameID identifier;
	return identifier++;
}

}

BlendingEditor::~BlendingEditor()
{
	while(!frame_.empty()) {
		removeFrame(frame_.begin()->first);
	}
}
BlendingEditor::FrameID BlendingEditor::addFrame(const ofRectangle &rect, glm::vec2 h_range, glm::vec2 v_range, const ofxBlendScreen::Quad &uv)
{
	auto frame = std::make_shared<Frame>();
	frame->vertex_frame = rect;
	frame->texture_uv_for_frame = uv;
	frame->points.init({1,1}, rect);
	frame->points.divideRow(0, {v_range[0], v_range[1]});
	frame->points.divideCol(0, {h_range[0], h_range[1]});
	frame->picker.setSelectable({{0,0},{3,0},{1,1},{2,1},{1,2},{2,2},{0,3},{3,3}});
	ofAddListener(on_point_hover_, &frame->picker, &MeshPicker::onPointHover);
	ofAddListener(on_point_selection_, &frame->picker, &MeshPicker::onPointSelection);
	ofAddListener(on_rect_selection_, &frame->picker, &MeshPicker::onRectSelection);
	auto identifier = NEXT_ID();
	frame_.insert(std::make_pair(identifier, frame));
	return identifier;
}
void BlendingEditor::removeFrame(FrameID identifier)
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
void BlendingEditor::clear()
{
	frame_.clear();
}
ofMesh BlendingEditor::getResult() const
{
	ofMesh ret;
	for(auto &&f : frame_) {
		ret.append(getResult(f.first));
	}
	return ret;
}
ofMesh BlendingEditor::getResult(FrameID identifier) const
{
	auto found = frame_.find(identifier);
	if(found == end(frame_)) {
		return {};
	}
	auto &&frame = found->second;
	auto &&points = frame->points;
	return ofxBlendScreen::createMesh(
	  {*points.getPoint(0, 0).v, *points.getPoint(3, 0).v, *points.getPoint(0, 3).v, *points.getPoint(3, 3).v},
	  {*points.getPoint(1, 1).v, *points.getPoint(2, 1).v, *points.getPoint(1, 2).v, *points.getPoint(2, 2).v},
	  frame->vertex_frame, frame->texture_uv_for_frame
	);
}

std::vector<ofx::mapper::Mesh::PointRef> BlendingEditor::getHover()
{
	std::vector<ofx::mapper::Mesh::PointRef> ret;
	for(auto &&f : frame_) {
		auto &&picked = f.second->picker.getHover();
		for(auto &&p : picked) {
			ret.push_back(f.second->points.getPoint(p.x, p.y));
		}
	}
	return ret;
}
std::vector<ofx::mapper::Mesh::PointRef> BlendingEditor::getSelected()
{
	std::vector<ofx::mapper::Mesh::PointRef> ret;
	for(auto &&f : frame_) {
		auto &&picked = f.second->picker.getSelected();
		for(auto &&p : picked) {
			ret.push_back(f.second->points.getPoint(p.x, p.y));
		}
	}
	return ret;
}
std::vector<ofx::mapper::Mesh::PointRef> BlendingEditor::getHover() const
{
	std::vector<ofx::mapper::Mesh::PointRef> ret;
	for(auto &&f : frame_) {
		auto &&picked = f.second->picker.getHover();
		for(auto &&p : picked) {
			ret.push_back(f.second->points.getPoint(p.x, p.y));
		}
	}
	return ret;
}
std::vector<ofx::mapper::Mesh::PointRef> BlendingEditor::getSelected() const
{
	std::vector<ofx::mapper::Mesh::PointRef> ret;
	for(auto &&f : frame_) {
		auto &&picked = f.second->picker.getSelected();
		for(auto &&p : picked) {
			ret.push_back(f.second->points.getPoint(p.x, p.y));
		}
	}
	return ret;
}
