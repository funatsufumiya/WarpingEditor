#include "SaveData.h"
#include "ofxBlendScreen.h"

namespace {
template<typename T>
void writeTo(std::ostream& os, const T& t) {
	os.write(reinterpret_cast<const char*>(&t), sizeof(T));
}
template<typename T>
void readFrom(std::istream& is, T& t) {
	is.read(reinterpret_cast<char*>(&t), sizeof(T));
}
template<>
void writeTo<glm::vec3>(std::ostream &os, const glm::vec3 &t) {
	writeTo(os, t[0]);
	writeTo(os, t[1]);
	writeTo(os, t[2]);
}
template<>
void readFrom<glm::vec3>(std::istream &is, glm::vec3 &t) {
	readFrom(is, t[0]);
	readFrom(is, t[1]);
	readFrom(is, t[2]);
}
template<>
void writeTo<ofxBlendScreen::Shader::Params>(std::ostream &os, const ofxBlendScreen::Shader::Params &t) {
	writeTo(os, t.gamma);
	writeTo(os, t.luminance_control);
	writeTo(os, t.blend_power);
	writeTo(os, t.base_color);
}
template<>
void readFrom<ofxBlendScreen::Shader::Params>(std::istream &is, ofxBlendScreen::Shader::Params &t) {
	readFrom(is, t.gamma);
	readFrom(is, t.luminance_control);
	readFrom(is, t.blend_power);
	readFrom(is, t.base_color);
}
}
template<>
void SaveData::pack<ofxBlendScreen::Shader::Params>(std::ostream &stream, const ofxBlendScreen::Shader::Params &t)
{
	writeTo(stream, t);
}
template<>
void SaveData::unpack<ofxBlendScreen::Shader::Params>(std::istream &stream, ofxBlendScreen::Shader::Params &t)
{
	readFrom(stream, t);
}

void SaveData::pack(std::ostream &stream) const
{
	// header
	stream << "maap";
	writeTo<std::size_t>(stream, 1);	// version number

	for(auto &&d : data_) {
		stream << d.first;
		auto pos_to_write_chunksize = stream.tellp();
		writeTo<std::size_t>(stream, 0);	// placeholder for chunksize
		auto begin_chunksize = stream.tellp();
		d.second->pack(stream);
		auto end_pos = stream.tellp();
		auto chunksize = end_pos - begin_chunksize;
		stream.seekp(pos_to_write_chunksize, std::ios_base::beg);
		writeTo(stream, chunksize);
		stream.seekp(end_pos, std::ios_base::beg);
	}
}

void SaveData::unpack(std::istream &stream)
{
	// header
	char maap[4];
	readFrom(stream, maap);
	assert(strncmp(maap, "maap", 4) == 0);
	std::size_t version;
	readFrom(stream, version);
	
	while(stream.good()) {
		char chunkname[4];
		std::size_t chunksize;
		readFrom(stream, chunkname);
		readFrom(stream, chunksize);
		if(stream.eof() || stream.fail()) {
			break;
		}
		auto next_pos = std::size_t(stream.tellg()) + chunksize;
		auto found = find_if(begin(data_), end(data_), [chunkname](const std::pair<std::string, std::shared_ptr<HasSaveData>> &p) {
			return strncmp(chunkname, p.first.c_str(), 4) == 0;
		});
		if(found == end(data_)) {
			ofLogWarning("SaveData") << "skipped unhandled chunk: " << chunkname;
		}
		else {
			found->second->unpack(stream);
		}
		stream.seekg(next_pos, std::ios_base::beg);
	}
}

