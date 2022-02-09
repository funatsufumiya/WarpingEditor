#include "ProjectFolder.h"

namespace {
template<typename T>
auto getJsonValue(const ofJson &json, const std::string &key, T default_value={}) -> T {
	return json.contains(key) ? json[key].get<T>() : default_value;
}
template<typename T>
void updateByJsonValue(T &t, const ofJson &json, const std::string &key) {
	t = getJsonValue<T>(json, key, t);
}
}

namespace nlohmann {
template<>
struct adl_serializer<ProjectFolder::Texture> {
	static void to_json(ofJson &j, const ProjectFolder::Texture &v) {
		switch(v.type) {
			case ProjectFolder::Texture::FILE:
				j["type"] = "File";
				j["arg"] = v.file;
				break;
			case ProjectFolder::Texture::NDI:
				j["type"] = "NDI";
				j["arg"] = v.ndi;
				break;
		}
	}
	static void from_json(const ofJson &j, ProjectFolder::Texture &v) {
		auto upper_type = ofToUpper(getJsonValue<std::string>(j, "type", "File"));
		if(upper_type == "FILE") {
			updateByJsonValue(v.file, j, "arg");
		}
		else if(upper_type == "NDI") {
			updateByJsonValue(v.ndi, j, "arg");
		}
	}
};
template<>
struct adl_serializer<glm::vec4> {
	static void to_json(ofJson &j, const glm::vec4 &v) {
		j = {v[0],v[1],v[2],v[3]};
	}
	static void from_json(const ofJson &j, glm::vec4 &v) {
		v = {j[0],j[1],j[2],j[3]};
	}
};
template<>
struct adl_serializer<glm::vec2> {
	static void to_json(ofJson &j, const glm::vec2 &v) {
		j = {v[0],v[1]};
	}
	static void from_json(const ofJson &j, glm::vec2 &v) {
		v = {j[0],j[1]};
	}
};
template<>
struct adl_serializer<ProjectFolder::Viewport> {
	static void to_json(ofJson &j, const ProjectFolder::Viewport &v) {
		j = {
			{"main", v.main},
			{"uv", {
				{"pos", v.uv.first},
				{"scale", v.uv.second}
			}},
			{"warp", {
				{"pos", v.warp.first},
				{"scale", v.warp.second}
			}}
		};
	}
	static void from_json(const ofJson &j, ProjectFolder::Viewport &v) {
		updateByJsonValue(v.main, j, "main");
		updateByJsonValue(v.uv.first, j["uv"], "pos");
		updateByJsonValue(v.uv.second, j["uv"], "scale");
		updateByJsonValue(v.warp.first, j["warp"], "pos");
		updateByJsonValue(v.warp.second, j["warp"], "scale");
	}
};

template<>
struct adl_serializer<ProjectFolder::Export> {
	static void to_json(ofJson &j, const ProjectFolder::Export &v) {
		j = {
			{"folder", v.folder},
			{"filename", v.filename},
			{"is_arb", v.is_arb},
			{"max_mesh_size", v.max_mesh_size}
		};
	}
	static void from_json(const ofJson &j, ProjectFolder::Export &v) {
		updateByJsonValue(v.folder, j, "folder");
		updateByJsonValue(v.filename, j, "filename");
		updateByJsonValue(v.is_arb, j, "is_arb");
		updateByJsonValue(v.max_mesh_size, j, "max_mesh_size");
	}
};
template<>
struct adl_serializer<ProjectFolder::Backup> {
	static void to_json(ofJson &j, const ProjectFolder::Backup &v) {
		j = {
			{"enabled", v.enabled},
			{"folder", v.folder},
			{"limit", v.limit}
		};
	}
	static void from_json(const ofJson &j, ProjectFolder::Backup &v) {
		updateByJsonValue(v.enabled, j, "enabled");
		updateByJsonValue(v.folder, j, "folder");
		updateByJsonValue(v.limit, j, "limit");
	}
};
}

ofJson ProjectFolder::toJson() const
{
	return {
		{"texture", texture_},
		{"viewport", viewport_},
		{"export", export_},
		{"backup", backup_},
		{"filename", filename_}
	};
}
void ProjectFolder::loadJson(const ofJson &json)
{
	updateByJsonValue(texture_, json, "texture");
	updateByJsonValue(viewport_, json, "viewport");
	updateByJsonValue(export_, json, "export");
	updateByJsonValue(backup_, json, "backup");
	updateByJsonValue(filename_, json, "filename");
}

void ProjectFolder::setup() {
	if(isValid()) {
		load();
	}
	else {
		setRelative("");
	}
}
void ProjectFolder::load() {
	loadJson(ofLoadJson(getAbsolute(getProjFileName())));
}
void ProjectFolder::save() const {
	ofSavePrettyJson(getAbsolute(getProjFileName()), toJson());
}
void ProjectFolder::backup() const {
	auto bu = backup_;
	if(!bu.enabled) {
		return;
	}
	ofDirectory folder(getRelative(bu.folder));
	if(!folder.exists()) {
		folder.create();
	}
	std::string filename = getFileName()+"_"+ofGetTimestampString("%Y%m%d_%H%M%S") + ".bin";
	ofFile(getAbsolute(getFileName()+".bin")).copyTo(ofFilePath::join(folder.path(), filename));
	int num = bu.limit;
	if(num > 0) {
		folder.sortByDate();
		for(int i = 0; i < folder.size() - num; ++i) {
			folder.getFile(i).remove();
		}
	}
}

void ProjectFolder::setTextureSourceFile(const std::string &file_name)
{
	texture_.type = Texture::FILE;
	texture_.file = file_name;
}
void ProjectFolder::setTextureSourceNDI(const std::string &ndi_name)
{
	texture_.type = Texture::NDI;
	texture_.ndi = ndi_name;
}
