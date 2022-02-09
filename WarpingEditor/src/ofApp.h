#pragma once

#include "ofMain.h"
#include "ofxImGui.h"
#include "WarpingEditor.h"
#include "UVEditor.h"
#include "ImageSource.h"
#include "ofxNDIFinder.h"
#include "WorkFolder.h"
#include "ProjectFolder.h"

class MainApp;

class GuiApp : public ofBaseApp
{
public:
	void setup();
	void update();
	void draw();
	void dragEvent(ofDragInfo dragInfo) override;
	
	void save(bool do_backup=true) const;
	void openRecent(int index=0);
	void openProject(const std::filesystem::path &proj_path);
	
	void keyPressed(int key);
	void setMainApp(std::shared_ptr<MainApp> app) { main_app_ = app; }
	void setMainWindow(std::shared_ptr<ofAppBaseWindow> window) { main_window_ = window; }
private:
	std::shared_ptr<MainApp> main_app_;
	std::shared_ptr<ofAppBaseWindow> main_window_;
	ofxImGui::Gui gui_;
	std::shared_ptr<ImageSource> texture_source_;
	WarpingEditor warp_;
	UVEditor uv_;
	enum State {
		EDIT_UV,
		EDIT_WRAP
	};
	int state_=EDIT_UV;
	std::vector<EditorBase*> editor_;
	
	void backup();
	void loadRecent();
	void updateRecent(const ProjectFolder &proj);
	std::deque<WorkFolder> recent_;
	
	void exportMesh(float resample_min_interval, const std::filesystem::path &filepath, bool is_arb=false) const;
	void exportMesh(const ProjectFolder &proj) const;

	
	ofxNDIFinder ndi_finder_;
	
	mutable ProjectFolder proj_;
};

class MainApp : public ofBaseApp
{
public:
	void setup();
	void update();
	void draw();
	
	void setTexture(ofTexture texture) { texture_ = texture; }
	void setMesh(const ofMesh &mesh) { mesh_ = mesh; }
private:
	ofMesh mesh_;
	ofTexture texture_;
};
