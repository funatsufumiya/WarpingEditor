#pragma once

#include "ofMain.h"
#include "ofxImGui.h"
#include "WarpingMeshEditor.h"
#include "WarpingUVEditor.h"
#include "BlendingEditor.h"
#include "ImageSource.h"
#include "ofxNDIFinder.h"
#include "WorkFolder.h"
#include "ProjectFolder.h"

class ResultView;

class GuiApp : public ofBaseApp
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
		EDIT_BLEND,
		NUM_STATE
	};
	int state_;
	bool isStateWarping() const { return state_ == EDIT_WARP_UV || state_ == EDIT_WARP_MESH; }
	bool isStateBlending() const { return state_ == EDIT_BLEND; }
	std::vector<EditorBase*> editor_;

	std::shared_ptr<WarpingData> warping_data_;
	WarpingUVEditor warp_uv_;
	WarpingMeshEditor warp_mesh_;
	
	std::shared_ptr<BlendingData> blending_data_;
	BlendingEditor blend_editor_;
	
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
	
	ofFbo fbo_;
};

class ResultView : public ofBaseApp
{
public:
	void setup() override;
	void draw() override;
	
	void setTexture(const ofTexture &texture) { texture_ = texture; }
	void setMesh(const ofMesh &mesh) { mesh_ = mesh; }
	
	void setScaleToViewport(bool enable) { is_scale_to_viewport_ = enable; }
	bool isScaleToViewport() const { return is_scale_to_viewport_; }
	
	void setEnableBlending(bool enable) { is_enabled_blending_ = enable; }
	bool isEnabledBlending() const { return is_enabled_blending_; }
	
	ofxBlendScreen::Shader::Params& getBlendParams() { return shader_.getParams(); } 
private:
	ofMesh mesh_;
	ofTexture texture_;
	bool is_scale_to_viewport_;
	bool is_enabled_blending_;
	ofxBlendScreen::Shader shader_;
};
