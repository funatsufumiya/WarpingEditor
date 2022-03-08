#pragma once

#include "MeshEditor.h"

class WarpingMeshEditor : public MeshEditor
{
public:
	enum {
		MODE_MESH,
		MODE_DIVISION
	};
	void update() override;
	void drawControl(float parent_scale) const override;
	void drawBackground() const override {
		ofPushStyle();
		ofSetColor(ofColor::black);
		ofDrawRectangle(0,0,background_size_.x,background_size_.y);
		ofPopStyle();
	}
	void setBackgroundSize(const glm::vec2 &size) { background_size_ = size; }
	glm::vec2 getWorkAreaSize() const override { return background_size_; }
	void gui() override;
	
	bool isPreventMeshInterpolation() const override { return mode_==MODE_DIVISION; }

private:
	int mode_=MODE_MESH;
	bool is_mesh_div_edited_=false;
	bool is_div_point_valid_=false;
	glm::vec2 div_point_;
	glm::vec2 background_size_;
};

