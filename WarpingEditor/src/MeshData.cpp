#include "MeshData.h"

#pragma mark - IO

namespace {
template<typename T>
void writeTo(std::ostream& os, const T& t) {
	os.write(reinterpret_cast<const char*>(&t), sizeof(T));
}
template<typename T>
void readFrom(std::istream& is, T& t) {
	is.read(reinterpret_cast<char*>(&t), sizeof(T));
}
}

void DataContainerBase::save(const std::filesystem::path &filepath, glm::vec2 scale) const
{
	ofFile file(filepath, ofFile::WriteOnly);
	pack(file, scale);
	file.close();
}
void DataContainerBase::load(const std::filesystem::path &filepath, glm::vec2 scale)
{
	clear();
	ofFile file(filepath);
	unpack(file, scale);
	file.close();
}


template<typename Data>
void DataContainer<Data>::update() {
	for(auto &&d : data_) {
		d.second->update();
	}
}
template<typename Data>
bool DataContainer<Data>::remove(const std::string &name) {
	auto found = data_.find(name);
	if(found == std::end(data_)) {
		return false;
	}
	data_.erase(found);
	return true;
}
template<typename Data>
bool DataContainer<Data>::remove(const std::shared_ptr<Data> data) {
	auto found = std::find_if(begin(data_), end(data_), [data](const std::pair<std::string, std::shared_ptr<Data>> d) {
		return d.second == data;
	});
	if(found == std::end(data_)) {
		return false;
	}
	data_.erase(found);
	return true;
}

template<typename Data>
bool DataContainer<Data>::isVisible(std::shared_ptr<Data> data) const
{
	return std::any_of(begin(data_), end(data_), [](const std::pair<std::string, std::shared_ptr<Data>> d) {
		return d.second->is_solo;
	}) ? data->is_solo && !data->is_hidden : !data->is_hidden;
}
template<typename Data>
bool DataContainer<Data>::isEditable(std::shared_ptr<Data> data, bool include_hidden) const
{
	return !data->is_locked && (include_hidden || isVisible(data));
}

template<typename Data>
std::map<std::string, std::shared_ptr<Data>> DataContainer<Data>::getVisibleData() const
{
	std::map<std::string, std::shared_ptr<Data>> ret;
	auto isVisible = std::any_of(begin(data_), end(data_), [](const std::pair<std::string, std::shared_ptr<Data>> d) {
		return d.second->is_solo;
	}) ? [](std::shared_ptr<Data> d) {
		return d->is_solo && !d->is_hidden;
	} : [](std::shared_ptr<Data> d) {
		return !d->is_hidden;
	};
	for(auto &&d : data_) {
		if(isVisible(d.second)) {
			ret.insert(d);
		}
	}
	return ret;
}
template<typename Data>
std::map<std::string, std::shared_ptr<Data>> DataContainer<Data>::getEditableData(bool include_hidden) const
{
	std::map<std::string, std::shared_ptr<Data>> ret;
	auto isVisible = std::any_of(begin(data_), end(data_), [](const std::pair<std::string, std::shared_ptr<Data>> d) {
		return d.second->is_solo;
	}) ? [](std::shared_ptr<Data> d) {
		return d->is_solo && !d->is_hidden;
	} : [](std::shared_ptr<Data> d) {
		return !d->is_hidden;
	};
	for(auto &&d : data_) {
		if(!d.second->is_locked && (include_hidden || isVisible(d.second))) {
			ret.insert(d);
		}
	}
	return ret;
}

#pragma mark - IO

template<typename Data>
void DataContainer<Data>::pack(std::ostream &stream, glm::vec2 scale) const
{
	writeTo(stream, data_.size());
	const int name_alignemt = 4;
	for(auto &&d : data_) {
		auto name = d.first;
		std::size_t name_size = name.size();
		std::size_t pad_size = std::ceil(name_size/(float)name_alignemt) * name_alignemt;
		writeTo(stream, name.size());
		stream.write(name.c_str(), pad_size);
		d.second->pack(stream, scale);
	}
}

