#include "ofApp.h"
#include "ofxBlendScreen.h"
#include "BlendingEditor.h"
#include "WarpingEditor.h"
#include "ResourceStorage.h"
#include "gui/Gui.h"
#include "imgui_internal.h"
#include "AppGui.h"

using namespace maaaaap;

class App
{
public:
	SourceInspector source_info_;
	RenderTextureInspector target_info_;
	WarpingMeshInspector warp_mesh_info_;
	BlendingMeshInspector blend_mesh_info_;
	OutputInspector output_info_;
	ResourceStorage storage_;
	App():mode_(WARP, NUM_MODE) {
	}
	void setup() {
		auto window_rect = ofGetWindowRect();
		ofDisableArbTex();
		auto source = make_shared<Source>();
		{
			auto image = std::make_shared<ImageFile>();
			image->load("of.png");
			source->set(image);
		}
		auto source2 = make_shared<Source>();
		{
			auto image = std::make_shared<ImageFile>();
			image->load("of.png");
			source2->set(image);
		}
		auto warp = make_shared<WarpingMesh>();
		{
			auto wm = make_shared<ofx::mapper::Mesh>();
			wm->init({1,1}, {0,0,256,256}, {0,0,1,1});
			warp->set(wm);
		}
		auto warp2 = make_shared<WarpingMesh>();
		{
			auto wm = make_shared<ofx::mapper::Mesh>();
			wm->init({1,1}, {256,256,512,256}, {0,0,1,1});
			warp2->set(wm);
		}
		auto rt = make_shared<RenderTexture>();
		{
			auto fbo = make_shared<Fbo>();
			fbo->get().allocate(window_rect.width, window_rect.height, GL_RGBA);
			rt->set(fbo);
		}
		auto rt2 = make_shared<RenderTexture>();
		{
			auto fbo = make_shared<Fbo>();
			fbo->get().allocate(window_rect.width, window_rect.height, GL_RGBA);
			rt2->set(fbo);
			storage_.add(rt2);
		}
		auto blending = make_shared<BlendingMesh>();
		{
			ofRectangle rect = window_rect;
			blending->vertex_frame_ = rect;
			blending->vertex_outer_ = rect;
			blending->vertex_inner_ = rect;
			blending->texture_uv_for_frame_ = {0.5f,0.f,0.5f,1.f};
		}
		auto blending2 = make_shared<BlendingMesh>();
		{
			ofRectangle rect = window_rect;
			rect.x += rect.width;
			blending2->vertex_frame_ = rect;
			blending2->vertex_outer_ = rect;
			blending2->vertex_inner_ = rect;
			blending2->texture_uv_for_frame_ = {0.f,0.f,0.5f,1.f};
		}
		auto o = make_shared<Output>();
		auto o2 = make_shared<Output>();
		storage_.bind(warp, source);
		storage_.bind(warp, rt);
		storage_.bind(warp2, source2);
		storage_.bind(warp2, rt2);
		storage_.bind(blending, rt);
		storage_.bind(blending, o);
		storage_.bind(blending2, rt2);
		storage_.bind(blending2, o2);
		
		warp_mesh_info_.setup();
		blend_mesh_info_.setup();
		output_info_.setup();
		shader_.setup();
		auto &p = shader_.getParams();
		p.gamma = {1,1,1};
		p.blend_power = 0.5f;
		p.luminance_control = 0.5f;
		p.base_color = {0,0,0};
	}
	void update() {
		updateWarpTexture();
	}
	void draw() const {
	}
	void gui() {
		using namespace ImGui;
		{
			static std::shared_ptr<Source> src_selected = nullptr;
			if(Begin("Sources")) {
				auto src = storage_.getContainer<Source>();
				int i = 0;
				char buf[256]={};
				for(auto &&s : src) {
					ImFormatString(buf, 256, "%s%d", "source", i++);
					if(Selectable(buf, src_selected == s)) {
						src_selected = s;
					}
				}
			}
			End();
			if(Begin("Source_preview") && src_selected) {
				source_info_.Info(src_selected);
			}
			End();
		}
		{
			static std::shared_ptr<WarpingMesh> warp_selected = nullptr;
			if(Begin("WarpingMeshes")) {
				auto warp = storage_.getContainer<WarpingMesh>();
				int i = 0;
				char buf[256]={};
				for(auto &&w : warp) {
					ImFormatString(buf, 256, "%s%d", "warp", i++);
					if(Selectable(buf, warp_selected == w)) {
						warp_selected = w;
					}
				}
			}
			End();
			if(Begin("WarpingMesh_input") && warp_selected) {
				auto source = storage_.getSourceFor(warp_selected);
				if(BeginCombo("source", "---select a source---")) {
					auto src = storage_.getContainer<Source>();
					int i = 0;
					char buf[256]={};
					for(auto &&s : src) {
						ImFormatString(buf, 256, "%s%d", "source", i++);
						if(Selectable(buf, source == s)) {
							storage_.bind(warp_selected, s);
						}
					}
					EndCombo();
				}
				warp_mesh_info_.EditUV(warp_selected, storage_);
			}
			End();
			if(Begin("WarpingMesh_output") && warp_selected) {
				auto target_selected = storage_.getRenderTextureIncluding(warp_selected);
				if(BeginCombo("target", "---select a target---")) {
					auto target = storage_.getContainer<RenderTexture>();
					int i = 0;
					char buf[256]={};
					for(auto &&t : target) {
						ImFormatString(buf, 256, "%s%d", "target", i++);
						if(Selectable(buf, target_selected == t)) {
							storage_.bind(warp_selected, t);
						}
					}
					EndCombo();
				}
				warp_mesh_info_.EditMesh(warp_selected, storage_);
			}
			else {
				warp_mesh_info_.inactivateInteraction();
			}
			End();
		}
		{
			static std::shared_ptr<RenderTexture> target_selected = nullptr;
			if(Begin("RenderTextures")) {
				auto texture = storage_.getContainer<RenderTexture>();
				int i = 0;
				char buf[256]={};
				for(auto &&t : texture) {
					ImFormatString(buf, 256, "%s%d", "texture", i++);
					if(Selectable(buf, target_selected == t)) {
						target_selected = t;
					}
				}
			}
			End();
			if(Begin("RenderTexture_preview") && target_selected) {
				target_info_.Info(target_selected);
			}
			End();
		}
		{
			static std::shared_ptr<BlendingMesh> blend_selected = nullptr;
			if(Begin("BlendingMeshes")) {
				auto blend = storage_.getContainer<BlendingMesh>();
				int i = 0;
				char buf[256]={};
				for(auto &&b : blend) {
					ImFormatString(buf, 256, "%s%d", "blend", i++);
					if(Selectable(buf, blend_selected == b)) {
						blend_selected = b;
					}
				}
			}
			End();
			if(Begin("BlendingMesh_input") && blend_selected) {
				auto source = storage_.getRenderTextureReferencedBy(blend_selected);
				if(BeginCombo("render texture", "---select a render texture---")) {
					auto textures = storage_.getContainer<RenderTexture>();
					int i = 0;
					char buf[256]={};
					for(auto &&t : textures) {
						ImFormatString(buf, 256, "%s%d", "texture", i++);
						if(Selectable(buf, source == t)) {
							storage_.bind(blend_selected, t);
						}
					}
					EndCombo();
				}
				blend_mesh_info_.EditUV(blend_selected, storage_);
			}
			End();
			if(Begin("BlendingMesh_output") && blend_selected) {
				auto output_selected = storage_.getOutputFor(blend_selected);
				if(BeginCombo("output", "---select a output---")) {
					auto output = storage_.getContainer<Output>();
					int i = 0;
					char buf[256]={};
					for(auto &&o : output) {
						ImFormatString(buf, 256, "%s%d", "output", i++);
						if(Selectable(buf, output_selected == o)) {
							storage_.bind(blend_selected, o);
						}
					}
					EndCombo();
				}
				blend_mesh_info_.EditMesh(blend_selected, storage_, shader_);
			}
			else {
				blend_mesh_info_.inactivateInteraction();
			}
			End();
		}
		{
			static std::shared_ptr<Output> output_selected = nullptr;
			if(Begin("Output")) {
				auto src = storage_.getContainer<Output>();
				int i = 0;
				char buf[256]={};
				for(auto &&s : src) {
					ImFormatString(buf, 256, "%s%d", "output", i++);
					if(Selectable(buf, output_selected == s)) {
						output_selected = s;
					}
				}
			}
			End();
			if(Begin("Output_preview")) {
				output_info_.EditMesh(output_selected, storage_, shader_);
			}
			else {
				output_info_.inactivateInteraction();
			}
			End();
		}
	}

