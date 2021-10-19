#pragma once

#include "ofxBlendScreen.h"
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
	MeshPicker(ofx::mapper::Mesh &mesh);
	virtual ~MeshPicker();
	
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
	ofx::mapper::Mesh &mesh_;
	PointPick picker_;
	Mode mode_;
	ofx::mapper::Selector hover_, selection_, selectable_;
};

class BlendingEditor : public ofxEditorFrame
{
public:
	virtual ~BlendingEditor();
	using FrameID = std::size_t;
	FrameID addFrame(const ofRectangle &rect, glm::vec2 h_range={0,1}, glm::vec2 v_range={0,1}, const ofxBlendScreen::Quad &uv={1,1});
	void removeFrame(FrameID identifier);
	void clear();
	ofMesh getResult() const;
	ofMesh getResult(FrameID identifier) const;
	
	std::vector<ofx::mapper::Mesh::PointRef> getHover();
	std::vector<ofx::mapper::Mesh::PointRef> getSelected();
	std::vector<ofx::mapper::Mesh::PointRef> getHover() const;
	std::vector<ofx::mapper::Mesh::PointRef> getSelected() const;
	
private:
	struct Frame {
		ofxBlendScreen::Quad vertex_frame;
		ofxBlendScreen::Quad texture_uv_for_frame;
		ofx::mapper::Mesh points;
		MeshPicker picker;
		Frame():picker(points){}
	};
	std::unordered_map<FrameID, std::shared_ptr<Frame>> frame_;
};
