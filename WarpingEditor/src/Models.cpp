#include "Models.h"

std::pair<std::string, std::shared_ptr<Data::Mesh>> Data::create(const std::string &name, const ofRectangle &rect) {
	std::string n = name;
	int index=0;
	auto m = std::make_shared<Mesh>();
	while(!add(n, m)) {
		n = name+ofToString(index++);
	}
	m->init(rect);
	return std::make_pair(n, m);
}
std::pair<std::string, std::shared_ptr<Data::Mesh>> Data::createCopy(const std::string &name, std::shared_ptr<Mesh> src)
{
	auto ret = create(name);
	*ret.second = *src;
	return ret;
}

void Data::update() {
	for(auto &&m : mesh_) {
		m.second->update();
	}
}
bool Data::remove(const std::string &name) {
	auto found = mesh_.find(name);
	if(found == std::end(mesh_)) {
		return false;
	}
	mesh_.erase(found);
	return true;
}
bool Data::remove(const std::shared_ptr<Mesh> mesh) {
	auto found = std::find_if(begin(mesh_), end(mesh_), [mesh](const std::pair<std::string, std::shared_ptr<Mesh>> m) {
		return m.second == mesh;
	});
	if(found == std::end(mesh_)) {
		return false;
	}
	mesh_.erase(found);
	return true;
}


bool Data::isVisible(std::shared_ptr<Mesh> mesh) const
{
	return std::any_of(begin(mesh_), end(mesh_), [](const std::pair<std::string, std::shared_ptr<Data::Mesh>> m) {
		return m.second->is_solo;
	}) ? mesh->is_solo && !mesh->is_hidden : !mesh->is_hidden;
}
bool Data::isEditable(std::shared_ptr<Mesh> mesh, bool include_hidden) const
{
	return !mesh->is_locked && (include_hidden || isVisible(mesh));
}

std::pair<std::string, std::shared_ptr<Data::Mesh>> Data::find(std::shared_ptr<geom::Quad> quad)
{
	auto found = std::find_if(begin(mesh_), end(mesh_), [quad](const std::pair<std::string, std::shared_ptr<Mesh>> m) {
		return m.second->uv_quad == quad;
	});
	if(found == std::end(mesh_)) {
		return {"", nullptr};
	}
	return *found;
}

std::pair<std::string, std::shared_ptr<Data::Mesh>> Data::find(std::shared_ptr<ofx::mapper::Mesh> mesh)
{
	auto found = std::find_if(begin(mesh_), end(mesh_), [mesh](const std::pair<std::string, std::shared_ptr<Mesh>> m) {
		return m.second->mesh == mesh;
	});
	if(found == std::end(mesh_)) {
		return {"", nullptr};
	}
	return *found;
}

std::map<std::string, std::shared_ptr<Data::Mesh>> Data::getVisibleMesh() const
{
	std::map<std::string, std::shared_ptr<Data::Mesh>> ret;
	auto isVisible = std::any_of(begin(mesh_), end(mesh_), [](const std::pair<std::string, std::shared_ptr<Data::Mesh>> m) {
		return m.second->is_solo;
	}) ? [](std::shared_ptr<Data::Mesh> m) {
		return m->is_solo && !m->is_hidden;
	} : [](std::shared_ptr<Data::Mesh> m) {
		return !m->is_hidden;
	};
	for(auto &&m : mesh_) {
		if(isVisible(m.second)) {
			ret.insert(m);
		}
	}
	return ret;
}
std::map<std::string, std::shared_ptr<Data::Mesh>> Data::getEditableMesh(bool include_hidden) const
{
	std::map<std::string, std::shared_ptr<Data::Mesh>> ret;
	auto isVisible = std::any_of(begin(mesh_), end(mesh_), [](const std::pair<std::string, std::shared_ptr<Data::Mesh>> m) {
		return m.second->is_solo;
	}) ? [](std::shared_ptr<Data::Mesh> m) {
		return m->is_solo && !m->is_hidden;
	} : [](std::shared_ptr<Data::Mesh> m) {
		return !m->is_hidden;
	};
	for(auto &&m : mesh_) {
		if(!m.second->is_locked && (include_hidden || isVisible(m.second))) {
			ret.insert(m);
		}
	}
	return ret;
}

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

void Data::exportMesh(const std::filesystem::path &filepath, float resample_min_interval, const glm::vec2 &coord_size, bool only_visible) const
{
	getMeshForExport(resample_min_interval, coord_size).save(filepath);
}

ofMesh Data::getMeshForExport(float resample_min_interval, const glm::vec2 &coord_size, bool only_visible) const
{
	ofMesh ret;
	for(auto &&m : only_visible ? getVisibleMesh() : mesh_) {
		m.second->setDirty();	// force clear cache
		ret.append(m.second->getMesh(resample_min_interval, coord_size));
	}
	return ret;
}

void Data::save(const std::filesystem::path &filepath) const
{
	ofFile file(filepath, ofFile::WriteOnly);
	pack(file);
	file.close();
}
void Data::load(const std::filesystem::path &filepath)
{
	mesh_.clear();
	ofFile file(filepath);
	unpack(file);
	file.close();
}

void Data::pack(std::ostream &stream) const
{
	writeTo(stream, mesh_.size());
	const int name_alignemt = 4;
	for(auto &&m : mesh_) {
		auto name = m.first;
		std::size_t name_size = name.size();
		std::size_t pad_size = std::ceil(name_size/(float)name_alignemt) * name_alignemt;
		writeTo(stream, name.size());
		stream.write(name.c_str(), pad_size);
		m.second->pack(stream);
	}
}
void Data::unpack(std::istream &stream)
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
		auto mesh = std::make_shared<Mesh>();
		mesh->unpack(stream);
		mesh_.insert(std::make_pair(name, mesh));
	}
}

void Data::Mesh::pack(std::ostream &stream) const
{
	writeTo(stream, is_hidden);
	writeTo(stream, is_locked);
	writeTo(stream, is_solo);
	for(int i = 0; i < uv_quad->size(); ++i) {
		writeTo(stream, uv_quad->pt[i].x);
		writeTo(stream, uv_quad->pt[i].y);
	}
	mesh->pack(stream, interpolator.get());
}
void Data::Mesh::unpack(std::istream &stream)
{
	readFrom(stream, is_hidden);
	readFrom(stream, is_locked);
	readFrom(stream, is_solo);
	for(int i = 0; i < uv_quad->size(); ++i) {
		readFrom(stream, uv_quad->pt[i].x);
		readFrom(stream, uv_quad->pt[i].y);
	}
	mesh->unpack(stream, interpolator.get());
}

