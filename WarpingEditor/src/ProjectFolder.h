#pragma once

#include "WorkFolder.h"
#include "ImageSource.h"

class ProjectFolder : public WorkFolder
{
public:
	void setup() {
		if(isValid()) {
			load();
		}
		else {
			setRelative("");
		}
	}
	void load() {
		settings_ = ofLoadJson(getAbsolute(getProjFileName()));
		Data::shared().load(getAbsolute(getFileName()+".bin"));
	}
	void save() const {
		Data::shared().save(getAbsolute(getFileName()+".bin"));
		ofSavePrettyJson(getAbsolute(getProjFileName()), settings_);
	}
	void backup() const {
		auto bu = settings_["backup"];
		if(!bu["enabled"].get<bool>()) {
			return;
		}
		ofDirectory folder(getRelative(bu["folder"].get<std::string>()));
		if(!folder.exists()) {
			folder.create();
		}
		std::string filename = getFileName()+"_"+ofGetTimestampString("%Y%m%d_%H%M%S") + ".bin";
		ofFile(getAbsolute(getFileName()+".bin")).copyTo(ofFilePath::join(folder.path(), filename));
		int num = bu["limit"];
		if(num > 0) {
			folder.sortByDate();
			for(int i = 0; i < folder.size() - num; ++i) {
				folder.getFile(i).remove();
			}
		}
	}
	std::string getFileName() const { return settings_["filename"]; }
	std::string getProjFileName() const { return "project.json"; }
	std::shared_ptr<ImageSource> buildTextureSource() const {
		return buildTextureSource(settings_["texture"]["type"].get<std::string>(), settings_["texture"]["arg"].get<std::string>());
	}
	ofRectangle getMainViewport() const { return ofRectangle{settings_["viewport"]["main"][0],settings_["viewport"]["main"][1],settings_["viewport"]["main"][2],settings_["viewport"]["main"][3]}; }
	std::pair<glm::vec2, float> getUVView() const { return {{settings_["viewport"]["uv_editor"]["pos"][0],settings_["viewport"]["uv_editor"]["pos"][1]}, settings_["viewport"]["uv_editor"]["scale"]}; }
	std::pair<glm::vec2, float> getWarpView() const { return {{settings_["viewport"]["warp_editor"]["pos"][0], settings_["viewport"]["warp_editor"]["pos"][1]}, settings_["viewport"]["warp_editor"]["scale"]}; }
	float getExportMeshMinInterval() const { return settings_["export"]["max_mesh_size"]; }
	void setTextureSource(const std::string &type, const std::string &arg) {
		settings_["texture"]["type"] = type;
		if(type == "File") {
			settings_["texture"]["arg"] = getRelative(arg).string();
		}
		else {
			settings_["texture"]["arg"] = arg;
		}
	}
	void setMainViewport(const ofRectangle &viewport) {
		settings_["viewport"]["main"] = std::vector<float>{viewport.x,viewport.y,viewport.width,viewport.height};
	}
	void setUVView(const glm::vec2 &pos, float scale) {
		settings_["viewport"]["uv_editor"]["pos"] = std::vector<float>{pos.x, pos.y};
		settings_["viewport"]["uv_editor"]["scale"] = scale;
	}
	void setWarpView(const glm::vec2 &pos, float scale) {
		settings_["viewport"]["warp_editor"]["pos"] = std::vector<float>{pos.x, pos.y};
		settings_["viewport"]["warp_editor"]["scale"] = scale;
	}
	void setExportMeshMinInterval(float interval) {
		settings_["export"]["max_mesh_size"] = interval;
	}
	void setFileName(const std::string &filename) {
		settings_["filename"] = filename;
	}
private:
	ofJson settings_={
		{"texture", {
			{ "type", "File" },
			{ "arg", "test.png" }
		}},
		{"viewport", {
			{"main", {0,0,1920,1080}},
			{"uv_editor", {
				{"pos", {0,0}},
				{"scale", 1}
			}},
			{"warp_editor", {
				{"pos", {0,0}},
				{"scale", 1}
			}}
		}},
		{"export", {
			{"max_mesh_size", 100}
		}},
		{"backup", {
			{"enabled", true},
			{"folder", "backup"},
			{"limit", 0}
		}},
		{"filename", "data"}
	};

private:
	std::shared_ptr<ImageSource> buildTextureSource(const std::string &type, const std::string &arg) const {
		std::shared_ptr<ImageSource> ret = std::make_shared<ImageSource>();
		auto upper_type = ofToUpper(type);
		if(upper_type == "FILE") {
			if(ret->loadFromFile(getAbsolute(arg))) {
				return ret;
			}
		}
		else if(upper_type == "NDI") {
			if(ret->setupNDI(arg)) {
				return ret;
			}
		}
		return nullptr;
	}
};
