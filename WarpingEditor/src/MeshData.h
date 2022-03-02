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

class MeshData
{
public:
	struct Mesh {
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
		Mesh() {
			uv_quad = std::make_shared<geom::Quad>();
			mesh = std::make_shared<ofx::mapper::Mesh>();
			interpolator = std::make_shared<ofx::mapper::Interpolator>();
			interpolator->setMesh(mesh);
		}
		Mesh& operator=(const Mesh &src) {
			*uv_quad = *src.uv_quad;
			*mesh = *src.mesh;
			*interpolator = *src.interpolator;
			interpolator->setMesh(mesh);
			return *this;
		}
		void init(const ofRectangle &rect) {
			mesh->init({1,1}, rect);
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
	std::pair<std::string, std::shared_ptr<Mesh>> create(const std::string &name="data", const ofRectangle &rect={0,0,1,1});
	std::pair<std::string, std::shared_ptr<Mesh>> createCopy(const std::string &name, std::shared_ptr<Mesh> src);
	void update();
	bool remove(const std::string &name);
	bool remove(const std::shared_ptr<Mesh> mesh);
	std::map<std::string, std::shared_ptr<Mesh>>& getMesh() { return mesh_; }
	std::map<std::string, std::shared_ptr<Mesh>> getVisibleMesh() const;
	std::map<std::string, std::shared_ptr<Mesh>> getEditableMesh(bool include_hidden=false) const;
	bool isVisible(std::shared_ptr<Mesh> mesh) const;
	bool isEditable(std::shared_ptr<Mesh> mesh, bool include_hidden=false) const;
	std::pair<std::string, std::shared_ptr<MeshData::Mesh>> find(std::shared_ptr<geom::Quad> quad);
	std::pair<std::string, std::shared_ptr<MeshData::Mesh>> find(std::shared_ptr<ofx::mapper::Mesh> mesh);

	void uvRescale(const glm::vec2 &scale);
	void save(const std::filesystem::path &filepath, glm::vec2 scale) const;
	void load(const std::filesystem::path &filepath, glm::vec2 scale);
	void pack(std::ostream &stream, glm::vec2 scale) const;
	void unpack(std::istream &stream, glm::vec2 scale);
	void exportMesh(const std::filesystem::path &filepath, float resample_min_interval, const glm::vec2 &coord_size, bool only_visible=true) const;
	ofMesh getMeshForExport(float resample_min_interval, const glm::vec2 &coord_size, bool only_visible=true) const;
private:
	std::map<std::string, std::shared_ptr<Mesh>> mesh_;
	bool add(const std::string &name, std::shared_ptr<Mesh> mesh) {
		return mesh_.insert(std::make_pair(name, mesh)).second;
	}
};
