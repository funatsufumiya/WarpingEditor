#pragma once

#include "ofxEditorFrame.h"
#include "ofxMapperMesh.h"
#include <glm/vec2.hpp>
#include "Pick.h"
#include <unordered_map>

class WarpingEditor : public ofxEditorFrame
{
public:
	virtual ~WarpingEditor();
	using MeshID = std::size_t;
	MeshID addMesh(const ofRectangle &quad, const ofRectangle &uv={0,0,1,1});
	void removeMesh(MeshID identifier);
	void clear();
	ofMesh getResult() const;
	ofMesh getResult(MeshID identifier) const;
	
	std::vector<ofx::mapper::Mesh::PointRef> getHover();
	std::vector<ofx::mapper::Mesh::PointRef> getSelected();
	std::vector<ofx::mapper::Mesh::PointRef> getHover() const;
	std::vector<ofx::mapper::Mesh::PointRef> getSelected() const;
	
private:
	struct Mesh {
		std::shared_ptr<ofx::mapper::Mesh> points;
		MeshPicker picker;
	};
	std::unordered_map<MeshID, std::shared_ptr<Mesh>> mesh_;
};
