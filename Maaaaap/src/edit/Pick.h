#pragma once

#include "ofxEditorFrame.h"
#include "ofxMapperMesh.h"
#include "ofxMapperSelector.h"
#include <glm/vec2.hpp>

class Pick
{
public:
	struct Point {
		glm::ivec2 index;
	};
	virtual std::vector<Point> pickPoints(const ofx::mapper::Mesh &mesh, const glm::vec2 &pos){ return {}; }
};

class PointPick : public Pick
{
public:
	std::vector<Point> pickPoints(const ofx::mapper::Mesh &mesh, const glm::vec2 &pos) override {
		std::vector<Point> ret;
		for(auto &&p : mesh.getPointsAround(pos, margin_)) {
			ret.push_back({p.first});
		}
		return ret;
	}
	void setSelectionMargin(float margin) { margin_ = margin; }
private:
	float margin_;
};


class MeshPicker
{
public:
	void setMesh(std::shared_ptr<ofx::mapper::Mesh> mesh);
	
	void setSelectable(std::initializer_list<std::pair<int,int>> selectables);
	
	enum Mode {
		REPLACE, ADD, TOGGLE
	};

	void setMode(Mode mode) { mode_ = mode; }
	void onPointHover(const ofxEditorFrame::PointHoverArg &arg);
	void onPointSelection(const ofxEditorFrame::PointSelectionArg &arg);
	void onRectSelection(const ofxEditorFrame::RectSelectionArg &arg);
	
	std::vector<glm::ivec2> getHover();
	std::vector<glm::ivec2> getSelected();
	
private:
	std::shared_ptr<ofx::mapper::Mesh> mesh_;
	PointPick picker_;
	Mode mode_;
	ofx::mapper::Selector hover_, selection_, selectable_;
};
