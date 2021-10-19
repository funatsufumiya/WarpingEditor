#include "ofApp.h"
#include "ofxBlendScreen.h"
#include "BlendingEditor.h"

class App
{
public:
	void setup() {
		ofDisableArbTex();
		ofLoadImage(tex_, "of.png");
		shader_.setup();
		editor_.setup();
		editor_.addFrame({0,0,300,300}, {0.1f, 0.9f}, {0.1f, 0.9f}, {1,1});
		editor_.addFrame({100,100,300,500}, {0.2f, 0.5f}, {0.f, 1.f}, {1,1});
		editor_.setRegion({100,100,500,500});
	}
	void update() {
	}
	void draw() const {
//		ofMesh mesh = ofxBlendScreen::createMesh(ofRectangle{0,0,500,500}, ofRectangle{200,0,100,500}, ofRectangle{-250,-250,1000,1000},ofRectangle{0,0,1,1});
		ofMesh mesh = editor_.getResult();
		ofPushStyle();
		ofEnableBlendMode(OF_BLENDMODE_ADD);
		editor_.pushMatrix();
		shader_.begin(tex_, gamma_, blend_power_, luminance_control_, base_luminance_);
		mesh.drawFaces();
//		ofTranslate(300,0);
//		mesh.drawFaces();
		shader_.end();
		ofEnableBlendMode(OF_BLENDMODE_ALPHA);
		ofSetColor(ofColor::red);
		for(auto &&p : editor_.getHover()) {
			ofDrawCircle(*p.v, 10);
		}
		for(auto &&p : editor_.getSelected()) {
			ofDrawCircle(*p.v, 10);
		}
		editor_.popMatrix();
		ofPopStyle();
	}
	void gui() {
	}
	void setBlendParameters(float gamma, float blend_power, float luminance_control, float base_luminance) {
		gamma_=gamma, blend_power_=blend_power,
		luminance_control_=luminance_control,
		base_luminance_=base_luminance;
	}

	void keyPressed(int key){
		glm::vec3 move = {0,0,0};
		float amount = 10;
		switch(key) {
			case OF_KEY_LEFT: move.x -= amount; break;
			case OF_KEY_RIGHT: move.x += amount; break;
			case OF_KEY_UP: move.y -= amount; break;
			case OF_KEY_DOWN: move.y += amount; break;
		}
		for(auto &&p : editor_.getSelected()) {
			*p.v += move;
		}
	}
private:
	BlendingEditor editor_;
	mutable ofxBlendScreen::Shader shader_;
	ofTexture tex_;
	float gamma_=1.0f, blend_power_=0.5f, luminance_control_=0.5f, base_luminance_=0.0f;
};

//--------------------------------------------------------------
void ofApp::setup(){
	ofBackground(0);
	app_ = std::make_shared<App>();
	app_->setup();
	gui_.setup();
}

//--------------------------------------------------------------
void ofApp::update(){
	app_->update();
}

//--------------------------------------------------------------
void ofApp::draw(){
	app_->draw();
	gui_.begin();
	app_->gui();
	gui_.end();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	app_->keyPressed(key);
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
	float blend = x/(float)ofGetWidth();
	float lumi = y/(float)ofGetHeight();
	app_->setBlendParameters(1, blend, lumi, 0.0f);
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
