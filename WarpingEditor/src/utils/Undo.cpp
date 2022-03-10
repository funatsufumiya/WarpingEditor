#include "Undo.h"
#include "ofApp.h"

namespace {
uint32_t crc32_table[256];
bool crc32_table_init = false;
void make_crc32_table(void) {
	for (uint32_t i = 0; i < 256; i++) {
		uint32_t c = i;
		for (int j = 0; j < 8; j++) {
			c = (c & 1) ? (0xEDB88320 ^ (c >> 1)) : (c >> 1);
		}
		crc32_table[i] = c;
	}
	crc32_table_init = true;
}

uint32_t crc32(uint8_t *buf, std::size_t len) {
	if(!crc32_table_init) {
		make_crc32_table();
	}
	uint32_t c = 0xFFFFFFFF;
	for (size_t i = 0; i < len; i++) {
		c = crc32_table[(c ^ buf[i]) & 0xFF] ^ (c >> 8);
	}
	return c ^ 0xFFFFFFFF;
}
uint32_t crc32(const std::string &str) {
	return crc32((uint8_t*)str.data(), str.length());
}

}
uint32_t UndoDescriptor::getUndoStateDescriptor()
{
	return crc32(undo_.create());
}

void Undo::setup(GuiApp *app)
{
	app_ = app;
}
void Undo::enableAuto(float check_interval)
{
	enableModifyChecker(descriptor_, check_interval);
}

void Undo::disableAuto()
{
	disableModifyChecker();
}

Undo::DataType Undo::create() const
{
	std::stringstream stream;
	app_->packDataFile(stream);
	cache_ = stream.str();
	return cache_;
}
Undo::DataType Undo::createUndo() const
{
	return cache_;
}
void Undo::loadUndo(const DataType &data)
{
	std::stringstream stream(data);
	app_->unpackDataFile(stream);
	cache_ = data;
}
