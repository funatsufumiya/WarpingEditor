#pragma once

#include "WorkFolder.h"
#include "Editor.h"

class ProjectFolder : public WorkFolder
{
public:
	void setup();
	void load();
	void save() const;
	void backup() const;
	
	std::filesystem::path getDataFilePath() const { return getAbsolute(getDataFileName()+".maap"); }
	std::string getDataFileName() const { return filename_; }
	std::string getProjFileName() const { return "project.json"; }
	
	int getTextureType() const { return texture_.type; }
	std::filesystem::path getTextureFilePath() const { return getAbsolute(texture_.file); }
	const std::string& getTextureNDIName() const { return texture_.ndi; }
	glm::vec2 getTextureSizeCache() const { return texture_.size_cache; }

	glm::vec4 getResultViewport() const { return viewport_.result; }
	std::pair<glm::vec2, float> getUVView() const { return viewport_.uv; }
	std::pair<glm::vec2, float> getWarpView() const { return viewport_.warp; }
	std::pair<glm::vec2, float> getBlendView() const { return viewport_.blend; }

	std::string getResultEditorName() const { return result_.editor_name; }
	bool isResultScaleToViewport() const { return result_.is_scale_to_viewport; }
	bool isResultShowControl() const { return result_.is_show_control; }
	
	glm::ivec2 getBridgeResolution() const { return bridge_.resolution; }
	
	float getExportMeshMinInterval() const { return export_.max_mesh_size; }
	std::string getExportFolder() const { return export_.folder; }
	std::string getExportFileName() const { return export_.filename; }
	bool getIsExportMeshArb() const { return export_.is_arb; }
	
	bool isBackupEnabled() const { return backup_.enabled; }
	std::filesystem::path getBackupFolder() const { return getRelative(backup_.folder); }
	std::filesystem::path getBackupFilePath() const;
	int getBackupNumLimit() const { return backup_.limit; }
	
	EditorBase::GridData getUVGridData() const { return grid_.uv; }
	EditorBase::GridData getWarpGridData() const { return grid_.warp; }
	EditorBase::GridData getBlendGridData() const { return grid_.blend; }
	
	void setTextureSourceFile(const std::string &file_name);
	void setTextureSourceNDI(const std::string &ndi_name);
	void setTextureSizeCache(const glm::vec2 size) { texture_.size_cache = size; }
	
	void setResultViewport(const glm::vec4 &viewport) { viewport_.result = viewport; }
	void setUVView(const glm::vec2 &pos, float scale) { viewport_.uv = {pos, scale}; }
	void setWarpView(const glm::vec2 &pos, float scale) { viewport_.warp = {pos, scale}; }
	void setBlendView(const glm::vec2 &pos, float scale) { viewport_.blend = {pos, scale}; }
	
	void setResultEditorName(const std::string &name) { result_.editor_name = name; }
	void setResultScaleToViewport(bool enable) { result_.is_scale_to_viewport = enable; }
	void setResultShowControl(bool enable) { result_.is_show_control = enable; }
	
	void setBridgeResolution(glm::ivec2 resolution) { bridge_.resolution = resolution; }

	void setExportMeshMinInterval(float interval) { export_.max_mesh_size = interval; }
	void setExportFolder(const std::string &folder) { export_.folder = folder; }
	void setExportFileName(const std::string &filename) { export_.filename = filename; }
	void setIsExportMeshArb(bool is_arb) { export_.is_arb = is_arb; }

	void setUVGridData(const EditorBase::GridData &data) { grid_.uv = data; }
	void setWarpGridData(const EditorBase::GridData &data) { grid_.warp = data; }
	void setBlendGridData(const EditorBase::GridData &data) { grid_.blend = data; }
	
	void setFileName(const std::string &filename) { filename_ = filename; }
	
	struct Texture {
		enum {
			FILE, NDI
		};
		int type = FILE;
		std::string file;
		std::string ndi;
		glm::ivec2 size_cache;
	};
	struct Viewport {
		glm::vec4 result={0,0,1920,1080};
		std::pair<glm::vec2, float> uv{{0,0},1}, warp{{0,0},1}, blend{{0,0},1};
	};
	struct Export {
		std::string folder, filename="mesh.ply";
		float max_mesh_size=100;
		bool is_arb=false;
	};
	struct Backup {
		bool enabled=true;
		std::string folder;
		int limit=0;
	};
	struct Grid {
		EditorBase::GridData uv, warp, blend;
	};
	struct Bridge {
		glm::ivec2 resolution={1920,1080};
	};
	struct Result {
		std::string editor_name="uv";
		bool is_scale_to_viewport=false;
		bool is_show_control=false;
	};
private:
	Texture texture_;
	Viewport viewport_;
	Export export_;
	Backup backup_;
	Grid grid_;
	Bridge bridge_;
	Result result_;
	std::string filename_;
	
	ofJson toJson() const;
	void loadJson(const ofJson &json);
};
