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

	std::shared_ptr<MeshType> getIfInside(std::shared_ptr<DataType> data, const glm::vec2 &pos, float &distance) override;

	bool isPreventMeshInterpolation() const override { return true; }
	
	void beginShader() const override { shader_.begin(tex_); }
	void endShader() const override { shader_.end(); }
	void gui() override;
	
	ofxBlendScreen::Shader::Params& getShaderParam() const { return shader_.getParams(); }
	void setShaderParam(const ofxBlendScreen::Shader::Params &params) { shader_.getParams() = params; }

private:
	std::vector<int> getEditableMeshIndex(int state);
	
	mutable ofxBlendScreen::Shader shader_;
};
