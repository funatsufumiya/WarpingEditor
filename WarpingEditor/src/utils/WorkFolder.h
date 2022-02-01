#pragma once

#include "ofFileUtils.h"

class WorkFolder
{
public:
	bool loadJson(const std::filesystem::path &json_path);
	void saveJson(const std::filesystem::path &json_path) const;
	bool setRelative(const std::filesystem::path &path, bool create_if_not_exist=false);
	bool setAbsolute(const std::filesystem::path &path, bool create_if_not_exist=false);
	std::filesystem::path getAbsolute(const std::filesystem::path &path="") const;
	std::filesystem::path getRelative(const std::filesystem::path &path="") const;
	
	bool isValid() const;
private:
	std::filesystem::path rel_, abs_;
};
