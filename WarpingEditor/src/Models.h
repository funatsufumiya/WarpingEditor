//
//  Models.h
//  WarpingEditor
//
//  Created by Iwatani Nariaki on 2021/12/22.
//

#pragma once

#include <map>
#include "ofxMapperMesh.h"
#include "ofxMapperUpSampler.h"
#include "Quad.h"

class Data
{
public:
	static Data& shared() {
		static Data instance;
		return instance;
	}
	struct Mesh {
		std::shared_ptr<geom::Quad> uv_quad;
		std::shared_ptr<ofx::mapper::Mesh> mesh;
		std::shared_ptr<ofx::mapper::Interpolator> interpolator;
		ofMesh getMesh(float resample_min_interval, const glm::vec2 &remap_coord={1,1}) const {
			ofMesh ret = ofx::mapper::UpSampler().proc(*mesh, resample_min_interval);
			auto uv = geom::getScaled(*uv_quad, remap_coord);
			for(auto &t : ret.getTexCoords()) {
				t = geom::rescalePosition(uv, t);
			}
			return ret;
		}
		void init(const ofRectangle &rect) {
			uv_quad = std::make_shared<geom::Quad>();
			mesh = std::make_shared<ofx::mapper::Mesh>();
			mesh->init({1,1}, rect);
			interpolator = std::make_shared<ofx::mapper::Interpolator>();
			interpolator->setMesh(mesh);
			mesh->divideRow(0, 0.5f);
			mesh->divideCol(0, 0.5f);
			interpolator->selectPoint(1,1);
		}
		void update() {
			interpolator->update();
		}
	};
	std::pair<std::string, std::shared_ptr<Mesh>> create(const std::string &name="data") {
		std::string n = name;
		int index=0;
		auto m = std::make_shared<Mesh>();
		while(!add(n, m)) {
			n = name+ofToString(index++);
		}
		m->init({0,0,640, 480});
		return std::make_pair(n, m);
	}
	void update() {
		std::for_each(begin(mesh_), end(mesh_), [](const std::pair<std::string, std::shared_ptr<Mesh>> m) {
			m.second->update();
		});
	}
	bool remove(const std::string &name) {
		auto found = mesh_.find(name);
		if(found == std::end(mesh_)) {
			return false;
		}
		mesh_.erase(found);
		return true;
	}
	bool remove(const std::shared_ptr<Mesh> mesh) {
		auto found = std::find_if(begin(mesh_), end(mesh_), [mesh](const std::pair<std::string, std::shared_ptr<Mesh>> m) {
			return m.second == mesh;
		});
		if(found == std::end(mesh_)) {
			return false;
		}
		mesh_.erase(found);
		return true;
	}
	std::map<std::string, std::shared_ptr<Mesh>>& getMesh() { return mesh_; }
private:
	std::map<std::string, std::shared_ptr<Mesh>> mesh_;
	bool add(const std::string &name, std::shared_ptr<Mesh> mesh) {
		return mesh_.insert(std::make_pair(name, mesh)).second;
	}
};