	void keyPressed(int key){
	}
private:
	mutable ofxBlendScreen::Shader shader_;
	ofTexture tex_;
	template<typename T>
	class Wrap {
	public:
		Wrap(const T &range0, const T &range1) {
			range_[0] = range0;
			range_[1] = range1;
		}
		operator T const&() const { return val_; }
		void next() { if(++val_ > range_[1]) val_=range_[0]; }
	private:
		T val_;
		T range_[2];
	};
	enum {
		WARP, CROP, BLEND,
		NUM_MODE = BLEND
	};
	Wrap<int> mode_;
	
	void updateWarpTexture() {
		auto render_textures = storage_.getContainer<RenderTexture>();
		for(auto &&r : render_textures) {
			r->begin();
			ofClear(0);
			for(auto &&w : storage_.getWarpsBoundTo(r)) {
				auto tex = storage_.getSourceFor(w);
				tex->getTexture().bind();
				w->getMesh().draw();
				tex->getTexture().unbind();
			}
			r->end();
		}
	}
};

//--------------------------------------------------------------
void ofApp::setup(){
	ofBackground(0);
	app_ = std::make_shared<App>();
	app_->setup();
	gui_.setup(nullptr, true, ImGuiConfigFlags_DockingEnable, true);
}

//--------------------------------------------------------------
void ofApp::update(){
	app_->update();
}

//--------------------------------------------------------------
void ofApp::draw(){
	app_->draw();
	gui_.begin();
	ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
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
