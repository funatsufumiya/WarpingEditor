#pragma once

#include "ofGLBaseTypes.h"
#include "ofxNDIFinder.h"
#include "ofxNDIVideoGrabber.h"

class ImageSourceImpl : public ofBaseHasTexture
{
public:
	virtual void update() {}
	virtual bool isFrameNew() const { return false; }
	void setUseTexture(bool bUseTex) override { }
	bool isUsingTexture() const override { return true; }
};

class ImageSource : public ofBaseHasTexture
{
public:
	bool loadFromFile(const std::filesystem::path &filepath);
	template<typename T>
	bool setupNDI(T &&source);
	void update() { impl_->update(); }
	bool isFrameNew() const { return impl_->isFrameNew(); }
	
	ofTexture& getTexture() override { return impl_->getTexture(); }
	const ofTexture& getTexture() const override { return impl_->getTexture(); };
	void setUseTexture(bool bUseTex) override { impl_->setUseTexture(bUseTex); }
	bool isUsingTexture() const override { return impl_->isUsingTexture(); }

protected:
	std::shared_ptr<ImageSourceImpl> impl_;
};

namespace {
class NDIGrabber : public ImageSourceImpl, public ofxNDIVideoGrabber
{
public:
	using ofxNDIVideoGrabber::update;
	ofTexture& getTexture() override { return ofVideoGrabber::getTexture(); }
	const ofTexture& getTexture() const override { return ofVideoGrabber::getTexture(); };
};
}
template<typename T>
bool ImageSource::setupNDI(T &&source)
{
	auto impl = std::make_shared<NDIGrabber>();
	bool ret = impl->setup(source);
	if(ret) {
		impl_ = impl;
	}
	return ret;
}
