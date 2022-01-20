#pragma once

#include "ofMain.h"
#include "ofxImGui.h"
#include "WarpingEditor.h"
#include "UVEditor.h"
#include "ImageSource.h"

class MainApp;

class GuiApp : public ofBaseApp
{
public:
	void setup();
	void update();
	void draw();
	
	void keyPressed(int key);
	void setMainApp(std::shared_ptr<MainApp> app) { main_app_ = app; }
	void setMainWindow(std::shared_ptr<ofAppBaseWindow> window) { main_window_ = window; }
private:
	std::shared_ptr<MainApp> main_app_;
	std::shared_ptr<ofAppBaseWindow> main_window_;
	ofxImGui::Gui gui_;
	ImageSource texture_source_;
	WarpingEditor warp_;
	UVEditor uv_;
	enum State {
		EDIT_UV,
		EDIT_WRAP
	};
	int state_=EDIT_UV;
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
