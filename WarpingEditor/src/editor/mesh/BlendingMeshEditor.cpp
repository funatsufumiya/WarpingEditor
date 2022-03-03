#include "BlendingMeshEditor.h"

namespace {
template <typename ... Args>
std::string format(const std::string& fmt, Args ... args )
{
	size_t len = std::snprintf( nullptr, 0, fmt.c_str(), args ... );
	std::vector<char> buf(len + 1);
	std::snprintf(&buf[0], len + 1, fmt.c_str(), args ... );
	return std::string(&buf[0], &buf[0] + len);
}}
void BlendingMeshEditor::gui()
{
	using namespace ImGui;
	auto &&data = *data_;
	
	struct GuiMesh {
		std::string label;
		std::shared_ptr<MeshType> mesh;
	};
	struct GuiPoint {
		std::string label;
		std::shared_ptr<MeshType> mesh;
		MeshType::PointRef point;
	};
	std::vector<GuiMesh> meshes;
	std::vector<GuiPoint> points;
	std::function<bool(GuiPoint)> guiPoint;
	std::function<bool(GuiMesh)> guiMesh = [&](GuiMesh mesh) {
		bool ret = false;
		auto d = data.find(mesh.mesh);
		if(!d.second) {
			return false;
		}
		if(TreeNode(mesh.label.c_str())) {
			for(auto &&p : d.second->interpolator->getSelected()) {
				ret |= guiPoint({format("[%d,%d]", p.col, p.row), mesh.mesh, p});
			}
			TreePop();
		}
		return ret;
	};
	float v_min[2] = {0,0};
	float v_max[2] = {tex_.getWidth(), tex_.getHeight()};
	std::vector<std::pair<std::string, std::vector<ImGui::DragScalarAsParam>>> params{
		{"px", {
			{glm::ivec2{0, tex_.getWidth()}, 1, "%d"},
			{glm::ivec2{0, tex_.getHeight()}, 1, "%d"}
		}},
		{"%", {
			{glm::vec2{0, 100}, 0.1f, "%.02f%%"}
		}},
		{"rate", {
			{glm::vec2{0, 1}, 0.001f, "%.03f"}
		}},
	};
	if(Begin("Mesh")) {
		if(BeginTabBar("#filter")) {
			if(BeginTabItem("selected")) {
				for(auto &&weak : op_selection_.mesh) {
					auto mesh = data.find(weak.lock());
					if(mesh.second) {
						auto m = getMeshType(*mesh.second);
						meshes.push_back({mesh.first, m});
					}
				}
				for(auto &&point : op_selection_.point) {
					auto mesh = data.find(point.first.lock());
					if(mesh.second) {
						auto m = getMeshType(*mesh.second);
						for(IndexType i : point.second) {
							points.emplace_back(GuiPoint{format("%s[%d,%d]", mesh.first.c_str(), i.first, i.second), m, m->getPoint(i.first, i.second)});
						}
					}
				}
				EndTabItem();
			}
			if(BeginTabItem("editable")) {
				for(auto &&mesh : data.getEditableMesh()) {
					if(mesh.second) {
						auto m = getMeshType(*mesh.second);
						meshes.push_back({mesh.first, m});
					}
				}
				EndTabItem();
			}
			if(BeginTabItem("all")) {
				for(auto &&mesh : data.getMesh()) {
					if(mesh.second) {
						auto m = getMeshType(*mesh.second);
						meshes.push_back({mesh.first, m});
					}
				}
				EndTabItem();
			}
			EndTabBar();
		};
		if(BeginTabBar("#mode")) {
			if(BeginTabItem("Vertex")) {
				guiPoint = [=](GuiPoint p) {
					return DragFloatNAs(p.label, &p.point.v->x, 2, v_min, v_max, nullptr, nullptr, params, ImGuiSliderFlags_NoRoundToFormat);
				};
				EndTabItem();
			}
			if(BeginTabItem("Color")) {
				guiPoint = [=](GuiPoint p) {
					Text("%s", p.label.c_str()); SameLine();
					return ColorEdit4(p.label.c_str(), &p.point.c->r, ImGuiColorEditFlags_NoLabel);
				};
				EndTabItem();
			}
			EndTabBar();
		}
		for(auto &&m : meshes) {
			if(guiMesh(m)) {
				data.find(m.mesh).second->setDirty();
			}
		}
		for(auto &&p : points) {
			if(guiPoint(p)) {
				data.find(p.mesh).second->setDirty();
			}
		}
	}
	End();
	if(Begin("Move Selected Together")) {
		glm::vec2 v_min{0,0};
		glm::vec2 v_max{tex_.getWidth(), tex_.getHeight()};
		std::vector<std::pair<std::string, std::vector<ImGui::DragScalarAsParam>>> params{
			{"px", {
				{glm::ivec2{0, tex_.getWidth()}, 1, "%d"},
				{glm::ivec2{0, tex_.getHeight()}, 1, "%d"}
			}},
			{"%", {
				{glm::vec2{0, 100}, 0.1f, "%.02f%%"}
			}},
			{"rate", {
				{glm::vec2{0, 1}, 0.001f, "%.03f"}
			}},
		};
		std::function<void(glm::vec2)> func;
		if(BeginTabBar("#mode")) {
			if(BeginTabItem("Vertex")) {
				func = [&](glm::vec2 delta) {
					moveSelected(delta);
				};
				EndTabItem();
			}
			EndTabBar();
		}
		auto result = gui2DPanel("panel", &v_min.x, &v_max.x, params);
		if(result.first) {
			func(result.second);
		}
	}
	End();
	if(Begin("Grid")) {
		Checkbox("show", &grid_.is_show);
		Checkbox("snap", &grid_.enabled_snap);
		DragFloatNAs("offset", &grid_.offset.x, 2, v_min, v_max, nullptr, nullptr, params, ImGuiSliderFlags_NoRoundToFormat);
		bool clamp_min[] = {true, true};
		DragFloatNAs("size", &grid_.size.x, 2, v_min, v_max, clamp_min, nullptr, params, ImGuiSliderFlags_NoRoundToFormat);
	}
	End();
}
