#include "AppGui.h"
#include "Models.h"
#include "ofxImGui.h"
#include "imgui_internal.h"
#include "Quad.h"
#include "gui/Gui.h"
#include "ofGraphics.h"

using namespace maaaaap;
using namespace ImGui;

namespace {
template <typename ... Args>
std::string format(const std::string& fmt, Args ... args )
{
	size_t len = std::snprintf( nullptr, 0, fmt.c_str(), args ... );
	std::vector<char> buf(len + 1);
	std::snprintf(&buf[0], len + 1, fmt.c_str(), args ... );
	return std::string(&buf[0], &buf[0] + len);
}
std::string getGLConstantName(GLint value) {
	std::string name = "undefined";
#define CASE(v) case v: name = #v; break;
	switch(value) {
			CASE(GL_TEXTURE_2D);
			CASE(GL_TEXTURE_RECTANGLE_ARB);
			CASE(GL_NEAREST);
			CASE(GL_LINEAR);
			CASE(GL_NEAREST_MIPMAP_NEAREST);
			CASE(GL_LINEAR_MIPMAP_NEAREST);
			CASE(GL_NEAREST_MIPMAP_LINEAR);
			CASE(GL_LINEAR_MIPMAP_LINEAR);
			CASE(GL_CLAMP);
			CASE(GL_REPEAT);
			CASE(GL_CLAMP_TO_EDGE);
			CASE(GL_CLAMP_TO_BORDER);
			CASE(GL_MIRRORED_REPEAT);
			CASE(GL_MIRROR_CLAMP_TO_EDGE);
	}
#undef CASE
	return format("%s(0x%x)", name.c_str(), value);
}
std::string getCompressionTypeName(ofTexCompression value) {
	std::string name = "undefined";
#define CASE(v) case v: name = #v; break;
	switch(value) {
		CASE(OF_COMPRESS_NONE);
		CASE(OF_COMPRESS_SRGB);
		CASE(OF_COMPRESS_ARB);
	}
#undef CASE
	return format("%s(0x%x)", name.c_str(), value);
}
}

void TextureInspector::Preview(ofTexture texture)
{
	ImVec2 size = GetContentRegionAvail();
	size.y = texture.getHeight() * size.x / texture.getWidth();
	Image(reinterpret_cast<ImTextureID>(texture.getTextureData().textureID), size);
}

void TextureInspector::Info(ofTexture texture)
{
	auto &data = texture.getTextureData();
	Text("textureID : %d", data.textureID);
	Text("glInternalFormat : %s", ofGetGLInternalFormatName(data.glInternalFormat).c_str());
	Text("textureTarget : %s", getGLConstantName(data.textureTarget).c_str());
	Text("tex_t : %f", data.tex_t);
	Text("tex_u : %f", data.tex_u);
	Text("tex_w : %f", data.tex_w);
	Text("tex_h : %f", data.tex_h);
	Text("width : %f", data.width);
	Text("height : %f", data.height);
	Text("bFlipTexture : %s", data.bFlipTexture ? "true" : "false");
	Text("compressionType : %s", getCompressionTypeName(data.compressionType).c_str());
	Text("bAllocated : %s", data.bAllocated ? "true" : "false");
	Text("minFilter : %s", getGLConstantName(data.minFilter).c_str());
	Text("magFilter : %s", getGLConstantName(data.magFilter).c_str());
	Text("wrapModeHorizontal : %s", getGLConstantName(data.wrapModeHorizontal).c_str());
	Text("wrapModeVertical : %s", getGLConstantName(data.wrapModeVertical).c_str());
	Text("bufferId : %d", data.bufferId);
}
void TextureInspector::EditInfo(ofTexture texture)
{
}
void SourceInspector::Info(std::shared_ptr<Source> source)
{
	TextureInspector tex_info;
	Columns(2);
	auto tex = source->getTexture();
	tex_info.Preview(tex);
	NextColumn();
	tex_info.Info(tex);
	Columns(1);
}

void RenderTextureInspector::Info(std::shared_ptr<RenderTexture> source)
{
	TextureInspector tex_info;
	Columns(2);
	auto tex = source->getTexture();
	tex_info.Preview(tex);
	NextColumn();
	tex_info.Info(tex);
	Columns(1);
}

