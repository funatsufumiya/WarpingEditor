#pragma once

#include "MeshEditor.h"

class BlendingMeshEditor : public MeshEditor
{
public:
	using DataType = MeshEditor::DataType;
	using MeshType = MeshEditor::MeshType;
	using IndexType = MeshEditor::IndexType;
	using PointType = MeshEditor::PointType;

	void gui() override;
};
