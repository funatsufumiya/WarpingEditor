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
template<typename T>
struct adl_serializer<glm::tvec4<T>> {
	static void to_json(ofJson &j, const glm::tvec4<T> &v) {
		j = {v[0],v[1],v[2],v[3]};
	}
	static void from_json(const ofJson &j, glm::tvec4<T> &v) {
		v = {j[0],j[1],j[2],j[3]};
	}
};
template<typename T>
struct adl_serializer<glm::tvec3<T>> {
	static void to_json(ofJson &j, const glm::tvec3<T> &v) {
		j = {v[0],v[1],v[2]};
	}
	static void from_json(const ofJson &j, glm::tvec3<T> &v) {
		v = {j[0],j[1],j[2]};
	}
};
template<typename T>
struct adl_serializer<glm::tvec2<T>> {
	static void to_json(ofJson &j, const glm::tvec2<T> &v) {
		j = {v[0],v[1]};
	}
	static void from_json(const ofJson &j, glm::tvec2<T> &v) {
		v = {j[0],j[1]};
	}
};
template<>
struct adl_serializer<EditorBase::GridData> {
	static void to_json(ofJson &j, const EditorBase::GridData &v) {
		j = {
			{"show", v.is_show},
			{"snap", v.enabled_snap},
			{"offset", v.offset},
			{"size", v.size}
		};
	}
	static void from_json(const ofJson &j, EditorBase::GridData &v) {
		updateByJsonValue(v.is_show, j, "show");
		updateByJsonValue(v.enabled_snap, j, "snap");
		updateByJsonValue(v.offset, j, "offset");
		updateByJsonValue(v.size, j, "size");
	}
};
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
		j["size_cache"] = v.size_cache;
	}
	static void from_json(const ofJson &j, ProjectFolder::Texture &v) {
		auto upper_type = ofToUpper(getJsonValue<std::string>(j, "type", "File"));
		if(upper_type == "FILE") {
			updateByJsonValue(v.file, j, "arg");
		}
		else if(upper_type == "NDI") {
			updateByJsonValue(v.ndi, j, "arg");
		}
		updateByJsonValue(v.size_cache, j, "size_cache");
	}
};
template<>
struct adl_serializer<ProjectFolder::Viewport> {
	static void to_json(ofJson &j, const ProjectFolder::Viewport &v) {
		j = {
			{"result", v.result},
			{"uv", {
				{"pos", v.uv.first},
				{"scale", v.uv.second}
			}},
			{"warp", {
				{"pos", v.warp.first},
				{"scale", v.warp.second}
			}},
			{"blend", {
				{"pos", v.blend.first},
				{"scale", v.blend.second}
			}}
		};
	}
	static void from_json(const ofJson &j, ProjectFolder::Viewport &v) {
		updateByJsonValue(v.result, j, "result");
		auto uv = getJsonValue<ofJson>(j, "uv");
		updateByJsonValue(v.uv.first, uv, "pos");
		updateByJsonValue(v.uv.second, uv, "scale");
		auto warp = getJsonValue<ofJson>(j, "warp");
		updateByJsonValue(v.warp.first, warp, "pos");
		updateByJsonValue(v.warp.second, warp, "scale");
		auto blend = getJsonValue<ofJson>(j, "blend");
		updateByJsonValue(v.blend.first, blend, "pos");
		updateByJsonValue(v.blend.second, blend, "scale");
	}
};

template<>
struct adl_serializer<ProjectFolder::Export::Mesh> {
	static void to_json(ofJson &j, const ProjectFolder::Export::Mesh &v) {
		j = {
			{"filename", v.filename},
			{"max_mesh_size", v.max_mesh_size}
		};
	}
	static void from_json(const ofJson &j, ProjectFolder::Export::Mesh &v) {
		updateByJsonValue(v.filename, j, "filename");
		updateByJsonValue(v.max_mesh_size, j, "max_mesh_size");
	}
};
template<>
struct adl_serializer<ProjectFolder::Export::BlendShader> {
	static void to_json(ofJson &j, const ProjectFolder::Export::BlendShader &v) {
		j = {
			{"filename", v.filename}
		};
	}
	static void from_json(const ofJson &j, ProjectFolder::Export::BlendShader &v) {
		updateByJsonValue(v.filename, j, "filename");
	}
};