void WarpingMeshInspector::setup()
{
	frame_.setup();
}

void WarpingMeshInspector::EditUV(std::shared_ptr<WarpingMesh> mesh, ResourceStorage &storage)
{
	Columns(2);
	auto &uv_quad = mesh->texcoord_range_;
	if(auto source = storage.getSourceFor(mesh)) {
		auto tex = source->getTexture();
		ImVec2 uv_a(uv_quad[0].x, uv_quad[0].y);
		ImVec2 uv_b(uv_quad[1].x, uv_quad[1].y);
		ImVec2 uv_c(uv_quad[3].x, uv_quad[3].y);
		ImVec2 uv_d(uv_quad[2].x, uv_quad[2].y);
		ImVec2 size = GetContentRegionAvail();
		size.y = tex.getHeight() * size.x / tex.getWidth();
		ImageWithUVOverlapped(reinterpret_cast<ImTextureID>(tex.getTextureData().textureID), uv_a, uv_b, uv_c, uv_d, size);
	}
	NextColumn();
	EditUVQuad("quad", uv_quad);
	Columns(1);
}

void WarpingMeshInspector::EditMesh(std::shared_ptr<WarpingMesh> mesh, ResourceStorage &storage)
{
	callback_arg_.emplace_back(this, mesh, storage);
	
	ImVec2 pos = GetCursorScreenPos(), size = GetContentRegionAvail();
	InvisibleButton("pane", size);
	pos = pos*GetIO().DisplayFramebufferScale;
	size = size*GetIO().DisplayFramebufferScale;
	frame_.setRegion({pos.x, pos.y, size.x, size.y});
	if(IsItemActive() || IsItemHovered()) {
		frame_.enableMouseInteraction();
	}
	else {
		frame_.disableMouseInteraction();
	}
	ImGui::GetWindowDrawList()->AddCallback([](const ImDrawList* parent_list, const ImDrawCmd* cmd) {
		auto &self = *(WarpingMeshInspector*)(cmd->UserCallbackData);
		auto arg = self.callback_arg_.front();
		self.callback_arg_.pop_front();
		auto &storage = arg.storage;
		auto mesh = arg.mesh;
		auto source = storage.getSourceFor(mesh);
		auto target = storage.getRenderTextureIncluding(mesh);
		auto target_tex = target->get()->getTexture();
		auto source_tex = source->getTexture();
		auto &frame = self.frame_;
		frame.pushMatrix();
		frame.pushScissor();
		ofPushStyle();
		ofSetColor(ofColor::gray);
		target_tex.draw(0,0);
		ofSetColor(ofColor::white);
		source_tex.bind();
		mesh->getMesh().draw();
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

void WarpingMeshInspector::inactivateInteraction()
{
	frame_.disableMouseInteraction();
}

void BlendingMeshInspector::setup()
{
	frame_.setup();
}

void BlendingMeshInspector::EditUV(std::shared_ptr<BlendingMesh> mesh, ResourceStorage &storage)
{
	Columns(2);
	auto &uv_quad = mesh->texture_uv_for_frame_;
	if(auto source = storage.getRenderTextureReferencedBy(mesh)) {
		auto tex = source->getTexture();
		ImVec2 uv_a(uv_quad[0].x, uv_quad[0].y);
		ImVec2 uv_b(uv_quad[1].x, uv_quad[1].y);
		ImVec2 uv_c(uv_quad[3].x, uv_quad[3].y);
		ImVec2 uv_d(uv_quad[2].x, uv_quad[2].y);
		ImVec2 size = GetContentRegionAvail();
		size.y = tex.getHeight() * size.x / tex.getWidth();
		ImageWithUVOverlapped(reinterpret_cast<ImTextureID>(tex.getTextureData().textureID), uv_a, uv_b, uv_c, uv_d, size);
	}
	NextColumn();
	EditUVQuad("quad", uv_quad);
	Columns(1);
}

void BlendingMeshInspector::EditMesh(std::shared_ptr<BlendingMesh> mesh, ResourceStorage &storage, ofxBlendScreen::Shader &shader)
{
	callback_arg_.emplace_back(this, mesh, storage, shader);
	
	ImVec2 pos = GetCursorScreenPos(), size = GetContentRegionAvail();
	InvisibleButton("pane", size);
	pos = pos*GetIO().DisplayFramebufferScale;
	size = size*GetIO().DisplayFramebufferScale;
	frame_.setRegion({pos.x, pos.y, size.x, size.y});
	if(IsItemActive() || IsItemHovered()) {
		frame_.enableMouseInteraction();
	}
	else {
		frame_.disableMouseInteraction();
	}
	ImGui::GetWindowDrawList()->AddCallback([](const ImDrawList* parent_list, const ImDrawCmd* cmd) {
		auto &self = *(BlendingMeshInspector*)(cmd->UserCallbackData);
		auto arg = self.callback_arg_.front();
		self.callback_arg_.pop_front();
		auto &storage = arg.storage;
		auto mesh = arg.mesh;
		auto target = storage.getOutputFor(mesh);
		auto source = storage.getRenderTextureReferencedBy(mesh);
		if(!target || !source) return;
		auto drawSingle = [&](std::shared_ptr<BlendingMesh> blend) {
			if(auto src = storage.getRenderTextureReferencedBy(blend)) {
				auto &shader = arg.shader;
				shader.begin(src->getTexture());
				blend->getMesh().draw();
				shader.end();
			}
			else {
				blend->getMesh().draw();
			}
		};
		auto &frame = self.frame_;
		frame.pushMatrix();
		frame.pushScissor();
		for(auto &blend : storage.getContainer<BlendingMesh>()) {
			if(blend == mesh || storage.getOutputFor(blend) != target) continue;
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
		drawSingle(mesh);
		frame.popScissor();
		frame.popMatrix();
	}, this);
	ImGui::GetWindowDrawList()->AddCallback(ImDrawCallback_ResetRenderState, nullptr);
}

void BlendingMeshInspector::inactivateInteraction()
{
	frame_.disableMouseInteraction();
}


void OutputInspector::setup()
{
	frame_.setup();
}

void OutputInspector::EditMesh(std::shared_ptr<Output> output, ResourceStorage &storage, ofxBlendScreen::Shader &shader)
{
	callback_arg_.emplace_back(this, output, storage, shader);
	
	ImVec2 pos = GetCursorScreenPos(), size = GetContentRegionAvail();
	InvisibleButton("pane", size);
	pos = pos*GetIO().DisplayFramebufferScale;
	size = size*GetIO().DisplayFramebufferScale;
	frame_.setRegion({pos.x, pos.y, size.x, size.y});
	if(IsItemActive() || IsItemHovered()) {
		frame_.enableMouseInteraction();
	}
	else {
		frame_.disableMouseInteraction();
	}
	ImGui::GetWindowDrawList()->AddCallback([](const ImDrawList* parent_list, const ImDrawCmd* cmd) {
		auto &self = *(OutputInspector*)(cmd->UserCallbackData);
		auto arg = self.callback_arg_.front();
		self.callback_arg_.pop_front();
		auto &storage = arg.storage;
		auto output = arg.output;
		auto source = storage.getBlendsBoundTo(output);
		auto shader = arg.shader;
		auto drawSingle = [&](std::shared_ptr<BlendingMesh> blend) {
			if(auto src = storage.getRenderTextureReferencedBy(blend)) {
				shader.begin(src->getTexture());
				blend->getMesh().draw();
				shader.end();
			}
			else {
				blend->getMesh().draw();
			}
		};
		auto drawOutput = [&](std::shared_ptr<Output> output) {
			for(auto blend : storage.getBlendsBoundTo(output)) {
				drawSingle(blend);
			}
		};
		auto &frame = self.frame_;
		frame.pushMatrix();
		frame.pushScissor();
		for(auto &o : storage.getContainer<Output>()) {
			if(o == output) continue;
			drawOutput(o);
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
		drawOutput(output);
		frame.popScissor();
		frame.popMatrix();
	}, this);
	ImGui::GetWindowDrawList()->AddCallback(ImDrawCallback_ResetRenderState, nullptr);
}

void OutputInspector::inactivateInteraction()
{
	frame_.disableMouseInteraction();
}
