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
	void draw() const override;
	void gui() override;
	
	bool isPreventMeshInterpolation() const override { return mode_==MODE_DIVISION; }

private:
	int mode_=MODE_MESH;
	bool is_mesh_div_edited_=false;
	bool is_div_point_valid_=false;
	glm::vec2 div_point_;
};

