#pragma once

#include "ofxBlendScreen.h"
#include "ofxEditorFrame.h"
#include "ofxMapperMesh.h"
#include <glm/vec2.hpp>
#include "Pick.h"
#include "Quad.h"

class CroppingEditor : public ofxEditorFrame
{
public:
	virtual ~CroppingEditor();
	using FrameID = std::size_t;
	FrameID addFrame(const ofRectangle &rect, glm::vec2 h_range={0,1}, glm::vec2 v_range={0,1}, const geom::Quad &uv={1,1});
	void removeFrame(FrameID identifier);
	void clear();
	ofMesh getResult() const;
	ofMesh getResult(FrameID identifier) const;
	geom::Quad* getUVQuad(FrameID identifier);
	
	std::vector<ofx::mapper::Mesh::PointRef> getHover();
	std::vector<ofx::mapper::Mesh::PointRef> getSelected();
	std::vector<ofx::mapper::Mesh::PointRef> getHover() const;
	std::vector<ofx::mapper::Mesh::PointRef> getSelected() const;
	
private:
	struct Frame {
		geom::Quad vertex_frame;
		geom::Quad texture_uv_for_frame;
		std::shared_ptr<ofx::mapper::Mesh> points;
		MeshPicker picker;
	};
	std::unordered_map<FrameID, std::shared_ptr<Frame>> frame_;
};
