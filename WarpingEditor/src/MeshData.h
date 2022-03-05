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
	virtual void rescale(const glm::vec2 &scale) {}
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
	bool isDirtyAny() const;
	std::map<std::string, std::shared_ptr<DataType>>& getData() { return data_; }
	std::map<std::string, std::shared_ptr<DataType>> getVisibleData() const;
	std::map<std::string, std::shared_ptr<DataType>> getEditableData(bool include_hidden=false) const;
	bool isVisible(std::shared_ptr<DataType> mesh) const;
	bool isEditable(std::shared_ptr<DataType> mesh, bool include_hidden=false) const;

	void pack(std::ostream &stream, glm::vec2 scale) const override;
	void unpack(std::istream &stream, glm::vec2 scale) override;
	
	void gui(std::function<bool(DataType&)> is_selected, std::function<void(DataType&, bool)> set_selected, std::function<void()> create_new);
protected:
	std::map<std::string, std::shared_ptr<DataType>> data_;
	bool add(const std::string &name, std::shared_ptr<DataType> data) {
		return data_.insert(std::make_pair(name, data)).second;
	}
	std::pair<std::string, std::shared_ptr<DataType>> createCopy(const std::string &name, std::shared_ptr<DataType> src);

	std::pair<std::string, std::weak_ptr<DataType>> mesh_edit_;
	std::string mesh_name_buf_;
	bool need_keyboard_focus_=false;
};

struct MeshData {
	bool is_hidden=false;
	bool is_locked=false;
	bool is_solo=false;
	void setDirty() { is_dirty_ = true; }
	bool isDirty() const { return is_dirty_; }
protected:
	mutable ofMesh cache_;
	mutable float cached_resample_interval_;
	mutable ofRectangle cached_valid_viewport_;
	mutable bool is_dirty_=true;
};

struct WarpingMesh : public MeshData {
	using UVType = geom::Quad;
	using MeshType = ofx::mapper::Mesh;
	std::shared_ptr<UVType> uv_quad;
	std::shared_ptr<MeshType> mesh;
	std::shared_ptr<ofx::mapper::Interpolator> interpolator;
	ofMesh getMesh(float resample_min_interval, const glm::vec2 &remap_coord={1,1}, const ofRectangle *use_area=nullptr) const;
	ofMesh createMesh(float resample_min_interval, const glm::vec2 &remap_coord={1,1}, const ofRectangle *use_area=nullptr) const;
	WarpingMesh() {
		uv_quad = std::make_shared<UVType>();
		mesh = std::make_shared<MeshType>();
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
};


struct BlendingMesh : public MeshData {
	template<int N>
	struct Mesh_ {
		geom::Quad quad[N];
		static int size() { return N; }
	};
	using MeshType = Mesh_<2>;
	std::shared_ptr<MeshType> mesh;
	void init(const ofRectangle &frame, float default_inner_ratio);
	void update(){}
	void pack(std::ostream &stream, glm::vec2 scale) const{}
	void unpack(std::istream &stream, glm::vec2 scale){}

	ofMesh getMesh(float resample_min_interval, const glm::vec2 &remap_coord={1,1}, const ofRectangle *use_area=nullptr) const;
	ofMesh getWireframe(const glm::vec2 &remap_coord={1,1}, const ofFloatColor &color=ofFloatColor::white) const;
	ofMesh createMesh(float resample_min_interval, const glm::vec2 &remap_coord={1,1}, const ofRectangle *use_area=nullptr) const;
	bool blend_l=true;
	bool blend_r=true;
	bool blend_t=true;
	bool blend_b=true;
	BlendingMesh() {
		mesh = std::make_shared<MeshType>();
	}
	BlendingMesh& operator=(const BlendingMesh &src) {
		*mesh = *src.mesh;
		blend_l = src.blend_l;
		blend_r = src.blend_r;
		blend_t = src.blend_t;
		blend_b = src.blend_b;
		return *this;
	}
};
class WarpingData : public DataContainer<WarpingMesh>
{
public:
	using UVType = WarpingMesh::UVType;
	using MeshType = WarpingMesh::MeshType;
	std::pair<std::string, std::shared_ptr<DataType>> create(const std::string &name, const glm::ivec2 &num_cells, const ofRectangle &vert_rect, const ofRectangle &coord_rect={0,0,1,1});
	std::pair<std::string, std::shared_ptr<DataType>> createCopy(const std::string &name, std::shared_ptr<DataType> src);
	std::pair<std::string, std::shared_ptr<DataType>> find(std::shared_ptr<UVType> quad);
	std::pair<std::string, std::shared_ptr<DataType>> find(std::shared_ptr<MeshType> mesh);
	
	void rescale(const glm::vec2 &scale) override { uvRescale(scale); }
	void uvRescale(const glm::vec2 &scale);

	void exportMesh(const std::filesystem::path &filepath, float resample_min_interval, const glm::vec2 &coord_size, bool only_visible=true) const;
	ofMesh getMesh(float resample_min_interval, const glm::vec2 &coord_size, ofRectangle *viewport=nullptr, bool only_visible=true) const;
	ofMesh getMeshForExport(float resample_min_interval, const glm::vec2 &coord_size, bool only_visible=true) const;
};

class BlendingData : public DataContainer<BlendingMesh>
{
public:
	using MeshType = BlendingMesh::MeshType;
	std::pair<std::string, std::shared_ptr<DataType>> create(const std::string &name, const ofRectangle &frame, const float &default_inner_ratio);
	std::pair<std::string, std::shared_ptr<DataType>> find(std::shared_ptr<MeshType> mesh);
	void exportMesh(const std::filesystem::path &filepath, float resample_min_interval, const glm::vec2 &coord_size, bool only_visible=true) const;
	ofMesh getMesh(float resample_min_interval, const glm::vec2 &coord_size, ofRectangle *viewport=nullptr, bool only_visible=true) const;
	ofMesh getMeshForExport(float resample_min_interval, const glm::vec2 &coord_size, bool only_visible=true) const;
};

extern template class DataContainer<WarpingMesh>;
extern template class DataContainer<BlendingMesh>;
