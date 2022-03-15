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
#include "Undo.h"
#include "SaveData.h"

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
	
	void saveDataFile(const std::filesystem::path &filepath) const;
	void loadDataFile(const std::filesystem::path &filepath);
	void packDataFile(std::ostream &stream) const;
	void unpackDataFile(std::istream &stream);
	
	void keyPressed(int key) override;
	void mouseReleased(int x, int y, int button) override;
	
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
	std::string stateName(int state) const;
	bool isStateWarping() const { return state_ == EDIT_WARP_UV || state_ == EDIT_WARP_MESH; }
	bool isStateBlending() const { return state_ == EDIT_BLEND; }
	std::map<std::string, std::shared_ptr<EditorBase>> editor_;

	std::shared_ptr<WarpingData> warping_data_;
	std::shared_ptr<WarpingUVEditor> warp_uv_;
	std::shared_ptr<WarpingMeshEditor> warp_mesh_;
	
	std::shared_ptr<BlendingData> blending_data_;
	std::shared_ptr<BlendingEditor> blend_editor_;
	
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
	
	void undo() { undo_.undo(); }
	void redo() { undo_.redo(); }

	ofxNDIFinder ndi_finder_;
	
	mutable ProjectFolder proj_;
	
	Undo undo_;
	void initUndo();
	
	ofFbo fbo_;
};

class ResultView : public ofBaseApp
{
public:
	void setup() override;
	void draw() override;
	
	void setEditor(std::shared_ptr<EditorBase> editor) { editor_ = editor; }
	std::shared_ptr<EditorBase> getEditor() const { return editor_; }
	
	void setScaleToViewport(bool enable) { is_scale_to_viewport_ = enable; }
	bool isScaleToViewport() const { return is_scale_to_viewport_; }

	void setShowControl(bool enable) { is_show_control_ = enable; }
	bool isShowControl() const { return is_show_control_; }

	void setShowCursor(bool enable) { is_show_cursor_ = enable; }
	bool isShowCursor() const { return is_show_cursor_; }
private:
	std::shared_ptr<EditorBase> editor_;
	bool is_scale_to_viewport_;
	bool is_show_control_;
	bool is_show_cursor_;
};
