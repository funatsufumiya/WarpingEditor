#include "WarpingMeshEditor.h"
#include "ofGraphics.h"

namespace {
ofMesh makeCross(const WarpingMeshEditor::PointType &point, const ofColor &color, float line_length, float stroke_width, float degree)
{
	ofMesh ret;
	ret.setMode(OF_PRIMITIVE_TRIANGLES);
	glm::vec3 p{point, 0};
	ret.addVertex(p);
	float hl = line_length/2.f;
	float hs = stroke_width/2.f;
	for(int i = 0; i < 4; ++i) {
		ofIndexType index = (ofIndexType)ret.getNumVertices();
		float rot = ofDegToRad(i*90+degree);
		ret.addVertex(glm::vec3(glm::rotate(glm::vec2{hl,hs}, rot),0)+p);
		ret.addVertex(glm::vec3(glm::rotate(glm::vec2{hs,hs}, rot),0)+p);
		ret.addVertex(glm::vec3(glm::rotate(glm::vec2{hs,hl}, rot),0)+p);
		ret.addIndices({
			0,index+1,index,
			0,index+2,index+1,
			0,index+3,index+2,
		});
	}
	ret.addVertex(ret.getVertex(1));
	ret.getColors().assign(ret.getNumVertices(), color);
	
	return ret;
}
bool isCorner(const WarpingMeshEditor::MeshType &mesh, WarpingMeshEditor::IndexType index) {
	int x = index.first, y = index.second;
	return (x==0||x==mesh.getNumCols()) && (y==0||y==mesh.getNumRows());
}
}

void WarpingMeshEditor::moveMesh(MeshType &mesh, const glm::vec2 &delta)
{
	app::isOpAlt()
	? MeshEditor::moveMeshCoord(mesh, -delta/glm::vec2{tex_.getWidth(), tex_.getHeight()})
	: MeshEditor::moveMesh(mesh, delta);
}
void WarpingMeshEditor::movePoint(MeshType &mesh, IndexType index, const glm::vec2 &delta)
{
	app::isOpAlt()
	? MeshEditor::movePointCoord(mesh, index, -delta/glm::vec2{tex_.getWidth(), tex_.getHeight()})
	: MeshEditor::movePoint(mesh, index, delta);
}

void WarpingMeshEditor::update()
{
	auto mode_prev = mode_;
	if(ofGetKeyPressed('d')) {
		mode_ = MODE_DIVISION;
		setEnabledHoveringUneditablePoint(true);
		setEnabledRectSelection(false);
		setEnableMoveMeshByMouse(false);
		is_div_point_valid_ = false;
	}
	if(ofGetKeyPressed('m')) {
		mode_ = MODE_MESH;
		setEnabledHoveringUneditablePoint(false);
		setEnabledRectSelection(true);
		setEnableMoveMeshByMouse(true);
	}
	if(mode_ != mode_prev) {
		op_selection_ = OpSelection();
		op_hover_ = OpHover();
		op_rect_ = OpRect();
	}
	MeshEditor::update();
	auto &&data = *data_;
	if(mode_ == MODE_DIVISION) {
		if(mouse_.isFrameNew()) {
			is_div_point_valid_ = false;
			glm::vec2 dst_findex;
			bool is_row=false, is_col=false;
			std::shared_ptr<DataType> div_mesh;
			if(op_hover_.point.first.expired()) {
				div_point_ = getIn(mouse_.pos);
				if(grid_.enabled_snap) {
					glm::vec2 diff;
					if(calcSnapToGrid(div_point_, {grid_.offset, grid_.offset+grid_.size}, mouse_near_distance_/getScale(), diff)) {
						div_point_ += diff;
					}
				}
				op_hover_ = getHover(getOut(div_point_), true);
				div_mesh = data.find(op_hover_.mesh.lock()).second;

				if(div_mesh && data.isEditable(div_mesh)) {
					is_div_point_valid_ = true;
					auto mesh = getMeshType(*div_mesh);
					glm::vec2 result;
					if(mesh->getNearestPointOnLine(div_point_, dst_findex, result, is_row, mouse_near_distance_/getScale())) {
						div_point_ = result;
						is_col = !is_row;
					}
					else if(mesh->getIndexOfPoint(div_point_, dst_findex)) {
						is_row = is_col = true;
					}
				}
			}
			if(mouse_.isClicked(OF_MOUSE_BUTTON_LEFT)) {
				if(is_div_point_valid_) {
					auto mesh = getMeshType(*div_mesh);
					int col = dst_findex.x;
					int row = dst_findex.y;
					if(is_row && is_col) {
						mesh->divideCol(col, dst_findex.x - col);
						mesh->divideRow(row, dst_findex.y - row);
						div_mesh->interpolator->selectPoint(col+1, row+1);
					}
					else if(is_row) {
						mesh->divideCol(col, dst_findex.x - col);
						div_mesh->interpolator->selectPoint(col+1, row);
					}
					else if(is_col) {
						mesh->divideRow(row, dst_findex.y - row);
						div_mesh->interpolator->selectPoint(col, row+1);
					}
					div_mesh->setDirty();
					is_div_point_valid_ = false;
				}

				for(auto selection : op_selection_.point) {
					auto d = data.find(selection.first.lock());
					if(d.second) {
						auto mesh = d.second->mesh;
						for(auto index : selection.second) {
							if(isCorner(*mesh, index)) {
								continue;
							}
							d.second->interpolator->togglePoint(index.first, index.second);
							d.second->setDirty();
						}
					}
				}
				op_selection_.point.clear();
				op_hover_ = getHover(mouse_.pos, !is_enabled_hovering_uneditable_point_);
			}
		}
	}
}

