#pragma once

#include "Quad.h"
#include <string>
#include "imgui.h"
#include "ofTexture.h"

namespace ImGui {
void ImageUVRect(ImTextureID user_texture_id, const ImVec2& uv_a, const ImVec2& uv_b, const ImVec2& uv_c, const ImVec2& uv_d, const ImVec2& size = ImVec2(0,0), const ImVec4& tint_col = ImVec4(1,1,1,1), const ImVec4& border_col = ImVec4(0,0,0,0));
void ImageWithUVOverlapped(ImTextureID user_texture_id, const ImVec2& uv_a, const ImVec2& uv_b, const ImVec2& uv_c, const ImVec2& uv_d, const ImVec2& size = ImVec2(0,0), const ImVec4& base_tint_col = ImVec4(0.5f,0.5f,0.5f,1), const ImVec4& base_border_col = ImVec4(0,0,0,0), const ImVec4& overlap_tint_col = ImVec4(1,1,1,1), const ImVec4& overlap_border_col = ImVec4(1,1,1,1));
bool Drag2DButton(const char* label, ImVec2 &value, const ImVec2 &step=ImVec2(1,1), const char *value_format="%.1f", const ImVec2& size=ImVec2(0,0));
bool Drag2DButton(const char* label, ImVec2 &value, float step=1.f, const char *value_format="%.1f", const ImVec2& size=ImVec2(0,0));
bool EditUVQuad(const char* label, geom::Quad &quad, const ImVec2 &uv_size=ImVec2(1,1));
bool EditUVQuad(const char* label, geom::Quad &quad, ofTexture texture);
}
