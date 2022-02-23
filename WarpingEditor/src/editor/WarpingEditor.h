#pragma once

#include "Editor.h"

class WarpingEditor : public Editor<ofx::mapper::Mesh, std::pair<int,int>>
{
public:
	using DataType = Data::Mesh;
	using MeshType = ofx::mapper::Mesh;
	using IndexType = std::pair<int,int>;
	using PointType = glm::vec2;
	enum {
		MODE_MESH,
		MODE_DIVISION
	};
	void update() override;
	void draw() const override;
	void gui() override;
	
	bool isPreventMeshInterpolation() const override { return mode_==MODE_DIVISION; }

protected:
	std::shared_ptr<MeshType> getMeshType(const Data::Mesh &data) const override;
	PointType getPoint(const MeshType &mesh, const IndexType &index) const override;
	void forEachPoint(const Data::Mesh &data, std::function<void(const PointType&, IndexType)> func) const override;
	bool isEditablePoint(const Data::Mesh &data, IndexType index) const override;
	std::shared_ptr<MeshType> getIfInside(std::shared_ptr<Data::Mesh> data, const glm::vec2 &pos, float &distance) override;
	void moveMesh(MeshType &mesh, const glm::vec2 &delta) override;
	void movePoint(MeshType &mesh, IndexType index, const glm::vec2 &delta) override;
	ofMesh makeMeshFromMesh(const DataType &data, const ofColor &color) const override;
	ofMesh makeWireFromMesh(const DataType &data, const ofColor &color) const override;
	void moveSelectedCoord(const glm::vec2 &delta);
	void moveMeshCoord(MeshType &mesh, const glm::vec2 &delta);
	void movePointCoord(MeshType &mesh, IndexType index, const glm::vec2 &delta);
private:
	int mode_=MODE_MESH;
	bool is_mesh_div_edited_=false;
};

