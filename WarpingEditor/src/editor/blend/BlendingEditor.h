#pragma once

#include "Editor.h"
#include "ofxBlendScreen.h"
#include "ofFbo.h"

class BlendingEditor : public Editor<BlendingMesh, BlendingMesh::MeshType, std::pair<int,int>>
{
public:
	BlendingEditor() {
		shader_.setup();
	}
	PointType getPoint(const MeshType &mesh, const IndexType &index) const override;
	void moveMesh(MeshType &mesh, const glm::vec2 &delta) override;
	void movePoint(MeshType &mesh, IndexType index, const glm::vec2 &delta) override;
	std::shared_ptr<MeshType> getMeshType(const DataType &data) const override;
	void forEachPoint(const DataType &data, std::function<void(const PointType&, IndexType)> func) const override;
	ofMesh makeMeshFromMesh(const DataType &data, const ofColor &color) const override;
	ofMesh makeWireFromMesh(const DataType &data, const ofColor &color) const override;
	ofMesh makeBackground() const override;

	std::shared_ptr<MeshType> getIfInside(std::shared_ptr<DataType> data, const glm::vec2 &pos, float &distance) override;

	bool isPreventMeshInterpolation() const override { return true; }
	
	void setFboSize(glm::ivec2 size);
	void update() override;
	void draw() const override;
	void gui() override;

private:
	enum State {
		EDIT_FRAME,
		EDIT_VERTEX
	};
	int state_=EDIT_FRAME;
	std::vector<int> getEditableMeshIndex(int state);
	
	ofFbo fbo_;
	mutable ofxBlendScreen::Shader shader_;
};
