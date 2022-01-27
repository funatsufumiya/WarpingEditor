#include "ImageSource.h"
#include "ofImage.h"
#include "ofFileUtils.h"
#include "ofVideoPlayer.h"

namespace {
class ImageFile : public ImageSourceImpl {
public:
	bool load(const std::filesystem::path &filepath) {
		return ofLoadImage(texture_, filepath);
	}
	ofTexture& getTexture() override { return texture_; }
	const ofTexture& getTexture() const override { return texture_; };
protected:
	ofTexture texture_;
};
class VideoFile : public ImageSourceImpl {
public:
	bool load(const std::filesystem::path &filepath) {
		player_.setUseTexture(true);
		bool ret = player_.load(filepath.string());
		player_.setLoopState(OF_LOOP_NORMAL);
		player_.play();
		return ret;
	}
	void update() override {
		player_.update();
	}
	bool isFrameNew() const override { return player_.isFrameNew(); }
	ofTexture& getTexture() override { return player_.getTexture(); }
	const ofTexture& getTexture() const override { return player_.getTexture(); };
private:
	ofVideoPlayer player_;
};
}

bool ImageSource::loadFromFile(const std::filesystem::path &filepath)
{
	auto hasExt = [filepath](std::vector<std::string> ext) {
		return ofContains(ext, ofFilePath::getFileExt(filepath));
	};
	bool is_image = hasExt({"png","jpg","jpeg","tif","tiff","bmp","gif"});
	bool is_video = hasExt({"mov","mp4","mpg","wmv"});
	if(is_image) {
		auto impl = std::make_shared<ImageFile>();
		bool ret = impl->load(filepath);
		if(ret) {
			impl_ = impl;
		}
		return ret;
	}
	if(is_video) {
		auto impl = std::make_shared<VideoFile>();
		bool ret = impl->load(filepath);
		if(ret) {
			impl_ = impl;
		}
		return ret;
	}
	return false;
}

