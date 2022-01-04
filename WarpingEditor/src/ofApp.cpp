#include "ofApp.h"
#include "Models.h"

//--------------------------------------------------------------
void ofApp::setup(){
//	ofDisableArbTex();
	ofBackground(0);
	gui_.setup();
	uv_.setup();
	warp_.setup();
	ofLoadImage(texture_, "of.png");
	auto &data = Data::shared();
	data.create();
	data.create();
	uv_.setTexture(texture_);
	warp_.setTexture(texture_);
}

//--------------------------------------------------------------
void ofApp::update(){
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
