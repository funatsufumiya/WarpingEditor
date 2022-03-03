#pragma once

#include "ofMain.h"
#include "ofxImGui.h"
#include "WarpingMeshEditor.h"
#include "WarpingUVEditor.h"
#include "BlendingMeshEditor.h"
#include "BlendingUVEditor.h"
#include "ImageSource.h"
#include "ofxNDIFinder.h"
#include "WorkFolder.h"
#include "ProjectFolder.h"

class ResultView;

class WarpingApp : public ofBaseApp
{
public:
	void setup() override;
	void update() override;
	void draw() override;
	void dragEvent(ofDragInfo dragInfo) override;
	
	void save(bool do_backup=true) const;
	void openRecent(int index=0);
	void openProject(const std::filesystem::path &proj_path);
	
	void keyPressed(int key) override;
	void setResultApp(std::shared_ptr<ResultView> app) { result_app_ = app; }
	void setResultWindow(std::shared_ptr<ofAppBaseWindow> window) { result_window_ = window; }
private:
	enum State {
		EDIT_WARP_UV,
		EDIT_WARP_MESH,
		EDIT_BLEND_UV,
		EDIT_BLEND_MESH,
	};
	int state_;
	bool isStateWarping() const { return state_ == EDIT_WARP_UV || state_ == EDIT_WARP_MESH; }
	bool isStateBlending() const { return state_ == EDIT_BLEND_UV || state_ == EDIT_BLEND_MESH; }
	std::vector<EditorBase*> editor_;

	std::shared_ptr<MeshData> warping_data_;
	WarpingUVEditor warp_uv_;
	WarpingMeshEditor warp_mesh_;

	std::shared_ptr<MeshData> blending_data_;
	BlendingUVEditor blend_uv_;
	BlendingMeshEditor blend_mesh_;
	
	std::shared_ptr<ResultView> result_app_;
	std::shared_ptr<ofAppBaseWindow> result_window_;
	ofxImGui::Gui gui_;
	std::shared_ptr<ImageSource> texture_source_;
	
	void backup();
	void loadRecent();
	void updateRecent(const ProjectFolder &proj);
	std::deque<WorkFolder> recent_;
	
	void exportMesh(float resample_min_interval, const std::filesystem::path &filepath, bool is_arb=false) const;
	void exportMesh(const ProjectFolder &proj) const;

	ofxNDIFinder ndi_finder_;
	
	mutable ProjectFolder proj_;
};

class ResultView : public ofBaseApp
{
public:
	void setup() override;
	void update() override;
	void draw() override;
	
	void setTexture(ofTexture texture) { texture_ = texture; }
	void setMesh(const ofMesh &mesh) { mesh_ = mesh; }
private:
	ofMesh mesh_;
	ofTexture texture_;
};
