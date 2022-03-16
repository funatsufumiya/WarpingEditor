#include "GuiFunc.h"

namespace app {
static inline bool isOpAdd() {
	return ImGui::IsModKeyDown(ImGuiKeyModFlags_Shift);
}
static inline bool isOpToggle() {
#ifdef TARGET_OSX
	return ImGui::IsModKeyDown(ImGuiKeyModFlags_Super);
#else
	return ImGui::IsModKeyDown(ImGuiKeyModFlags_Ctrl);
#endif
}
static inline bool isOpAlt() {
	return ImGui::IsModKeyDown(ImGuiKeyModFlags_Alt);
}
static inline bool isOpDefault() {
	return !(isOpAdd() || isOpToggle() || isOpAlt());
}
}