template<>
struct adl_serializer<ProjectFolder::Export> {
	static void to_json(ofJson &j, const ProjectFolder::Export &v) {
		j = {
			{"folder", v.folder},
			{"is_arb", v.is_arb},
			{"warp", v.warp},
			{"blend", v.blend},
			{"blend_shader", v.blend_shader}
		};
	}
	static void from_json(const ofJson &j, ProjectFolder::Export &v) {
		updateByJsonValue(v.folder, j, "folder");
		updateByJsonValue(v.is_arb, j, "is_arb");
		updateByJsonValue(v.warp, j, "warp");
		updateByJsonValue(v.blend, j, "blend");
		updateByJsonValue(v.blend_shader, j, "blend_shader");
	}
};
template<>
struct adl_serializer<ProjectFolder::Bridge> {
	static void to_json(ofJson &j, const ProjectFolder::Bridge &v) {
		j = {
			{"resolution", v.resolution}
		};
	}
	static void from_json(const ofJson &j, ProjectFolder::Bridge &v) {
		updateByJsonValue(v.resolution, j, "resolution");
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
template<>
struct adl_serializer<ProjectFolder::Grid> {
	static void to_json(ofJson &j, const ProjectFolder::Grid &v) {
		j = {
			{"uv", v.uv},
			{"warp", v.warp},
			{"blend", v.blend}
		};
	}
	static void from_json(const ofJson &j, ProjectFolder::Grid &v) {
		updateByJsonValue(v.uv, j, "uv");
		updateByJsonValue(v.warp, j, "warp");
		updateByJsonValue(v.blend, j, "blend");
	}
};
template<>
struct adl_serializer<ProjectFolder::Result> {
	static void to_json(ofJson &j, const ProjectFolder::Result &v) {
		j = {
			{"editor_name", v.editor_name},
			{"is_scale_to_viewport", v.is_scale_to_viewport},
			{"is_show_control", v.is_show_control}
		};
	}
	static void from_json(const ofJson &j, ProjectFolder::Result &v) {
		updateByJsonValue(v.editor_name, j, "editor_name");
		updateByJsonValue(v.is_scale_to_viewport, j, "is_scale_to_viewport");
		updateByJsonValue(v.is_show_control, j, "is_show_control");
	}
};
template<>
struct adl_serializer<ofxBlendScreen::Shader::Params> {
	static void to_json(ofJson &j, const ofxBlendScreen::Shader::Params &v) {
		j = {
			{"gamma", v.gamma},
			{"luminance_control", v.luminance_control},
			{"blend_power", v.blend_power},
			{"base_color", v.base_color}
		};
	}
	static void from_json(const ofJson &j, ofxBlendScreen::Shader::Params &v) {
		updateByJsonValue(v.gamma, j, "gamma");
		updateByJsonValue(v.luminance_control, j, "luminance_control");
		updateByJsonValue(v.blend_power, j, "blend_power");
		updateByJsonValue(v.base_color, j, "base_color");
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
		{"bridge", bridge_},
		{"grid", grid_},
		{"filename", filename_},
		{"result", result_},
		{"blend_params", blend_params_}
	};
}
void ProjectFolder::loadJson(const ofJson &json)
{
	updateByJsonValue(texture_, json, "texture");
	updateByJsonValue(viewport_, json, "viewport");
	updateByJsonValue(export_, json, "export");
	updateByJsonValue(backup_, json, "backup");
	updateByJsonValue(bridge_, json, "bridge");
	updateByJsonValue(grid_, json, "grid");
	updateByJsonValue(filename_, json, "filename");
	updateByJsonValue(result_, json, "result");
	updateByJsonValue(blend_params_, json, "blend_params");
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

std::filesystem::path ProjectFolder::getBackupFilePath() const
{
	auto path = getDataFilePath();
	auto folder = ofFilePath::join(ofFilePath::getEnclosingDirectory(path), backup_.folder);
	auto basename = ofFilePath::getBaseName(path);
	auto ext = ofFilePath::getFileExt(path);
	return ofFilePath::join(folder, basename+"_"+ofGetTimestampString("%Y%m%d_%H%M%S"))+"."+ext;
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

