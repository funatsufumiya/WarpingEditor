#pragma once

#include "Editor.h"

class MeshEditor : public Editor<WarpingMesh, ofx::mapper::Mesh, std::pair<int,int>>
{
protected:
	std::shared_ptr<MeshType> getMeshType(const DataType &data) const override;
	PointType getPoint(const MeshType &mesh, const IndexType &index) const override;
	void forEachPoint(const DataType &data, std::function<void(const PointType&, IndexType)> func) const override;
	bool isEditablePoint(const DataType &data, IndexType index) const override;
	std::shared_ptr<MeshType> getIfInside(std::shared_ptr<DataType> data, const glm::vec2 &pos, float &distance) override;
	void moveMesh(MeshType &mesh, const glm::vec2 &delta) override;
	void movePoint(MeshType &mesh, IndexType index, const glm::vec2 &delta) override;
	ofMesh makeMeshFromMesh(const DataType &data, const ofColor &color) const override;
	ofMesh makeWireFromMesh(const DataType &data, const ofColor &color) const override;
	void moveSelectedCoord(const glm::vec2 &delta);
	void moveMeshCoord(MeshType &mesh, const glm::vec2 &delta);
	void movePointCoord(MeshType &mesh, IndexType index, const glm::vec2 &delta);
	std::set<IndexType> getIndices(std::shared_ptr<MeshType> mesh) const override;
};

