#include "Pick.h"

void MeshPicker::setMesh(std::shared_ptr<ofx::mapper::Mesh> mesh)
{
	mesh_ = mesh;
	picker_.setSelectionMargin(10);
	hover_.setMesh(mesh);
	selection_.setMesh(mesh);
	selectable_.setMesh(mesh);
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
	auto picked = picker_.pickPoints(*mesh_, arg.pos);
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
	auto picked = picker_.pickPoints(*mesh_, arg.pos);
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
