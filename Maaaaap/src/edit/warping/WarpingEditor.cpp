#include "WarpingEditor.h"

namespace {

WarpingEditor::MeshID NEXT_ID() {
	static WarpingEditor::MeshID identifier;
	return identifier++;
}

}

WarpingEditor::~WarpingEditor()
{
	while(!mesh_.empty()) {
		removeMesh(mesh_.begin()->first);
	}
}
WarpingEditor::MeshID WarpingEditor::addMesh(const ofRectangle &quad, const ofRectangle &uv)
{
	auto mesh = std::make_shared<Mesh>();
	auto points = std::make_shared<ofx::mapper::Mesh>();
	points->init({1,1}, quad, uv);
	mesh->points = points;
	mesh->picker.setMesh(points);
	mesh->picker.setSelectable({{0,0},{1,0},{0,1},{1,1}});
	ofAddListener(on_point_hover_, &mesh->picker, &MeshPicker::onPointHover);
	ofAddListener(on_point_selection_, &mesh->picker, &MeshPicker::onPointSelection);
	ofAddListener(on_rect_selection_, &mesh->picker, &MeshPicker::onRectSelection);
	auto identifier = NEXT_ID();
	mesh_.insert(std::make_pair(identifier, mesh));
	return identifier;
}
void WarpingEditor::removeMesh(MeshID identifier)
{
	auto found = mesh_.find(identifier);
	if(found == end(mesh_)) {
		return;
	}
	auto &mesh = found->second;
	ofRemoveListener(on_point_hover_, &mesh->picker, &MeshPicker::onPointHover);
	ofRemoveListener(on_point_selection_, &mesh->picker, &MeshPicker::onPointSelection);
	ofRemoveListener(on_rect_selection_, &mesh->picker, &MeshPicker::onRectSelection);
	mesh_.erase(found);
}
void WarpingEditor::clear()
{
	mesh_.clear();
}
ofMesh WarpingEditor::getResult() const
{
	ofMesh ret;
	for(auto &&m : mesh_) {
		ret.append(getResult(m.first));
	}
	return ret;
}
ofMesh WarpingEditor::getResult(MeshID identifier) const
{
	auto found = mesh_.find(identifier);
	if(found == end(mesh_)) {
		return {};
	}
	return found->second->points->getMesh();
}

std::vector<ofx::mapper::Mesh::PointRef> WarpingEditor::getHover()
{
	std::vector<ofx::mapper::Mesh::PointRef> ret;
	for(auto &&m : mesh_) {
		auto &&picked = m.second->picker.getHover();
		for(auto &&p : picked) {
			ret.push_back(m.second->points->getPoint(p.x, p.y));
		}
	}
	return ret;
}
std::vector<ofx::mapper::Mesh::PointRef> WarpingEditor::getSelected()
{
	std::vector<ofx::mapper::Mesh::PointRef> ret;
	for(auto &&m : mesh_) {
		auto &&picked = m.second->picker.getSelected();
		for(auto &&p : picked) {
			ret.push_back(m.second->points->getPoint(p.x, p.y));
		}
	}
	return ret;
}
std::vector<ofx::mapper::Mesh::PointRef> WarpingEditor::getHover() const
{
	std::vector<ofx::mapper::Mesh::PointRef> ret;
	for(auto &&m : mesh_) {
		auto &&picked = m.second->picker.getHover();
		for(auto &&p : picked) {
			ret.push_back(m.second->points->getPoint(p.x, p.y));
		}
	}
	return ret;
}
std::vector<ofx::mapper::Mesh::PointRef> WarpingEditor::getSelected() const
{
	std::vector<ofx::mapper::Mesh::PointRef> ret;
	for(auto &&m : mesh_) {
		auto &&picked = m.second->picker.getSelected();
		for(auto &&p : picked) {
			ret.push_back(m.second->points->getPoint(p.x, p.y));
		}
	}
	return ret;
}
