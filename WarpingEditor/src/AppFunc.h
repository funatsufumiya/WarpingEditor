#include "GuiFunc.h"

namespace app {
static inline bool isOpAdd() {
	return ImGui::IsModKeyDown(ImGuiKeyModFlags_Shift);
}
static inline bool isOpAlt() {
#ifdef TARGET_OSX
	return ImGui::IsModKeyDown(ImGuiKeyModFlags_Super);
#else
	return ImGui::IsModKeyDown(ImGuiKeyModFlags_Ctrl);
#endif
}
static inline bool isOpDefault() {
	return !(isOpAdd() || isOpAlt());
}
}
