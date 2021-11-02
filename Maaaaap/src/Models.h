#pragma once

#include "ofTexture.h"
#include "ofMesh.h"
#include "ofFbo.h"
#include "ofxMapperMesh.h"
#include "ofImage.h"
#include "ofxBlendScreen.h"
#include "Quad.h"

namespace maaaaap {
class Footage : public ofBaseHasTexture
{
public:
	virtual void setUseTexture(bool) override {}
	virtual bool isUsingTexture() const override { return true; }
};
class ImageFile : public Footage
{
public:
	void load(const std::string &filepath) {
		ofLoadImage(texture_, filepath);
	}
	ofTexture& getTexture() override { return texture_; }
	const ofTexture& getTexture() const override { return texture_; }
protected:
	ofTexture texture_;
};

class TextureOut : public Footage
{
public:
	virtual void begin()=0;
	virtual void end()=0;
};
class Fbo : public TextureOut
{
public:
	ofFbo& get() { return fbo_; }
	void begin() override {
		fbo_.begin();
	}
	void end() override {
		fbo_.end();
	}
	ofTexture& getTexture() override { return fbo_.getTexture(); }
	const ofTexture& getTexture() const override { return fbo_.getTexture(); }
protected:
	ofFbo fbo_;
};

class Source
{
public:
	void set(std::shared_ptr<Footage> tex) { texture_ = tex; }
	std::shared_ptr<Footage> get() { return texture_; }
	ofTexture getTexture() { return texture_->getTexture(); }
protected:
	std::shared_ptr<Footage> texture_;
};
class WarpingMesh
{
public:
	void set(std::shared_ptr<ofx::mapper::Mesh> mesh) { mesh_ = mesh; }
	std::shared_ptr<ofx::mapper::Mesh> get() { return mesh_; }
	ofMesh getMesh() const;
	geom::Quad texcoord_range_;
protected:
	std::shared_ptr<ofx::mapper::Mesh> mesh_;
};
class RenderTexture
{
public:
	void begin() { fbo_->begin(); }
	void end() { fbo_->end(); }
	void set(std::shared_ptr<Fbo> fbo) { fbo_ = fbo; }
	std::shared_ptr<Fbo> get() const { return fbo_; }
	ofTexture getTexture() { return fbo_->getTexture(); }
protected:
	std::shared_ptr<Fbo> fbo_;
};
class BlendingMesh
{
public:
	geom::Quad vertex_outer_;
	geom::Quad vertex_inner_;
	geom::Quad vertex_frame_;
	geom::Quad texture_uv_for_frame_;
	ofMesh getMesh() const;
};
class Output
{
public:
protected:
	ofRectangle rect_;
};
}
