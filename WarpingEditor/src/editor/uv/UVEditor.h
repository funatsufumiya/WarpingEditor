#pragma once

#include "Editor.h"

class UVEditor : public Editor<WarpingMesh, geom::Quad, int>
{
public:
	std::shared_ptr<MeshType> getMeshType(const DataType &data) const override;
	PointType getPoint(const MeshType &mesh, const IndexType &index) const override;
	bool isEditablePoint(const WarpingMesh &data, IndexType index) const override;
	void forEachPoint(const WarpingMesh &data, std::function<void(const PointType&, IndexType)> func) const override;
	std::shared_ptr<MeshType> getIfInside(std::shared_ptr<DataType> data, const glm::vec2 &pos, float &distance) override;
	void moveMesh(MeshType &mesh, const glm::vec2 &delta) override;
	void movePoint(MeshType &mesh, IndexType index, const glm::vec2 &delta) override;
	ofMesh makeMeshFromMesh(const DataType &data, const ofColor &color) const override;
	ofMesh makeWireFromMesh(const DataType &data, const ofColor &color) const override;
	std::set<IndexType> getIndices(std::shared_ptr<MeshType> mesh) const override;
	void gui() override;
};
