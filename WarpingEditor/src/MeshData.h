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
#include "ofxBlendScreen.h"
#include "SaveData.h"
#include "Memo.h"

class DataContainerBase : public HasSaveDataWithArg<glm::vec2>
{
public:
	void save(const std::filesystem::path &filepath, glm::vec2 scale) const;
	void load(const std::filesystem::path &filepath, glm::vec2 scale);
	using HasSaveDataWithArg::pack;
	using HasSaveDataWithArg::unpack;
	virtual void clear(){}
	virtual void rescale(const glm::vec2 &scale) {}
};

template<typename Data>
class DataContainer : public DataContainerBase
{
public:
	using DataType = Data;
	using NamedData = std::pair<std::string, std::shared_ptr<Data>>;
	using DataMap = std::vector<NamedData>;
	using NamedDataWeak = std::pair<std::string, std::weak_ptr<Data>>;

	void update();
	bool remove(const std::string &name);
	bool remove(const std::shared_ptr<DataType> mesh);
	void clear() override { data_.clear(); }
	bool isDirtyAny() const;
	DataMap& getData() { return data_; }
	DataMap getVisibleData() const;
	DataMap getEditableData(bool include_hidden=false) const;
	bool isVisible(std::shared_ptr<DataType> mesh) const;
	bool isEditable(std::shared_ptr<DataType> mesh, bool include_hidden=false) const;

	virtual void pack(std::ostream &stream, const glm::vec2 &scale) const override;
	virtual void unpack(std::istream &stream, const glm::vec2 &scale) override;
	
	void gui(std::function<bool(DataType&)> is_selected, std::function<void(DataType&, bool)> set_selected, std::function<void()> create_new);
protected:
	DataMap data_;
	std::pair<typename DataMap::iterator, bool> insert(DataMap &src, NamedData data) const {
		auto found = find(src, data.first);
		if(found != end(src)) {
			return {found, false};
		}
		return {src.insert(end(src), data), true};
	}
	typename DataMap::iterator find(DataMap &src, const std::string &name) const {
		for(auto it = begin(src); it != end(src); ++it) {
			if(it->first == name) {
				return it;
			}
		}
		return end(src);
	}
	NamedData createCopy(const std::string &name, std::shared_ptr<DataType> src);

	NamedDataWeak mesh_edit_;
	std::string mesh_name_buf_;
	bool need_keyboard_focus_=false;
};

class CacheChecker;
struct CacheIdentifier {
	float resample_min_interval;
	ofRectangle valid_viewport;
	CacheIdentifier& operator=(const CacheChecker &c);
};
struct CacheChecker {
	float resample_min_interval;
	const ofRectangle *use_area;
	bool operator!=(const CacheIdentifier &cache) const {
		return !ofIsFloatEqual(resample_min_interval, cache.resample_min_interval)
		|| (use_area && *use_area != cache.valid_viewport);
	}
};
inline CacheIdentifier& CacheIdentifier::operator=(const CacheChecker &c) {
	this->resample_min_interval = c.resample_min_interval;
	if(c.use_area) {
		this->valid_viewport = *c.use_area;
	}
	return *this;
}
struct MeshData {
	bool is_hidden=false;
	bool is_locked=false;
	bool is_solo=false;
	void setDirty() { is_dirty_ = true; }
	bool isDirty() const { return is_dirty_; }
	void pack(std::ostream &stream, glm::vec2 scale) const;
	void unpack(std::istream &stream, glm::vec2 scale);
	ofMesh getMesh(float resample_min_interval, const glm::vec2 &remap_coord={1,1}, const ofRectangle *use_area=nullptr) const;
	virtual ofMesh createMesh(float resample_min_interval, const glm::vec2 &remap_coord={1,1}, const ofRectangle *use_area=nullptr) const { return {}; }

protected:
	mutable Memo<ofMesh, CacheIdentifier, CacheChecker> memo_;
	mutable bool is_dirty_=true;
};

struct WarpingMesh : public MeshData {
	using UVType = geom::Quad;
	using MeshType = ofx::mapper::Mesh;
	std::shared_ptr<UVType> uv_quad;
	std::shared_ptr<MeshType> mesh;
	std::shared_ptr<ofx::mapper::Interpolator> interpolator;
	ofMesh createMesh(float resample_min_interval, const glm::vec2 &remap_coord={1,1}, const ofRectangle *use_area=nullptr) const override;
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
	void pack(std::ostream &stream, glm::vec2 scale) const;
	void unpack(std::istream &stream, glm::vec2 scale);

	ofMesh getWireframe(const glm::vec2 &remap_coord={1,1}, const ofFloatColor &color=ofFloatColor::white) const;
	ofMesh createMesh(float resample_min_interval, const glm::vec2 &remap_coord={1,1}, const ofRectangle *use_area=nullptr) const override;
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
	NamedData create(const std::string &name, const glm::ivec2 &num_cells, const ofRectangle &vert_rect, const ofRectangle &coord_rect={0,0,1,1});
	NamedData createCopy(const std::string &name, std::shared_ptr<DataType> src);
	NamedData find(std::shared_ptr<UVType> quad);
	NamedData find(std::shared_ptr<MeshType> mesh);
	
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
	BlendingData() {
		shader_ = std::make_shared<ofxBlendScreen::Shader>();
		shader_->setup();
	}
	NamedData create(const std::string &name, const ofRectangle &frame, const float &default_inner_ratio);
	NamedData find(std::shared_ptr<MeshType> mesh);
	void exportMesh(const std::filesystem::path &filepath, float resample_min_interval, const glm::vec2 &coord_size, bool only_visible=true) const;
	ofMesh getMesh(float resample_min_interval, const glm::vec2 &coord_size, ofRectangle *viewport=nullptr, bool only_visible=true) const;
	ofMesh getMeshForExport(float resample_min_interval, const glm::vec2 &coord_size, bool only_visible=true) const;

	std::shared_ptr<ofxBlendScreen::Shader> getShader() const { return shader_; }
	virtual void pack(std::ostream &stream, const glm::vec2 &scale) const override;
	virtual void unpack(std::istream &stream, const glm::vec2 &scale) override;
private:
	std::shared_ptr<ofxBlendScreen::Shader> shader_;
};

extern template class DataContainer<WarpingMesh>;
extern template class DataContainer<BlendingMesh>;
