#include "GuiFunc.h"
#include "imgui_internal.h"
#include "ofFileUtils.h"
#include "ofUtils.h"
#include "ofSystemUtils.h"

ImVec2 ImGui::GetMousePosInCurrentWindow()
{
	return GetMousePos() - GetWindowPos();
}

ImVec2 ImGui::GetMousePosRelativeToCursor()
{
	return GetMousePosInCurrentWindow() - GetCursorPos();
}

void ImGui::Rectangle(const ImVec2 &offset, const ImVec2 &size, ImU32 color, bool filled, float rounding)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	ImVec2 pos = window->DC.CursorPos;
	const ImRect bb(pos, pos + offset + size);
	ItemSize(bb, style.FramePadding.y);
	if(!ItemAdd(bb, 0)) {
		return;
	}
	if(filled) {
		RenderFrame(pos+offset, pos+offset+size, color, false);
	}
	else {
		const float border_size = g.Style.FrameBorderSize;
        window->DrawList->AddRect(pos+offset, pos+offset+size, color, rounding, ImDrawCornerFlags_All, border_size);
	}
}
void ImGui::Circle(const ImVec2 &offset, float radius, ImU32 color, bool filled, int num_segments)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	ImVec2 pos = window->DC.CursorPos;
	const ImRect bb(pos+offset-ImVec2(radius,radius), pos + offset + ImVec2(radius*2,radius*2));
	ItemSize(bb, style.FramePadding.y);
	if(!ItemAdd(bb, 0)) {
		return;
	}
	if(filled) {
		window->DrawList->AddCircleFilled(pos+offset, radius, color, num_segments);
	}
	else {
		const float border_size = g.Style.FrameBorderSize;
		window->DrawList->AddCircle(pos+offset, radius, color, num_segments, border_size);
	}
}

bool ImGui::EditText(const std::string &gui_id, std::string &str, std::size_t buffer_length, ImGuiInputTextFlags flags)
{
	std::vector<char> buf(buffer_length);
	memcpy(buf.data(), str.c_str(), str.length()+1);
	if(InputText(gui_id.c_str(), buf.data(), buffer_length, flags)) {
		str = buf.data();
		return true;
	}
	return false;
}
bool ImGui::EditTextEx(const std::string &gui_id, std::string &str, std::size_t buffer_length, ImGuiInputTextFlags flags)
{
	bool ret = EditText(gui_id, str, buffer_length, flags);
	SameLine();
	if(ArrowButton("##edit", ImGuiDir_Right)) {
		auto result = ofSystemTextBoxDialog(gui_id, str);
		if(result != str) {
			str = result;
			return true;
		}
	}
	return ret;
}

namespace {
bool SelectFileImpl(const std::string &path, std::string &selected, const std::vector<std::string> &ext, std::string &refpath)
{
	using namespace ImGui;
	bool ret = false;
	if(ofFile(path).isDirectory()) {
		if(TreeNodeEx(ofFilePath::getBaseName(path).c_str(), ImGuiTreeNodeFlags_OpenOnArrow)) {
			ofDirectory dir;
			if(!ext.empty()) {
				dir.allowExt("");
				for(auto &&e : ext) {
					dir.allowExt(e);
				}
			}
			dir.listDir(path);
			for(auto &f : dir) {
				ret |= SelectFileImpl(f.path(), selected, ext, refpath);
			}
			TreePop();
		}
		if(IsMouseDoubleClicked(0) && IsItemHovered()) {
			refpath = path;
		}
	}
	else {
		Indent();
		if(Selectable(ofFilePath::getFileName(path).c_str())) {
			selected = path;
			ret = true;
		}
		Unindent();
	}
	return ret;
}
}
bool ImGui::SelectFile(std::string &path, std::string &selected, const std::vector<std::string> &ext)
{
	Text("current: %s", path.c_str());
	if(Button("up")) {
		path = ofToDataPath(ofFilePath::getEnclosingDirectory(path+"/../"), true);
	}
	return SelectFileImpl(path, selected, ext, path);
}


