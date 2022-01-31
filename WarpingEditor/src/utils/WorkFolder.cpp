#include "WorkFolder.h"
#include "ofJson.h"

bool WorkFolder::loadJson(const std::filesystem::path &json_path)
{
	auto json = ofLoadJson(json_path);
	return setRelative(json["rel"])
	|| setAbsolute(json["abs"]);
}
void WorkFolder::saveJson(const std::filesystem::path &json_path) const
{
	ofJson json{
		{"rel", rel_.string()},
		{"abs", abs_.string()}
	};
	ofSavePrettyJson(json_path, json);
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
	return path == "" ? abs_ : ofFilePath::join(abs_, path);
}
std::filesystem::path WorkFolder::getRelative(const std::filesystem::path &path) const
{
	return path == "" ? rel_ : ofFilePath::join(rel_, path);
}
