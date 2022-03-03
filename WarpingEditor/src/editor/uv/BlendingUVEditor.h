#pragma once

#include "UVEditor.h"

class BlendingUVEditor : public UVEditor
{
public:
	using DataType = UVEditor::DataType;
	using MeshType = UVEditor::MeshType;
	using IndexType = UVEditor::IndexType;
	using PointType = UVEditor::PointType;
	void movePoint(MeshType &mesh, IndexType index, const glm::vec2 &delta) override;
};
