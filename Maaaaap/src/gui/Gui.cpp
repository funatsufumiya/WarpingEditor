#include "Gui.h"
#include "imgui_internal.h"
#include "imgui.h"

void ImGui::ImageUVRect(ImTextureID user_texture_id, const ImVec2& uv_a, const ImVec2& uv_b, const ImVec2& uv_c, const ImVec2& uv_d, const ImVec2& size_arg, const ImVec4& tint_col, const ImVec4& border_col) {
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return;

	const ImVec2 content_avail = GetContentRegionAvail();
	ImVec2 size = ImFloor(size_arg);
	if (size.x <= 0.0f)
		size.x = ImMax(content_avail.x + size.x, 4.0f); // Arbitrary minimum child size (0.0f causing too much issues)
	if (size.y <= 0.0f)
		size.y = ImMax(content_avail.y + size.y, 4.0f);

	ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
	if (border_col.w > 0.0f)
		bb.Max = bb.Max + ImVec2(2, 2);
	ItemSize(bb);
	if (!ItemAdd(bb, 0))
		return;

	ImVec2 p1(bb.Min), p2(bb.Max.x, bb.Min.y), p3(bb.Max), p4(bb.Min.x, bb.Max.y);
	if (border_col.w > 0.0f)
	{
		window->DrawList->AddRect(bb.Min, bb.Max, GetColorU32(border_col), 0.0f);
		window->DrawList->AddImageQuad(user_texture_id, p1+ImVec2(1, 1), p2+ImVec2(-1, 1), p3+ImVec2(-1, -1), p4+ImVec2(1, -1), uv_a, uv_b, uv_c, uv_d, GetColorU32(tint_col));
	}
	else
	{
		window->DrawList->AddImageQuad(user_texture_id, p1, p2, p3, p4, uv_a, uv_b, uv_c, uv_d, GetColorU32(tint_col));
	}
}

void ImGui::ImageWithUVOverlapped(ImTextureID user_texture_id, const ImVec2& uv_a, const ImVec2& uv_b, const ImVec2& uv_c, const ImVec2& uv_d, const ImVec2& size_arg, const ImVec4& base_tint_col, const ImVec4& base_border_col, const ImVec4& overlap_tint_col, const ImVec4& overlap_border_col) {
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return;

	const ImVec2 content_avail = GetContentRegionAvail();
	ImVec2 size = ImFloor(size_arg);
	if (size.x <= 0.0f)
		size.x = ImMax(content_avail.x + size.x, 4.0f); // Arbitrary minimum child size (0.0f causing too much issues)
	if (size.y <= 0.0f)
		size.y = ImMax(content_avail.y + size.y, 4.0f);

	ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
	if (base_border_col.w > 0.0f)
		bb.Max = bb.Max + ImVec2(2, 2);
	ItemSize(bb);
	if (!ItemAdd(bb, 0))
		return;

	if (base_border_col.w > 0.0f)
	{
		window->DrawList->AddRect(bb.Min, bb.Max, GetColorU32(base_border_col), 0.0f);
		window->DrawList->AddImage(user_texture_id, bb.Min + ImVec2(1, 1), bb.Max - ImVec2(1, 1), {0,0}, {1,1}, GetColorU32(base_tint_col));
	}
	else
	{
		window->DrawList->AddImage(user_texture_id, bb.Min, bb.Max, {0,0}, {1,1}, GetColorU32(base_tint_col));
	}

	ImVec2 p1 = ImLerp(bb.Min, bb.Max, uv_a)
	, p2 = ImLerp(bb.Min, bb.Max, uv_b)
	, p3 = ImLerp(bb.Min, bb.Max, uv_c)
	, p4 = ImLerp(bb.Min, bb.Max, uv_d);
	window->DrawList->AddImageQuad(user_texture_id, p1, p2, p3, p4, uv_a, uv_b, uv_c, uv_d, GetColorU32(overlap_tint_col));
	window->DrawList->AddQuad(p1, p2, p3, p4, GetColorU32(overlap_border_col), 0.0f);
}

bool ImGui::Drag2DButton(const char* label, ImVec2 &value, const ImVec2 &step, const char *value_format, const ImVec2& size) {
	char format[256] = {};
	ImFormatString(format, 256, "%s(%s,%s)###%s", "%s", value_format, value_format, label);
	char buf[256]={};
	ImFormatString(buf, 256, format, label, value.x, value.y);
	Button(buf, size);
	auto &io = GetIO();
	if (IsItemActive()) {
		ImGui::GetForegroundDrawList()->AddLine(io.MouseClickedPos[0], io.MousePos, ImGui::GetColorU32(ImGuiCol_Button), 4.0f);
		auto delta = io.MouseDelta;
		value = value + delta*step;
		return delta.x != 0 || delta.y != 0;
	}
	return false;
}
bool ImGui::Drag2DButton(const char* label, ImVec2 &value, float step, const char *value_format, const ImVec2& size) {
	return Drag2DButton(label, value, ImVec2(step, step), value_format, size);
}

bool ImGui::EditUVQuad(const char* label, geom::Quad &quad, const ImVec2 &uv_size) {
	bool ret = false;
	glm::vec2 range_x = {0, uv_size.x};
	glm::vec2 range_y = {0, uv_size.y};
#define EditVertex(name) \
	ImVec2 name(quad.name.x, quad.name.y); \
	if(Drag2DButton(#name, name, 0.001f, "%.3f")) { \
		quad.name.x = ImClamp(name.x, range_x[0], range_x[1]); \
		quad.name.y = ImClamp(name.y, range_y[0], range_y[1]); \
		ret |= true; \
	}
	EditVertex(lt); SameLine();
	EditVertex(rt);
	EditVertex(lb); SameLine();
	EditVertex(rb);
#undef EditVertex
	return ret;
}
bool ImGui::EditUVQuad(const char* label, geom::Quad &quad, ofTexture texture) {
	return EditUVQuad(label, quad, ImVec2(texture.getTextureData().tex_t, texture.getTextureData().tex_u));
}

