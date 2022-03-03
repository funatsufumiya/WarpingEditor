#pragma once

#include "Editor.h"

class UVEditor : public Editor<geom::Quad, int>
{
public:
	using DataType = MeshData::Mesh;
	using MeshType = geom::Quad;
	using IndexType = int;
	using PointType = glm::vec2;
	std::shared_ptr<MeshType> getMeshType(const MeshData::Mesh &data) const override;
	PointType getPoint(const MeshType &mesh, const IndexType &index) const override;
	bool isEditablePoint(const MeshData::Mesh &data, IndexType index) const override;
	void forEachPoint(const MeshData::Mesh &data, std::function<void(const PointType&, IndexType)> func) const override;
	std::shared_ptr<MeshType> getIfInside(std::shared_ptr<DataType> data, const glm::vec2 &pos, float &distance) override;
	void moveMesh(MeshType &mesh, const glm::vec2 &delta) override;
	void movePoint(MeshType &mesh, IndexType index, const glm::vec2 &delta) override;
	ofMesh makeMeshFromMesh(const DataType &data, const ofColor &color) const override;
	ofMesh makeWireFromMesh(const DataType &data, const ofColor &color) const override;
	ofMesh makeBackground() const override;
	void gui() override;
};
