#include "ofApp.h"
#include "ofxBlendScreen.h"
#include "BlendingEditor.h"
#include "WarpingEditor.h"
#include "ResourceStorage.h"
#include "gui/Gui.h"
#include "imgui_internal.h"

using namespace maaaaap;

class App
{
public:
	ResourceStorage storage_;
	ofxEditorFrame frame_warp_, frame_blend_, frame_output_;
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
		
		frame_warp_.setup();
		frame_blend_.setup();
		frame_output_.setup();
		shader_.setup();
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
				auto tex = src_selected->getTexture();
				ImVec2 size = GetContentRegionAvail();
				size.y = tex.getHeight() * size.x / tex.getWidth();
				Image(reinterpret_cast<ImTextureID>(tex.getTextureData().textureID), size);
			}
			End();
		}
		{
			static std::shared_ptr<WarpingMesh> warp_selected = nullptr;
			auto &frame = frame_warp_;
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
				auto &uv_quad = warp_selected->texcoord_range_;
				EditUVQuad("quad", uv_quad);
				if(source) {
					ImVec2 uv_a(uv_quad[0].x, uv_quad[0].y);
					ImVec2 uv_b(uv_quad[1].x, uv_quad[1].y);
					ImVec2 uv_c(uv_quad[3].x, uv_quad[3].y);
					ImVec2 uv_d(uv_quad[2].x, uv_quad[2].y);
					auto tex = source->getTexture();
					ImVec2 size = GetContentRegionAvail();
					size.y = tex.getHeight() * size.x / tex.getWidth();
					ImageWithUVOverlapped(reinterpret_cast<ImTextureID>(tex.getTextureData().textureID), uv_a, uv_b, uv_c, uv_d, size);
				}
			}
			End();
			bool is_frame_active = false;
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
				ImVec2 pos = GetCursorScreenPos(), size = GetContentRegionAvail();
				frame.setRegion({pos.x, pos.y, size.x, size.y});
				InvisibleButton("pane", size);
				is_frame_active = IsItemActive() || IsItemHovered();
				ImGui::GetWindowDrawList()->AddCallback([](const ImDrawList* parent_list, const ImDrawCmd* cmd) {
					if(!warp_selected) return;
					auto &self = *(App*)(cmd->UserCallbackData);
					auto target = self.storage_.getRenderTextureIncluding(warp_selected);
					auto source = self.storage_.getSourceFor(warp_selected);
					if(!target || !source) return;
					auto target_tex = target->get()->getTexture();
					auto source_tex = source->getTexture();
					auto &frame = self.frame_warp_;
					frame.pushMatrix();
					frame.pushScissor();
					ofPushStyle();
					ofSetColor(ofColor::gray);
					target_tex.draw(0,0);
					ofSetColor(ofColor::white);
					source_tex.bind();
					warp_selected->getMesh().draw();
					source_tex.unbind();
					ofSetColor(ofColor::red);
					ofNoFill();
					ofDrawRectangle({0,0,target_tex.getWidth(),target_tex.getHeight()});
					ofPopStyle();
					frame.popScissor();
					frame.popMatrix();
				}, this);
				ImGui::GetWindowDrawList()->AddCallback(ImDrawCallback_ResetRenderState, nullptr);
			}
			End();
			if(is_frame_active) {
				frame.enableMouseInteraction();
			}
			else {
				frame.disableMouseInteraction();
			}
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
				auto tex = target_selected->getTexture();
				ImVec2 size = GetContentRegionAvail();
				size.y = tex.getHeight() * size.x / tex.getWidth();
				Image(reinterpret_cast<ImTextureID>(tex.getTextureData().textureID), size);
			}
			End();
		}
		{
			static std::shared_ptr<BlendingMesh> blend_selected = nullptr;
			auto &frame = frame_blend_;
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
				auto &uv_quad = blend_selected->texture_uv_for_frame_;
				EditUVQuad("quad", uv_quad);
				if(source) {
					ImVec2 uv_a(uv_quad[0].x, uv_quad[0].y);
					ImVec2 uv_b(uv_quad[1].x, uv_quad[1].y);
					ImVec2 uv_c(uv_quad[3].x, uv_quad[3].y);
					ImVec2 uv_d(uv_quad[2].x, uv_quad[2].y);
					auto tex = source->getTexture();
					ImVec2 size = GetContentRegionAvail();
					size.y = tex.getHeight() * size.x / tex.getWidth();
					ImageWithUVOverlapped(reinterpret_cast<ImTextureID>(tex.getTextureData().textureID), uv_a, uv_b, uv_c, uv_d, size);
				}
			}
			End();
			bool is_frame_active = false;
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
				ImVec2 pos = GetCursorScreenPos(), size = GetContentRegionAvail();
				frame.setRegion({pos.x, pos.y, size.x, size.y});
				InvisibleButton("pane", size);
				is_frame_active = IsItemActive() || IsItemHovered();
				ImGui::GetWindowDrawList()->AddCallback([](const ImDrawList* parent_list, const ImDrawCmd* cmd) {
					if(!blend_selected) return;
					auto &self = *(App*)(cmd->UserCallbackData);
					auto target = self.storage_.getOutputFor(blend_selected);
					auto source = self.storage_.getRenderTextureReferencedBy(blend_selected);
					if(!target || !source) return;
					auto drawSingle = [&](std::shared_ptr<BlendingMesh> blend) {
						if(auto src = self.storage_.getRenderTextureReferencedBy(blend)) {
							auto &shader = self.shader_;
							auto &gamma = self.gamma_;
							auto &blend_power = self.blend_power_;
							auto &luminance_control = self.luminance_control_;
							auto &base_luminance = self.base_luminance_;
							shader.begin(src->getTexture(), gamma, blend_power, luminance_control, base_luminance);
							blend->getMesh().draw();
							shader.end();
						}
						else {
							blend->getMesh().draw();
						}
					};
					auto &frame = self.frame_blend_;
					frame.pushMatrix();
					frame.pushScissor();
					for(auto &blend : self.storage_.getContainer<BlendingMesh>()) {
						if(blend == blend_selected || self.storage_.getOutputFor(blend) != target) continue;
						drawSingle(blend);
					}
					frame.popScissor();
					frame.popMatrix();
					ofPushStyle();
					ofSetColor(ofColor::gray);
					ofEnableBlendMode(OF_BLENDMODE_SUBTRACT);
					ofDrawRectangle(frame.getRegion());
					ofPopStyle();
					frame.pushMatrix();
					frame.pushScissor();
					drawSingle(blend_selected);
					frame.popScissor();
					frame.popMatrix();
				}, this);
				ImGui::GetWindowDrawList()->AddCallback(ImDrawCallback_ResetRenderState, nullptr);
			}
			End();
			if(is_frame_active) {
				frame.enableMouseInteraction();
			}
			else {
				frame.disableMouseInteraction();
			}
		}
		{
			static std::shared_ptr<Output> output_selected = nullptr;
			auto &frame = frame_output_;
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
			bool is_frame_active = false;
			if(Begin("Output_preview")) {
				ImVec2 pos = GetCursorScreenPos(), size = GetContentRegionAvail();
				frame.setRegion({pos.x, pos.y, size.x, size.y});
				InvisibleButton("pane", size);
				is_frame_active = IsItemActive() || IsItemHovered();
				ImGui::GetWindowDrawList()->AddCallback([](const ImDrawList* parent_list, const ImDrawCmd* cmd) {
					auto &self = *(App*)(cmd->UserCallbackData);
					auto drawSingle = [&](std::shared_ptr<BlendingMesh> blend) {
						if(auto src = self.storage_.getRenderTextureReferencedBy(blend)) {
							auto &shader = self.shader_;
							auto &gamma = self.gamma_;
							auto &blend_power = self.blend_power_;
							auto &luminance_control = self.luminance_control_;
							auto &base_luminance = self.base_luminance_;
							shader.begin(src->getTexture(), gamma, blend_power, luminance_control, base_luminance);
							blend->getMesh().draw();
							shader.end();
						}
						else {
							blend->getMesh().draw();
						}
					};
					auto drawOutput = [&](std::shared_ptr<Output> output) {
						for(auto blend : self.storage_.getBlendsBoundTo(output)) {
							drawSingle(blend);
						}
					};
					auto &frame = self.frame_output_;
					frame.pushMatrix();
					frame.pushScissor();
					for(auto &output : self.storage_.getContainer<Output>()) {
						if(output == output_selected) continue;
						drawOutput(output);
					}
					frame.popScissor();
					frame.popMatrix();
					ofPushStyle();
					ofSetColor(ofColor::gray);
					ofEnableBlendMode(OF_BLENDMODE_SUBTRACT);
					ofDrawRectangle(frame.getRegion());
					ofPopStyle();
					frame.pushMatrix();
					frame.pushScissor();
					drawOutput(output_selected);
					frame.popScissor();
					frame.popMatrix();
				}, this);
				ImGui::GetWindowDrawList()->AddCallback(ImDrawCallback_ResetRenderState, nullptr);
			}
			End();
			if(is_frame_active) {
				frame.enableMouseInteraction();
			}
			else {
				frame.disableMouseInteraction();
			}
		}
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
			case OF_KEY_TAB: mode_.next(); break;
		}
		switch(mode_) {
			case WARP:
				for(auto &&p : warping_editor_.getSelected()) {
					*p.v += move;
				}
				break;
			case CROP:
				for(auto &&p : blend_editor_.getSelected()) {
					*p.t += glm::vec2(move) / glm::vec2(warped_.getWidth(), warped_.getHeight());
				}
				break;
			case BLEND:
				for(auto &&p : blend_editor_.getSelected()) {
					*p.v += move;
				}
				break;
		}
	}
private:
	WarpingEditor warping_editor_;
	BlendingEditor blend_editor_;
	ofFbo warped_;
	mutable ofxBlendScreen::Shader shader_;
	ofTexture tex_;
	float gamma_=1.0f, blend_power_=0.5f, luminance_control_=0.5f, base_luminance_=0.0f;
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
