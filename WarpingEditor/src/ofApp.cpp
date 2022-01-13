#include "ofApp.h"
#include "Models.h"
#include "GuiFunc.h"
#include "Icon.h"

namespace {
ofTexture t,f;
}

//--------------------------------------------------------------
void ofApp::setup(){
	ofDisableArbTex();
	ofBackground(0);
	
	gui_.setup();
	
	Icon::init();
	ofLoadImage(t, "t.png");
	ofLoadImage(f, "f.png");
	
	uv_.setup();
	warp_.setup();
	ofLoadImage(texture_, "of.png");
	auto &data = Data::shared();
	data.create();
	data.create();
	uv_.setTexture(texture_);
	warp_.setTexture(texture_);
	
	if(!ofGetUsingArbTex()) {
		auto size = ofGetWindowSize();
		uv_.scale(glm::min(size.x, size.y), {0,0});
	}
}

//--------------------------------------------------------------
void ofApp::update(){
	bool gui_focused = ImGui::GetIO().WantCaptureMouse;
	uv_.setEnableViewportEditByMouse(!gui_focused);
	uv_.setEnableMeshEditByMouse(!gui_focused);
	warp_.setEnableViewportEditByMouse(!gui_focused);
	warp_.setEnableMeshEditByMouse(!gui_focused);
	auto &data = Data::shared();
	data.update();
	switch(state_) {
		case EDIT_UV:
			uv_.setRegion(ofGetCurrentViewport());
			uv_.update();
			break;
		case EDIT_WRAP:
			warp_.setRegion(ofGetCurrentViewport());
			warp_.update();
			break;
	}
}

//--------------------------------------------------------------
void ofApp::draw(){
	switch(state_) {
		case EDIT_UV:
			uv_.draw();
			break;
		case EDIT_WRAP:
			warp_.draw();
			break;
	}
	gui_.begin();
	using namespace ImGui;
	if(Begin("Switch")) {
		auto &data = Data::shared();
		for(auto &&m : data.getMesh()) {
			PushID(m.first.c_str());
			ToggleButton("##hide", m.second->is_hidden, Icon::HIDE, Icon::SHOW, {17,17}, 0);	SameLine();
			ToggleButton("##lock", m.second->is_locked, Icon::LOCK, Icon::UNLOCK, {17,17}, 0);	SameLine();
			ToggleButton("##solo", m.second->is_solo, Icon::FLAG, Icon::BLANK, {17,17}, 0);	SameLine();
			Selectable(m.first.c_str());
			PopID();
		}
	}
	End();
	gui_.end();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	switch(key) {
		case OF_KEY_TAB:
			state_ ^= 1;
			break;
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