void WarpingMeshEditor::drawControl(float parent_scale) const
{
	switch(mode_) {
		case MODE_MESH:
			MeshEditor::drawControl(parent_scale);
			return;
		case MODE_DIVISION: {
			drawWire();
			float point_size = mouse_near_distance_/parent_scale;
			float cross_size = point_size*4;
			float cross_width = point_size/2.f;
			ofMesh mesh;
			mesh.setMode(OF_PRIMITIVE_TRIANGLES);
			forEachMesh([&](std::shared_ptr<DataType> m) {
				forEachPoint(*m, [&](const PointType &point, IndexType i) {
					if(isCorner(*m->mesh, i)) {
						mesh.append(makeMeshFromPoint(point, ofColor::black, point_size/2.f));
					}
					else if(isEditablePoint(*m, i)) {
						mesh.append(makeMeshFromPoint(point, {ofColor::white, 128}, point_size));
						if(isHoveredPoint(*m, i)) {
							mesh.append(makeCross(point, ofColor::red, cross_size, cross_width, 45));
						}
					}
					else {
						mesh.append(makeMeshFromPoint(point, {ofColor::green, 128}, point_size/2.f));
						if(isHoveredPoint(*m, i)) {
							mesh.append(makeCross(point, ofColor::green, cross_size, cross_width, 0));
						}
					}
				});
			});
			if(is_div_point_valid_) {
				mesh.append(makeCross(div_point_, ofColor::green, cross_size, cross_width, 0));
			}
			mesh.draw();
		}	break;
	}
}


namespace {
template <typename ... Args>
std::string format(const std::string& fmt, Args ... args )
{
	size_t len = std::snprintf( nullptr, 0, fmt.c_str(), args ... );
	std::vector<char> buf(len + 1);
	std::snprintf(&buf[0], len + 1, fmt.c_str(), args ... );
	return std::string(&buf[0], &buf[0] + len);
}}
void WarpingMeshEditor::gui()
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
				for(auto &&mesh : data.getEditableData()) {
					if(mesh.second) {
						auto m = getMeshType(*mesh.second);
						meshes.push_back({mesh.first, m});
					}
				}
				EndTabItem();
			}
			if(BeginTabItem("all")) {
				for(auto &&mesh : data.getData()) {
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
			if(BeginTabItem("TexCoord")) {
				float v_min[2] = {0,0};
				float v_max[2] = {1,1};
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
				guiPoint = [=](GuiPoint p) {
					return DragFloatNAs(p.label, &p.point.t->x, 2, v_min, v_max, nullptr, nullptr, params, ImGuiSliderFlags_NoRoundToFormat);
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
			if(BeginTabItem("TexCoord")) {
				v_max = {1,1};
				func = [&](glm::vec2 delta) {
					moveSelectedCoord(delta);
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
