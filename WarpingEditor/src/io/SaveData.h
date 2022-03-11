#pragma once

#include "ofConstants.h"
#include <iostream>
#include <filesystem>
#include "ofFileUtils.h"
#include "ofxBlendScreen.h"

class HasSaveData
{
public:
	virtual void pack(std::ostream &stream) const {}
	virtual void unpack(std::istream &stream) {}
};

template<typename Arg>
class HasSaveDataWithArg : public HasSaveData
{
public:
	void pack(std::ostream &stream) const { pack(stream, pack_arg_); }
	void unpack(std::istream &stream) { unpack(stream, unpack_arg_); }
	virtual void pack(std::ostream &stream, const Arg &arg) const {}
	virtual void unpack(std::istream &stream, const Arg &arg) {}
	void setPackArg(const Arg &arg) { pack_arg_ = arg; }
	void setUnpackArg(const Arg &arg) { unpack_arg_ = arg; }
protected:
	Arg pack_arg_, unpack_arg_;
};

class SaveData
{
public:
	void append(char chunk_name[4], std::shared_ptr<HasSaveData> data) {
		data_.push_back({std::string(chunk_name), data});
	}
	void clear() {
		data_.clear();
	}
	void save(const std::filesystem::path &filepath) const {
		ofFile file(filepath, ofFile::WriteOnly);
		pack(file);
		file.close();
	}
	void load(const std::filesystem::path &filepath) {
		ofFile file(filepath, ofFile::ReadOnly);
		unpack(file);
		file.close();
	}
	void pack(std::ostream &stream) const;
	void unpack(std::istream &stream);
	
	template<typename T> static void pack(std::ostream &stream, const T &t);
	template<typename T> static void unpack(std::istream &stream, T &t);
private:
	std::vector<std::pair<std::string, std::shared_ptr<HasSaveData>>> data_;
};

template<> void SaveData::pack<ofxBlendScreen::Shader::Params>(std::ostream &stream, const ofxBlendScreen::Shader::Params &t);
template<> void SaveData::unpack<ofxBlendScreen::Shader::Params>(std::istream &stream, ofxBlendScreen::Shader::Params &t);
