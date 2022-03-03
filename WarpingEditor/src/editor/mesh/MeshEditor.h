#pragma once

#include "Editor.h"

class MeshEditor : public Editor<ofx::mapper::Mesh, std::pair<int,int>>
{
public:
	using DataType = MeshData::Mesh;
	using MeshType = ofx::mapper::Mesh;
	using IndexType = std::pair<int,int>;
	using PointType = glm::vec2;

protected:
	std::shared_ptr<MeshType> getMeshType(const MeshData::Mesh &data) const override;
	PointType getPoint(const MeshType &mesh, const IndexType &index) const override;
	void forEachPoint(const MeshData::Mesh &data, std::function<void(const PointType&, IndexType)> func) const override;
	bool isEditablePoint(const MeshData::Mesh &data, IndexType index) const override;
	std::shared_ptr<MeshType> getIfInside(std::shared_ptr<MeshData::Mesh> data, const glm::vec2 &pos, float &distance) override;
	void moveMesh(MeshType &mesh, const glm::vec2 &delta) override;
	void movePoint(MeshType &mesh, IndexType index, const glm::vec2 &delta) override;
	ofMesh makeMeshFromMesh(const DataType &data, const ofColor &color) const override;
	ofMesh makeWireFromMesh(const DataType &data, const ofColor &color) const override;
	void moveSelectedCoord(const glm::vec2 &delta);
	void moveMeshCoord(MeshType &mesh, const glm::vec2 &delta);
	void movePointCoord(MeshType &mesh, IndexType index, const glm::vec2 &delta);
};

