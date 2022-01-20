#pragma once

#include "ofGLBaseTypes.h"

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
	void update() { impl_->update(); }
	bool isFrameNew() const { return impl_->isFrameNew(); }
	
	ofTexture& getTexture() override { return impl_->getTexture(); }
	const ofTexture& getTexture() const override { return impl_->getTexture(); };
	void setUseTexture(bool bUseTex) override { impl_->setUseTexture(bUseTex); }
	bool isUsingTexture() const override { return impl_->isUsingTexture(); }

protected:
	std::shared_ptr<ImageSourceImpl> impl_;
};

