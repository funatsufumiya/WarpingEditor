#pragma once

#include "imgui.h"
#include <math.h>
#include <numeric>

namespace ImGui {
	ImVec2 GetMousePosInCurrentWindow();
	ImVec2 GetMousePosRelativeToCursor();
	void Rectangle(const ImVec2 &offset, const ImVec2 &size, ImU32 color, bool filled, float rounding=0);
	void Circle(const ImVec2 &offset, float radius, ImU32 color, bool filled, int resolution=12);
	bool EditText(const std::string &gui_id, std::string &str, std::size_t buffer_length=256, ImGuiInputTextFlags flags=0);
	bool EditTextEx(const std::string &gui_id, std::string &str, std::size_t buffer_length=256, ImGuiInputTextFlags flags=0);
	bool SelectFile(std::string &path, std::string &selected, const std::vector<std::string> &ext={});
	bool SelectFileMenu(const std::string &path, std::string &selected, bool ignore_root=false, const std::vector<std::string> &ext={});
	void ImageRotated(ImTextureID tex_id, ImVec2 center, ImVec2 size, float angle);
	bool SpinnerInt(const std::string &gui_id, int &value, int min=std::numeric_limits<int>::min(), int max=std::numeric_limits<int>::max(), int step=1);
	bool IsModKeyDown(ImGuiKeyModFlags mod);
	bool IsKeyDown(int key_index, ImGuiKeyModFlags mod);
	bool IsKeyDownMac(int key_index, ImGuiKeyModFlags mod);
	bool IsKeyDownWin(int key_index, ImGuiKeyModFlags mod);
}