bool ImGui::SelectFileMenu(const std::string &path, std::string &selected, bool ignore_root, const std::vector<std::string> &ext)
{
	using namespace ImGui;
	bool ret = false;
	if(ofFile(path).isDirectory()) {
		if(ignore_root || BeginMenu(ofFilePath::getBaseName(path).c_str())) {
			ofDirectory dir;
			if(!ext.empty()) {
				dir.allowExt("");
				for(auto &&e : ext) {
					dir.allowExt(e);
				}
			}
			dir.listDir(path);
			dir.sort();
			for(auto &f : dir) {
				ret |= SelectFileMenu(f.path(), selected, false, ext);
			}
			if(!ignore_root) EndMenu();
		}
	}
	else {
		std::string relativepath = ofFilePath::makeRelative(ofToDataPath(""), path);
		if(MenuItem(ofFilePath::getFileName(path).c_str(), "", selected == relativepath)) {
			selected = relativepath;
			ret = true;
		}
	}

	return ret;
}

void ImGui::ImageRotated(ImTextureID tex_id, ImVec2 center, ImVec2 size, float angle)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return;
	
	ImDrawList* draw_list = GetWindowDrawList();
	auto cursor = GetCursorScreenPos();
	auto anchor = center + cursor;
	
	float cos_a = cosf(angle);
	float sin_a = sinf(angle);
	ImVec2 pos[4] =
	{
		anchor + ImRotate(ImVec2(-size.x * 0.5f, -size.y * 0.5f), cos_a, sin_a),
		anchor + ImRotate(ImVec2(+size.x * 0.5f, -size.y * 0.5f), cos_a, sin_a),
		anchor + ImRotate(ImVec2(+size.x * 0.5f, +size.y * 0.5f), cos_a, sin_a),
		anchor + ImRotate(ImVec2(-size.x * 0.5f, +size.y * 0.5f), cos_a, sin_a)
	};
	ImVec2 uvs[4] = 
	{ 
		ImVec2(0.0f, 0.0f), 
		ImVec2(1.0f, 0.0f), 
		ImVec2(1.0f, 1.0f), 
		ImVec2(0.0f, 1.0f) 
	};
	
	draw_list->AddImageQuad(tex_id, pos[0], pos[1], pos[2], pos[3], uvs[0], uvs[1], uvs[2], uvs[3], IM_COL32_WHITE);
	
	ImVec2 posmin(std::min<float>({pos[0].x,pos[1].x,pos[2].x,pos[3].x}), std::min<float>({pos[0].y,pos[1].y,pos[2].y,pos[3].y}));
	ImVec2 posmax(std::max<float>({pos[0].x,pos[1].x,pos[2].x,pos[3].x}), std::max<float>({pos[0].y,pos[1].y,pos[2].y,pos[3].y}));
	
	ImRect bb(posmin, posmax);
	ItemSize(center+(posmax-posmin)/2.f);
	ItemAdd(bb, 0);
}

bool ImGui::SpinnerInt(const std::string &gui_id, int &value, int min, int max, int step)
{
	int cache = value;
	PushID(gui_id.c_str());
	BeginGroup();
	if(ArrowButton("-", ImGuiDir_Left)) { value = glm::clamp(value-step, min, max); }
	SameLine();
	PushItemWidth(24);
	if(InputInt("##value", &value, 0)) { value = glm::clamp(value, min, max); }
	PopItemWidth();
	SameLine();
	if(ArrowButton("+", ImGuiDir_Right)) { value = glm::clamp(value+step, min, max); }
	EndGroup();
	PopID();
	return cache != value;
}


bool ImGui::IsModKeyDown(ImGuiKeyModFlags mod)
{
	auto &&io = ImGui::GetIO();
	return io.KeyMods == mod;
}

bool ImGui::IsKeyDown(int key_index, ImGuiKeyModFlags mod)
{
	auto &&io = ImGui::GetIO();
	return io.KeysDown[key_index] && IsModKeyDown(mod);
}
bool ImGui::IsKeyDownMac(int key_index, ImGuiKeyModFlags mod)
{
#ifdef TARGET_OSX
	return IsKeyDown(key_index, mod);
#else
	return false;
#endif
}
bool ImGui::IsKeyDownWin(int key_index, ImGuiKeyModFlags mod)
{
#ifdef TARGET_WIN32
	return IsKeyDown(key_index, mod);
#else
	return false;
#endif
}
