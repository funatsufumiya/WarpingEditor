#pragma once

#include "imgui.h"
#include <math.h>
#include <numeric>
#include <glm/fwd.hpp>

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
	bool ToggleButton(const std::string &gui_id, bool &value, GLuint true_tex, GLuint false_tex, ImVec2 size={0,0}, int frame_padding = -1, const ImVec4& bg_col = ImVec4(0,0,0,0), const ImVec4& tint_col = ImVec4(1,1,1,1));

	struct ScalarAsParam {
		ScalarAsParam() {}
		ScalarAsParam(const char *format)
		:format(format) {}
		union Value {
			char s_8; unsigned char u_8;
			short s_16; unsigned short u_16;
			int s_32; unsigned int u_32;
			long long s_64; unsigned long long u_64;
			float f;
			double d;
		};
		ImGuiDataType type;
		const char *format;
		private:
			template<typename T> void setType();
			template<> void setType<char>() { type = ImGuiDataType_S8; }
			template<> void setType<unsigned char>() { type = ImGuiDataType_U8; }
			template<> void setType<short>() { type = ImGuiDataType_S16; }
			template<> void setType<unsigned short>() { type = ImGuiDataType_U16; }
			template<> void setType<int>() { type = ImGuiDataType_S32; }
			template<> void setType<unsigned int>() { type = ImGuiDataType_U32; }
			template<> void setType<long long>() { type = ImGuiDataType_S64; }
			template<> void setType<unsigned long long>() { type = ImGuiDataType_U64; }
			template<> void setType<float>() { type = ImGuiDataType_Float; }
			template<> void setType<double>() { type = ImGuiDataType_Double; }
	};
	struct SliderScalarAsParam : public ScalarAsParam {
		SliderScalarAsParam() {}
		template<typename T>
		SliderScalarAsParam(glm::vec<2, T> range, const char *format)
		:ScalarAsParam(format) {
			memcpy(&this->v_min, &range[0], sizeof(T));
			memcpy(&this->v_max, &range[1], sizeof(T));
			setType<T>();
		}
		Value v_min, v_max;
		float getPosition(ImGui::ScalarAsParam::Value value, float v_min, float v_max);
		ImGui::ScalarAsParam::Value getValue(float v, float v_min, float v_max);
	};
	struct DragScalarAsParam : public SliderScalarAsParam {
		DragScalarAsParam() {}
		template<typename T>
		DragScalarAsParam(glm::vec<2, T> range, float speed, const char *format)
		:SliderScalarAsParam(range, format),speed(speed) {
		}
		float speed;
	};
	bool SliderFloatAs(const std::string &label, float *v, float v_min, float v_max, std::vector<std::pair<std::string, SliderScalarAsParam>> params, ImGuiSliderFlags flags=0);
	bool SliderFloatAs(const std::string &label, float *v, float v_min, float v_max, std::pair<std::string, SliderScalarAsParam> param, ImGuiSliderFlags flags=0);
	bool SliderFloatNAs(const std::string &label, float *v, int num_components, const float *v_min, const float *v_max, std::vector<std::pair<std::string, std::vector<SliderScalarAsParam>>> params, ImGuiSliderFlags flags=0);

	bool DragFloatAs(const std::string &label, float *v, float v_min, float v_max, bool clamp_min, bool clamp_max, std::vector<std::pair<std::string, DragScalarAsParam>> params, ImGuiSliderFlags flags=0);
	bool DragFloatAs(const std::string &label, float *v, float v_min, float v_max, bool clamp_min, bool clamp_max, std::pair<std::string, DragScalarAsParam> param, ImGuiSliderFlags flags=0);
	bool DragFloatNAs(const std::string &label, float *v, int components, const float *v_min, const float *v_max, bool *clamp_min, bool *clamp_max, std::vector<std::pair<std::string, std::vector<DragScalarAsParam>>> params, ImGuiSliderFlags flags=0);

}
