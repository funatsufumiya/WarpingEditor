#pragma once

#include "WorkFolder.h"
#include "ImageSource.h"

class ProjectFolder
{
public:
	void setup(WorkFolder work) {
		if(work_.isValid()) {
			settings_ = ofLoadJson(work.getAbsolute("project.json"));
		}
		else {
			work.setRelative("");
		}
		work_ = work;
	}
	void save() const {
		ofSavePrettyJson(work_.getAbsolute("project.json"), settings_);
	}
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
			settings_["texture"]["arg"] = work_.getRelative(arg).string();
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
private:
	WorkFolder work_;
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
		}}
	};

private:
	std::shared_ptr<ImageSource> buildTextureSource(const std::string &type, const std::string &arg) const {
		std::shared_ptr<ImageSource> ret = std::make_shared<ImageSource>();
		auto upper_type = ofToUpper(type);
		if(upper_type == "FILE") {
			if(ret->loadFromFile(work_.getAbsolute(arg))) {
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
