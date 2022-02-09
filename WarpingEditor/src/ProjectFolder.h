#pragma once

#include "WorkFolder.h"

class ProjectFolder : public WorkFolder
{
public:
	void setup();
	void load();
	void save() const;
	void backup() const;
	
	std::filesystem::path getDataPath() const { return getAbsolute(getFileName()+".bin"); }
	std::string getFileName() const { return filename_; }
	std::string getProjFileName() const { return "project.json"; }
	
	int getTextureType() const { return texture_.type; }
	std::filesystem::path getTextureFilePath() const { return getAbsolute(texture_.file); }
	const std::string& getTextureNDIName() const { return texture_.ndi; }
	
	glm::vec4 getMainViewport() const { return viewport_.main; }
	std::pair<glm::vec2, float> getUVView() const { return viewport_.uv; }
	std::pair<glm::vec2, float> getWarpView() const { return viewport_.warp; }
	
	float getExportMeshMinInterval() const { return export_.max_mesh_size; }
	std::string getExportFolder() const { return export_.folder; }
	std::string getExportFileName() const { return export_.filename; }
	bool getIsExportMeshArb() const { return export_.is_arb; }
	
	void setTextureSourceFile(const std::string &file_name);
	void setTextureSourceNDI(const std::string &ndi_name);
	
	void setMainViewport(const glm::vec4 &viewport) { viewport_.main = viewport; }
	void setUVView(const glm::vec2 &pos, float scale) { viewport_.uv = {pos, scale}; }
	void setWarpView(const glm::vec2 &pos, float scale) { viewport_.warp = {pos, scale}; }
	
	void setExportMeshMinInterval(float interval) { export_.max_mesh_size = interval; }
	void setExportFolder(const std::string &folder) { export_.folder = folder; }
	void setExportFileName(const std::string &filename) { export_.filename = filename; }
	void setIsExportMeshArb(bool is_arb) { export_.is_arb = is_arb; }
	
	void setFileName(const std::string &filename) { filename_ = filename; }
	
	struct Texture {
		enum {
			FILE, NDI
		};
		int type = FILE;
		std::string file;
		std::string ndi;
	};
	struct Viewport {
		glm::vec4 main;
		std::pair<glm::vec2, float> uv{{0,0},1}, warp{{0,0},1};
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
private:
	Texture texture_;
	Viewport viewport_;
	Export export_;
	Backup backup_;
	std::string filename_;
	
	ofJson toJson() const;
	void loadJson(const ofJson &json);
};
