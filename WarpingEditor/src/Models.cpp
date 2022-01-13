#include "Models.h"

std::pair<std::string, std::shared_ptr<Data::Mesh>> Data::create(const std::string &name) {
	std::string n = name;
	int index=0;
	auto m = std::make_shared<Mesh>();
	while(!add(n, m)) {
		n = name+ofToString(index++);
	}
	m->init({0,0,640, 480});
	return std::make_pair(n, m);
}
void Data::update() {
	std::for_each(begin(mesh_), end(mesh_), [](const std::pair<std::string, std::shared_ptr<Mesh>> m) {
		m.second->update();
	});
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

std::shared_ptr<Data::Mesh> Data::find(std::shared_ptr<ofx::mapper::Mesh> mesh)
{
	auto found = std::find_if(begin(mesh_), end(mesh_), [mesh](const std::pair<std::string, std::shared_ptr<Mesh>> m) {
		return m.second->mesh == mesh;
	});
	if(found == std::end(mesh_)) {
		return nullptr;
	}
	return found->second;
}

std::map<std::string, std::shared_ptr<Data::Mesh>> Data::getVisibleMesh()
{
	std::map<std::string, std::shared_ptr<Data::Mesh>> ret;
	auto isVisible = std::any_of(begin(mesh_), end(mesh_), [](const std::pair<std::string, std::shared_ptr<Data::Mesh>> m) {
		return m.second->is_solo;
	}) ? [](const std::pair<std::string, std::shared_ptr<Data::Mesh>> m) {
		return m.second->is_solo && !m.second->is_hidden;
	} : [](const std::pair<std::string, std::shared_ptr<Data::Mesh>> m) {
		return !m.second->is_hidden;
	};
	for(auto &&m : mesh_) {
		if(isVisible(m)) {
			ret.insert(m);
		}
	}
	return ret;
}
std::map<std::string, std::shared_ptr<Data::Mesh>> Data::getEditableMesh(bool include_hidden)
{
	std::map<std::string, std::shared_ptr<Data::Mesh>> ret;
	auto isVisible = std::any_of(begin(mesh_), end(mesh_), [](const std::pair<std::string, std::shared_ptr<Data::Mesh>> m) {
		return m.second->is_solo;
	}) ? [](const std::pair<std::string, std::shared_ptr<Data::Mesh>> m) {
		return m.second->is_solo && !m.second->is_hidden;
	} : [](const std::pair<std::string, std::shared_ptr<Data::Mesh>> m) {
		return !m.second->is_hidden;
	};
	for(auto &&m : mesh_) {
		if(!m.second->is_locked && (include_hidden || isVisible(m))) {
			ret.insert(m);
		}
	}
	return ret;
}