template<typename Data>
void DataContainer<Data>::unpack(std::istream &stream, glm::vec2 scale)
{
	const int name_alignemt = 4;
	std::size_t num;
	readFrom(stream, num);
	while(num-->0) {
		std::size_t name_size;
		readFrom(stream, name_size);
		std::size_t pad_size = std::ceil(name_size/(float)name_alignemt) * name_alignemt;
		std::string name;
		name.resize(pad_size);
		stream.read(const_cast<char*>(name.data()), pad_size);
		name.resize(name_size);
		auto data = std::make_shared<Data>();
		data->unpack(stream, scale);
		data_.insert(std::make_pair(name, data));
	}
}



// -------------

std::pair<std::string, std::shared_ptr<MeshData::Mesh>> MeshData::create(const std::string &name, const glm::ivec2 &num_cells, const ofRectangle &vert_rect, const ofRectangle &coord_rect) {
	std::string n = name;
	int index=0;
	auto m = std::make_shared<Mesh>();
	while(!add(n, m)) {
		n = name+ofToString(index++);
	}
	m->init(num_cells, vert_rect, coord_rect);
	return std::make_pair(n, m);
}
std::pair<std::string, std::shared_ptr<MeshData::Mesh>> MeshData::createCopy(const std::string &name, std::shared_ptr<Mesh> src)
{
	auto ret = create(name,{1,1},{0,0,1,1});
	*ret.second = *src;
	return ret;
}


void MeshData::uvRescale(const glm::vec2 &scale)
{
	for(auto &&d : data_) {
		*d.second->uv_quad = getScaled(*d.second->uv_quad, scale);
	}
}

std::pair<std::string, std::shared_ptr<MeshData::Mesh>> MeshData::find(std::shared_ptr<geom::Quad> quad)
{
	auto found = std::find_if(begin(data_), end(data_), [quad](const std::pair<std::string, std::shared_ptr<Mesh>> d) {
		return d.second->uv_quad == quad;
	});
	if(found == std::end(data_)) {
		return {"", nullptr};
	}
	return *found;
}

std::pair<std::string, std::shared_ptr<MeshData::Mesh>> MeshData::find(std::shared_ptr<ofx::mapper::Mesh> mesh)
{
	auto found = std::find_if(begin(data_), end(data_), [mesh](const std::pair<std::string, std::shared_ptr<Mesh>> d) {
		return d.second->mesh == mesh;
	});
	if(found == std::end(data_)) {
		return {"", nullptr};
	}
	return *found;
}

#pragma mark - IO

void MeshData::exportMesh(const std::filesystem::path &filepath, float resample_min_interval, const glm::vec2 &coord_size, bool only_visible) const
{
	getMeshForExport(resample_min_interval, coord_size).save(filepath);
}

ofMesh MeshData::getMeshForExport(float resample_min_interval, const glm::vec2 &coord_size, bool only_visible) const
{
	ofMesh ret;
	for(auto &&d : only_visible ? getVisibleData() : data_) {
		d.second->setDirty();	// force clear cache
		ret.append(d.second->getMesh(resample_min_interval, coord_size));
	}
	return ret;
}

void MeshData::Mesh::pack(std::ostream &stream, glm::vec2 scale) const
{
	writeTo(stream, is_hidden);
	writeTo(stream, is_locked);
	writeTo(stream, is_solo);
	for(int i = 0; i < uv_quad->size(); ++i) {
		writeTo(stream, uv_quad->pt[i].x*scale.x);
		writeTo(stream, uv_quad->pt[i].y*scale.y);
	}
	mesh->pack(stream, interpolator.get());
}
void MeshData::Mesh::unpack(std::istream &stream, glm::vec2 scale)
{
	readFrom(stream, is_hidden);
	readFrom(stream, is_locked);
	readFrom(stream, is_solo);
	for(int i = 0; i < uv_quad->size(); ++i) {
		readFrom(stream, uv_quad->pt[i].x); uv_quad->pt[i].x *= scale.x;
		readFrom(stream, uv_quad->pt[i].y); uv_quad->pt[i].y *= scale.y;
	}
	mesh->unpack(stream, interpolator.get());
}


template class DataContainer<WarpingMesh>;
