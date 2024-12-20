#include "WorkFolder.h"
#include "ofJson.h"

bool WorkFolder::loadJson(const std::filesystem::path &json_path)
{
	auto json = ofLoadJson(json_path);
	std::string rel = json["rel"].get<std::string>();
	std::string abs = json["abs"].get<std::string>();
	return setRelative(rel)
	|| setAbsolute(abs);
}
void WorkFolder::saveJson(const std::filesystem::path &json_path) const
{
	ofSavePrettyJson(json_path, toJson());
}
ofJson WorkFolder::toJson() const
{
	return {
		{"rel", rel_.string()},
		{"abs", abs_.string()}
	};
}

bool WorkFolder::setRelative(const std::filesystem::path &path, bool create_if_not_exist)
{
	return setAbsolute(ofToDataPath(path, true), create_if_not_exist);
}
bool WorkFolder::setAbsolute(const std::filesystem::path &path, bool create_if_not_exist)
{
	ofDirectory dir(path);
	if(create_if_not_exist && !dir.exists()) {
		if(!dir.create(true)) {
			return false;
		}
	}
	if(!dir.exists()) {
		return false;
	}
	abs_ = ofFilePath::getAbsolutePath(path, false);
	rel_ = ofFilePath::makeRelative(ofToDataPath("", true), ofToDataPath(path, true));
	return true;
}
std::filesystem::path WorkFolder::getAbsolute(const std::filesystem::path &path) const
{
    if (path == "") {
        return abs_;
    } else if (ofFilePath::isAbsolute(path)) {
        return path;
    } else {
        return ofFilePath::join(abs_, path);
    }
}

std::filesystem::path WorkFolder::getRelative(const std::filesystem::path &path) const
{
    if (path == "") {
        return rel_;
    } else if (ofFilePath::isAbsolute(path)) {
        return ofFilePath::makeRelative(abs_, path);
    } else {
        return ofFilePath::join(rel_, path);
    }
}

bool WorkFolder::isValid() const
{
	return ofDirectory::doesDirectoryExist(abs_, false);
}
