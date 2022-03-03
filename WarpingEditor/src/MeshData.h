//
//  MeshData.h
//  WarpingEditor
//
//  Created by Iwatani Nariaki on 2021/12/22.
//

#pragma once

#include <map>
#include "ofxMapperMesh.h"
#include "ofxMapperUpSampler.h"
#include "Quad.h"

class DataContainerBase
{
public:
	void save(const std::filesystem::path &filepath, glm::vec2 scale) const;
	void load(const std::filesystem::path &filepath, glm::vec2 scale);
	virtual void pack(std::ostream &stream, glm::vec2 scale) const {}
	virtual void unpack(std::istream &stream, glm::vec2 scale) {}
	virtual void clear(){}
};

template<typename Data>
class DataContainer : public DataContainerBase
{
public:
	using DataType = Data;
	void update();
	bool remove(const std::string &name);
	bool remove(const std::shared_ptr<DataType> mesh);
	void clear() override { data_.clear(); }
	std::map<std::string, std::shared_ptr<DataType>>& getData() { return data_; }
	std::map<std::string, std::shared_ptr<DataType>> getVisibleData() const;
	std::map<std::string, std::shared_ptr<DataType>> getEditableData(bool include_hidden=false) const;
	bool isVisible(std::shared_ptr<DataType> mesh) const;
	bool isEditable(std::shared_ptr<DataType> mesh, bool include_hidden=false) const;

	void pack(std::ostream &stream, glm::vec2 scale) const override;
	void unpack(std::istream &stream, glm::vec2 scale) override;
protected:
	std::map<std::string, std::shared_ptr<DataType>> data_;
	bool add(const std::string &name, std::shared_ptr<DataType> data) {
		return data_.insert(std::make_pair(name, data)).second;
	}
};

struct WarpingMesh {
	bool is_hidden=false;
	bool is_locked=false;
	bool is_solo=false;
	std::shared_ptr<geom::Quad> uv_quad;
	std::shared_ptr<ofx::mapper::Mesh> mesh;
	std::shared_ptr<ofx::mapper::Interpolator> interpolator;
	ofMesh getMesh(float resample_min_interval, const glm::vec2 &remap_coord={1,1}, const ofRectangle *use_area=nullptr) const {
		if(is_dirty_ || ofIsFloatEqual(cached_resample_interval_, resample_min_interval) || (use_area && *use_area != cached_valid_viewport_)) {
			cache_ = ofx::mapper::UpSampler().proc(*mesh, resample_min_interval, use_area);
			auto uv = geom::getScaled(*uv_quad, remap_coord);
			for(auto &t : cache_.getTexCoords()) {
				t = geom::rescalePosition(uv, t);
			}
			is_dirty_ = false;
			cached_resample_interval_ = resample_min_interval;
			if(use_area) {
				cached_valid_viewport_ = *use_area;
			}
		}
		return cache_;
	}
	WarpingMesh() {
		uv_quad = std::make_shared<geom::Quad>();
		mesh = std::make_shared<ofx::mapper::Mesh>();
		interpolator = std::make_shared<ofx::mapper::Interpolator>();
		interpolator->setMesh(mesh);
	}
	WarpingMesh& operator=(const WarpingMesh &src) {
		*uv_quad = *src.uv_quad;
		*mesh = *src.mesh;
		*interpolator = *src.interpolator;
		interpolator->setMesh(mesh);
		return *this;
	}
	void init(const glm::ivec2 &num_cells, const ofRectangle &vert_rect, const ofRectangle &coord_rect={0,0,1,1}) {
		mesh->init(num_cells, vert_rect, {0,0,1,1});
		*uv_quad = coord_rect;
	}
	void update() {
		interpolator->update();
	}
	void pack(std::ostream &stream, glm::vec2 scale) const;
	void unpack(std::istream &stream, glm::vec2 scale);
	void setDirty() { is_dirty_ = true; }
private:
	mutable ofMesh cache_;
	mutable float cached_resample_interval_;
	mutable ofRectangle cached_valid_viewport_;
	mutable bool is_dirty_=true;
};

extern template class DataContainer<WarpingMesh>;

class MeshData : public DataContainer<WarpingMesh>
{
public:
	using Mesh = WarpingMesh;
	std::pair<std::string, std::shared_ptr<Mesh>> create(const std::string &name, const glm::ivec2 &num_cells, const ofRectangle &vert_rect, const ofRectangle &coord_rect={0,0,1,1});
	std::pair<std::string, std::shared_ptr<Mesh>> createCopy(const std::string &name, std::shared_ptr<Mesh> src);
	std::pair<std::string, std::shared_ptr<MeshData::Mesh>> find(std::shared_ptr<geom::Quad> quad);
	std::pair<std::string, std::shared_ptr<MeshData::Mesh>> find(std::shared_ptr<ofx::mapper::Mesh> mesh);

	void uvRescale(const glm::vec2 &scale);
	void exportMesh(const std::filesystem::path &filepath, float resample_min_interval, const glm::vec2 &coord_size, bool only_visible=true) const;
	ofMesh getMeshForExport(float resample_min_interval, const glm::vec2 &coord_size, bool only_visible=true) const;
};

struct BlendData {
};
