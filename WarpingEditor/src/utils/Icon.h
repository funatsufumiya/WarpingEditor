#pragma once

#include "ofTexture.h"
#include <vector>
#include <string>
#include <map>
#include "ofImage.h"

class Icon
{
public:
	static GLuint BLANK, SHOW, HIDE, LOCK, UNLOCK, FLAG;
	static void init() {
		textures_.clear();
		std::map<GLuint*, std::string> files{
			{&BLANK, "icon/blank.png"},
			{&SHOW, "icon/show.png"},
			{&HIDE, "icon/hide.png"},
			{&LOCK, "icon/lock.png"},
			{&UNLOCK, "icon/unlock.png"},
			{&FLAG, "icon/flag.png"},
		};
		for(auto &&icon : files) {
			ofTexture tex;
			if(ofLoadImage(tex, icon.second)) {
				*icon.first = tex.getTextureData().textureID;
				textures_.push_back(tex);
			}
		}
	}
private:
	static std::vector<ofTexture> textures_;
};
