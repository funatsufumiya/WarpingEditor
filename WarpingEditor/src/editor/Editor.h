#pragma once

#include "ofxEditorFrame.h"
#include "MeshData.h"
#include "ofTexture.h"
#include "GuiFunc.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "ofGraphics.h"
#include "of3dUtils.h"

class EditorBase : public ofxEditorFrame
{
public:
	void setup();
	virtual void update();
	virtual void draw() const{}
	virtual void drawMesh(bool use_control_color) const {
		beginShader();
		tex_.bind();
		getMesh(use_control_color).draw();
		tex_.unbind();
		endShader();
	}
	virtual void drawWorkArea(const ofFloatColor &color, bool fill) const {
		auto size = getWorkAreaSize();
		ofPushStyle();
		ofSetColor(color);
		fill ? ofFill() : ofNoFill();
		ofDrawRectangle(0,0,size.x,size.y);
		ofPopStyle();
	};

	virtual void beginShader() const {}
	virtual void endShader() const {}
	virtual ofMesh getMesh(bool use_control_color) const { return {}; }
	virtual void drawControl(float parent_scale) const {}
	virtual void drawBackground() const {
		ofPushStyle();
		ofSetColor(32);
		tex_.draw(0,0);
		ofPopStyle();
	}
	virtual void gui() {}

	void setTexture(ofTexture tex) { tex_ = tex; }
	ofTexture getTexture() const { return tex_; }
	glm::vec2 getTextureResolution() const { return {tex_.getWidth(), tex_.getHeight()}; }

	void handleMouse(const ofxEditorFrame::MouseEventArg &arg) { mouse_.set(arg); }
	void setEnableViewportEditByMouse(bool enable) { is_viewport_editable_by_mouse_ = enable; }
	void setEnabledRectSelection(bool enable) { is_enabled_rect_selection_ = enable; }
	void setEnableMeshEditByMouse(bool enable) { is_mesh_editable_by_mouse_ = enable; }
	void setEnableMoveMeshByMouse(bool enable) { is_mesh_movable_by_mouse_ = enable; }
	
	virtual bool isPreventMeshInterpolation() const { return false; }

	virtual void moveSelectedOnScreenScale(const glm::vec2 &delta){}

	struct GridData {
		bool is_show=true;
		bool enabled_snap=true;
		glm::vec2 offset={0,0}, size={1920,1080};
	};

	const GridData& getGridData() const { return grid_; }
	void setGridData(const GridData &data) { grid_ = data; }

protected:
	ofTexture tex_;
	GridData grid_;
	virtual glm::vec2 getWorkAreaSize() const { return {tex_.getWidth(), tex_.getHeight()}; }

	bool is_viewport_editable_by_mouse_=true;
	bool is_enabled_rect_selection_=true;
	bool is_mesh_editable_by_mouse_=true;
	bool is_mesh_movable_by_mouse_=true;

	class MouseEvent : public ofxEditorFrame::MouseEventArg {
	public:
		bool isFrameNew() const { return is_frame_new_; }
		void set(const ofxEditorFrame::MouseEventArg &src) {
			*(ofxEditorFrame::MouseEventArg*)(this) = src;
			has_new_frame_ = true;
		}
		void update() {
			is_frame_new_ = has_new_frame_;
			has_new_frame_ = false;
		}
	private:
		bool is_frame_new_=false, has_new_frame_=false;
	} mouse_;
	virtual void procNewMouseEvent(const MouseEvent &mouse){}
};

namespace detail {
	template<bool B, typename T, typename F>
	using conditional_t = typename std::conditional<B, T, F>::type;
}

template<typename Data, typename Mesh, typename Index, typename Point=glm::vec2>
class Editor : public EditorBase
{
public:
	using DataType = Data;
	using MeshType = Mesh;
	using IndexType = Index;
	using PointType = Point;
	using ContainerType
	= detail::conditional_t<std::is_same<DataType, WarpingMesh>::value, WarpingData
	, detail::conditional_t<std::is_same<DataType, BlendingMesh>::value, BlendingData
	, nullptr_t
	>>;
	
	void setMeshData(std::shared_ptr<ContainerType> data) { data_ = data; }
	virtual void draw() const override;
	virtual ofMesh getMesh(bool use_control_color) const override;
	virtual void drawControl(float parent_scale) const override;
	
	void setEnabledHoveringUneditablePoint(bool enable) { is_enabled_hovering_uneditable_point_ = enable; }
	void moveSelectedOnScreenScale(const glm::vec2 &delta) override { moveSelected(delta/getScale()); }

	virtual bool isSelectedMesh(const DataType &data) const;
	virtual bool selectMesh(const DataType &data, bool with_points);
	virtual bool deselectMesh(const DataType &data, bool with_points);
	virtual bool isSelectedPoint(const DataType &data, IndexType index) const;
	
	bool calcSnapToGrid(const PointType &point, const ofRectangle &grid, float snap_distance, glm::vec2 &diff) const;
	virtual PointType getPoint(const MeshType &mesh, const IndexType &index) const = 0;

protected:
	std::shared_ptr<ContainerType> data_;
	void procNewMouseEvent(const MouseEvent &mouse) override;

	bool is_enabled_hovering_uneditable_point_=false;
	float mouse_near_distance_ = 10;
	struct OpHover {
		std::weak_ptr<MeshType> mesh;
		std::pair<std::weak_ptr<MeshType>, IndexType> point;
		bool isEmpty() const {
			return !mesh.lock() && !point.first.lock();
		}
	} op_hover_;
	struct OpRect {
		std::map<std::weak_ptr<MeshType>, std::set<IndexType>, std::owner_less<std::weak_ptr<MeshType>>> point;
	} op_rect_;
	struct OpSelection {
		std::set<std::weak_ptr<MeshType>, std::owner_less<std::weak_ptr<MeshType>>> mesh;
		std::map<std::weak_ptr<MeshType>, std::set<IndexType>, std::owner_less<std::weak_ptr<MeshType>>> point;
		bool contains(std::weak_ptr<MeshType> m) const {
			return mesh.find(m) != end(mesh);
		}
		bool contains(std::pair<std::weak_ptr<MeshType>, IndexType> p) const {
			auto found = point.find(p.first);
			if(found == end(point)) return false;
			auto indices = found->second;
			return find(begin(indices), end(indices), p.second) != end(indices);
		}
		bool addMesh(std::shared_ptr<MeshType> m) {
			return mesh.insert(m).second;
		}
		bool removeMesh(std::shared_ptr<MeshType> m) {
			return mesh.erase(m) > 0;
		}
		bool addPoints(std::shared_ptr<MeshType> m, std::set<IndexType> indices) {
			return point.insert({m, indices}).second;
		}
		bool removePoints(std::shared_ptr<MeshType> m, std::set<IndexType> indices) {
			auto found = point.find(m);
			if(found == end(point)) return false;
			int success_count = 0;
			for(auto &&i : indices) {
				success_count += found->second.erase(i);
			}
			return success_count > 0;
		}
	} op_selection_, op_selection_pressed_;
	
	bool is_grabbing_by_mouse_=false;
	bool isOpAdd() const;
	bool isOpAlt() const;
	bool isOpDefault() const;
	
	glm::vec2 snap_diff_={0,0};
	
	void drawWire() const;
	void drawPoint(bool only_editable_point, float parent_scale) const;
	void drawDragRect() const;
	void drawGrid() const;
	
	OpHover getHover(const glm::vec2 &screen_pos, bool only_editable_point);
	OpRect getRectHover(const ofRectangle &screen_rect, bool only_editable_point);
	OpSelection updateSelection(const OpSelection &selection, const OpHover &hover, bool for_grabbing);
	OpSelection updateSelection(const OpSelection &selection, const OpRect &rect);
	void moveSelected(const glm::vec2 &delta);
	virtual void moveMesh(MeshType &mesh, const glm::vec2 &delta) {}
	virtual void movePoint(MeshType &mesh, IndexType index, const glm::vec2 &delta){}
	void moveMeshOnScreenScale(MeshType &mesh, const glm::vec2 &delta) { moveMesh(mesh, delta/getScale()); }
	void movePointOnScreenScale(MeshType &mesh, IndexType index, const glm::vec2 &delta) { movePoint(mesh, delta/getScale()); }
	
	virtual std::set<IndexType> getIndices(std::shared_ptr<MeshType> mesh) const { return {}; }
	
	virtual std::shared_ptr<MeshType> getMeshType(const DataType &data) const { return nullptr; }
	
	virtual bool isEditablePoint(const DataType &data, IndexType index) const { return true; }
	virtual bool isHoveredMesh(const DataType &data) const;
	virtual bool isHoveredPoint(const DataType &data, IndexType index) const;
	virtual bool isRectHoveredPoint(const DataType &data, IndexType index) const;
	
	virtual void forEachMesh(std::function<void(std::shared_ptr<DataType>)> func) const;
	virtual void forEachPoint(const DataType &data, std::function<void(const PointType&, IndexType)> func) const {}
	
	virtual ofMesh makeMeshFromMesh(const DataType &mesh, const ofColor &color) const { return ofMesh(); }
	virtual ofMesh makeWireFromMesh(const DataType &mesh, const ofColor &color) const { return ofMesh(); }
	ofMesh makeMeshFromPoint(const PointType &point, const ofColor &color, float point_size) const;

	virtual std::pair<std::weak_ptr<MeshType>, IndexType> getNearestPoint(std::shared_ptr<DataType> data, const glm::vec2 &pos, float &distance2, bool filter_by_if_editable=true);
	virtual std::shared_ptr<MeshType> getIfInside(std::shared_ptr<DataType> data, const glm::vec2 &pos, float &distance) { return nullptr; }
	virtual std::map<std::weak_ptr<MeshType>, std::set<IndexType>, std::owner_less<std::weak_ptr<MeshType>>> getPointInsideRect(std::shared_ptr<DataType> data, const ofRectangle &rect, bool filter_by_if_editable=true);
	
	std::pair<bool, glm::vec2> gui2DPanel(const std::string &label, const float v_min[2], const float v_max[2], const std::vector<std::pair<std::string, std::vector<ImGui::DragScalarAsParam>>> &params) const;
};

template<typename Data, typename Mesh, typename Index, typename Point>
inline bool Editor<Data, Mesh, Index, Point>::isOpAdd() const
{
	return ImGui::IsModKeyDown(ImGuiKeyModFlags_Shift);
}
template<typename Data, typename Mesh, typename Index, typename Point>
inline bool Editor<Data, Mesh, Index, Point>::isOpAlt() const
{
#ifdef TARGET_OSX
	return ImGui::IsModKeyDown(ImGuiKeyModFlags_Super);
#else
	return ImGui::IsModKeyDown(ImGuiKeyModFlags_Ctrl);
#endif
}
template<typename Data, typename Mesh, typename Index, typename Point>
inline bool Editor<Data, Mesh, Index, Point>::isOpDefault() const
{
	return !(isOpAdd() || isOpAlt());
}


template<typename Data, typename Mesh, typename Index, typename Point>
inline typename Editor<Data, Mesh, Index, Point>::OpSelection Editor<Data, Mesh, Index, Point>::updateSelection(const OpSelection &selection, const OpHover &hover, bool for_grabbing)
{
	OpSelection ret = selection;
	if(isOpDefault()) {
		if(!(selection.contains(hover.mesh) || selection.contains(hover.point)) || !for_grabbing) {
			ret.mesh.clear();
			ret.point.clear();
		}
	}
	
	if(hover.point.first.lock()) {
		auto &point = ret.point[hover.point.first];
		auto result = point.insert(hover.point.second);
		bool is_new = result.second;
		if(!is_new) {
			if(isOpAlt()) {
				point.erase(result.first);
			}
		}
		else if(isOpDefault()) {
			ret.point.clear();
			ret.point.insert(std::make_pair(hover.point.first, std::set<IndexType>{hover.point.second}));
		}
	}
	if(hover.mesh.lock()) {
		auto result = ret.mesh.insert(hover.mesh);
		bool is_new = result.second;
		if(!is_new) {
			if(isOpAlt()) {
				ret.mesh.erase(result.first);
			}
		}
		else if(isOpDefault()) {
			ret.mesh = {hover.mesh};
		}
	}

	return ret;
}

template<typename Data, typename Mesh, typename Index, typename Point>
inline typename Editor<Data, Mesh, Index, Point>::OpSelection Editor<Data, Mesh, Index, Point>::updateSelection(const OpSelection &selection, const OpRect &rect)
{
	OpSelection ret = selection;
	if(isOpDefault()) {
		ret.mesh.clear();
		ret.point.clear();
	}
	for(auto &&p : rect.point) {
		auto &point = ret.point[p.first];
		for(auto &&i : p.second) {
			auto result = point.insert(i);
			bool is_new = result.second;
			if(!is_new) {
				if(isOpAlt()) {
					point.erase(result.first);
				}
			}
			else if(isOpDefault()) {
				ret.point[p.first].insert(i);
			}
		}
	}
	return ret;
}

template<typename Data, typename Mesh, typename Index, typename Point>
inline void Editor<Data, Mesh, Index, Point>::moveSelected(const glm::vec2 &delta)
{
	auto &&data = *data_;
	auto points = op_selection_.point;
	if(!op_selection_.mesh.empty()) {
		for(auto &&q : op_selection_.mesh) {
			if(auto ptr = q.lock()) {
				moveMesh(*ptr, delta);
				data.find(ptr).second->setDirty();
			}
			auto points_of_mesh = points.find(q);
			if(points_of_mesh != end(points)) {
				points.erase(points_of_mesh);
			}
		}
	}
	if(!points.empty()) {
		for(auto &&qp : points) {
			if(auto ptr = qp.first.lock()) {
				for(auto i : qp.second) {
					movePoint(*ptr, i, delta);
				}
				data.find(ptr).second->setDirty();
			}
		}
	}
}

template<typename Data, typename Mesh, typename Index, typename Point>
inline bool Editor<Data, Mesh, Index, Point>::calcSnapToGrid(const PointType &point, const ofRectangle &grid, float snap_distance, glm::vec2 &diff) const
{
	PointType p;
	p.x = ofWrap(point.x, grid.getLeft(), grid.getRight());
	p.y = ofWrap(point.y, grid.getTop(), grid.getBottom());
	auto nearSide = [](float x, float a, float b) {
		float da = a-x, db = b-x;
		return std::abs(da) < std::abs(db) ? da : db;
	};
	diff.x = nearSide(p.x, grid.getLeft(), grid.getRight());
	diff.y = nearSide(p.y, grid.getTop(), grid.getBottom());
	if(std::abs(diff.x) > snap_distance) diff.x = 0;
	if(std::abs(diff.y) > snap_distance) diff.y = 0;
	return diff.x != 0 || diff.y != 0;
}

template<typename Data, typename Mesh, typename Index, typename Point>
inline void Editor<Data, Mesh, Index, Point>::procNewMouseEvent(const MouseEvent &mouse)
{
	bool used = false;
	if(!used && is_mesh_editable_by_mouse_) {
		if(mouse.isDragged(OF_MOUSE_BUTTON_LEFT)) {
			if(is_grabbing_by_mouse_ && is_mesh_movable_by_mouse_) {
				moveSelected(mouse.delta/getScale()-snap_diff_);
				snap_diff_ = {0,0};
				if(grid_.enabled_snap && op_selection_.contains(op_hover_.point)) {
					glm::vec2 snap_diff;
					if(calcSnapToGrid(getPoint(*op_hover_.point.first.lock(), op_hover_.point.second), {grid_.offset, grid_.offset+grid_.size}, mouse_near_distance_/getScale(), snap_diff)) {
						moveSelected(snap_diff);
						snap_diff_ = snap_diff;
					}
				}
				used = true;
			}
		}
		if(mouse.isPressed(OF_MOUSE_BUTTON_LEFT)) {
			if(op_hover_.isEmpty()) {
				op_selection_pressed_ = OpSelection();
			}
			else {
				op_selection_pressed_ = updateSelection(op_selection_, op_hover_, false);
				op_selection_ = updateSelection(op_selection_, op_hover_, true);
				is_grabbing_by_mouse_ = op_selection_.contains(op_hover_.mesh) || op_selection_.contains(op_hover_.point);
				used = true;
			}
		}
		if(mouse.isClicked(OF_MOUSE_BUTTON_LEFT)) {
			op_selection_ = op_selection_pressed_;
			op_hover_ = OpHover();
			used = true;
		}
		if(mouse.isReleased(OF_MOUSE_BUTTON_LEFT)) {
			is_grabbing_by_mouse_ = false;
			snap_diff_ = {0,0};
		}
		if(is_enabled_rect_selection_) {
			if(mouse.isPressing(OF_MOUSE_BUTTON_RIGHT)) {
				op_rect_ = getRectHover(mouse.getDragRect(), !is_enabled_hovering_uneditable_point_);
				used = true;
			}
			else if(mouse.isReleased(OF_MOUSE_BUTTON_RIGHT)) {
				op_selection_ = updateSelection(op_selection_, op_rect_);
				op_rect_ = OpRect();
				used = true;
			}
		}
		if(!mouse.isPressingAny()) {
			op_hover_ = getHover(mouse.pos, !is_enabled_hovering_uneditable_point_);
		}
	} 
	if(!used && is_viewport_editable_by_mouse_) {
		if(mouse.isDragged(OF_MOUSE_BUTTON_LEFT)) {
			translate(mouse.delta);
			used = true;
		}
		if(mouse.isScrolledY()) {
			scale(pow(2, mouse.scroll.y/10.f), mouse.pos);
			used = true;
		}
	}
}
template<typename Data, typename Mesh, typename Index, typename Point>
ofMesh Editor<Data, Mesh, Index, Point>::getMesh(bool use_control_color) const
{
	ofMesh mesh;
	mesh.setMode(OF_PRIMITIVE_TRIANGLES);
	auto meshes = data_->getVisibleData();
	for(auto &&mm : meshes) {
		auto m = mm.second;
		if(use_control_color) {
			if(isSelectedMesh(*m)) {
				mesh.append(makeMeshFromMesh(*m, ofColor::white));
			}
			else if(isHoveredMesh(*m)) {
				mesh.append(makeMeshFromMesh(*m, {ofColor::yellow, 128}));
			}
			else {
				mesh.append(makeMeshFromMesh(*m, {ofColor::gray, 128}));
			}
		}
		else {
			mesh.append(makeMeshFromMesh(*m, ofColor::white));
		}
	}
	return mesh;
}
template<typename Data, typename Mesh, typename Index, typename Point>
void Editor<Data, Mesh, Index, Point>::drawWire() const
{
	ofMesh mesh;
	mesh.setMode(OF_PRIMITIVE_LINES);

	auto meshes = data_->getVisibleData();
	for(auto &&mm : meshes) {
		auto m = mm.second;
		mesh.append(makeWireFromMesh(*m, ofColor::white));
	}
	mesh.draw();
}
template<typename Data, typename Mesh, typename Index, typename Point>
void Editor<Data, Mesh, Index, Point>::drawPoint(bool only_editable_point, float parent_scale) const
{
	float point_size = mouse_near_distance_/parent_scale;
	ofMesh mesh;
	mesh.setMode(OF_PRIMITIVE_TRIANGLES);
	auto meshes = data_->getVisibleData();
	for(auto &&mm : meshes) {
		auto m = mm.second;
		forEachPoint(*m, [&](const PointType &point, IndexType i) {
			if(only_editable_point && !isEditablePoint(*m, i)) {
				return;
			}
			if(isSelectedPoint(*m, i)) {
				mesh.append(makeMeshFromPoint(point, ofColor::white, point_size));
			}
			if(isHoveredPoint(*m, i) || isRectHoveredPoint(*m, i)) {
				mesh.append(makeMeshFromPoint(point, {ofColor::yellow, 128}, point_size));
			}
			mesh.append(makeMeshFromPoint(point, {ofColor::gray, 128}, point_size));
		});
	};
	mesh.draw();
}
template<typename Data, typename Mesh, typename Index, typename Point>
void Editor<Data, Mesh, Index, Point>::drawDragRect() const
{
	if(mouse_.isPressing(OF_MOUSE_BUTTON_RIGHT)) {
		auto rect = mouse_.getDragRect();
		if(rect.isEmpty()) {
			return;
		}
		ofPushStyle();
		ofSetColor({ofColor::white, 128});
		ofDrawRectangle(rect);
		ofPopStyle();
	}
}
template<typename Data, typename Mesh, typename Index, typename Point>
void Editor<Data, Mesh, Index, Point>::drawGrid() const
{
	auto offset = grid_.offset;
	auto size = grid_.size;
	glm::vec2 region_lt = region_.getTopLeft(), region_rb = region_.getBottomRight();
	region_lt = getIn(region_lt);
	region_rb = getIn(region_rb);
	auto calcRange = [](float offset, float width, float l, float r) {
		r += width-ofWrap(r-l, 0, width);
		offset = ofWrap(offset, l, r);
		return glm::vec2{
			offset - width*ceil((offset-l)/width),
			offset + width*ceil((r-offset)/width)
		};
	};
	auto lr = calcRange(offset.x, size.x, region_lt.x, region_rb.x);
	auto tb = calcRange(offset.y, size.y, region_lt.y, region_rb.y);
	ofMesh m;
	m.setMode(OF_PRIMITIVE_LINES);
	for(auto x = lr[0]; x <= lr[1]; x += size.x) {
		m.addVertices({{x, tb[0], 0}, {x, tb[1], 0}});
	}
	for(auto y = tb[0]; y <= tb[1]; y += size.y) {
		m.addVertices({{lr[0], y, 0}, {lr[1], y, 0}});
	}
	m.drawWireframe();

	ofPushStyle();
	ofSetDrawBitmapMode(OF_BITMAPMODE_MODEL_BILLBOARD);
	for(auto x = lr[0]; x <= lr[1]; x += size.x) {
		ofDrawBitmapString(ofToString(x), x, region_rb[1]);
	}
	for(auto y = tb[0]; y <= tb[1]; y += size.y) {
		ofDrawBitmapString(ofToString(y), region_lt[0], y);
	}
	ofPopStyle();
}

template<typename Data, typename Mesh, typename Index, typename Point>
void Editor<Data, Mesh, Index, Point>::draw() const
{
	pushScissor();
	pushMatrix();
	drawBackground();
	if(grid_.is_show) {
		drawGrid();
	}
	drawMesh(true);
	drawControl(getScale());
	drawWorkArea(ofColor::red, false);
	popMatrix();
	if(is_enabled_rect_selection_) {
		drawDragRect();
	}
	popScissor();
}

template<typename Data, typename Mesh, typename Index, typename Point>
void Editor<Data, Mesh, Index, Point>::drawControl(float parent_scale) const
{
	drawWire();
	drawPoint(!is_enabled_hovering_uneditable_point_, parent_scale);
}

template<typename Data, typename Mesh, typename Index, typename Point>
std::pair<std::weak_ptr<Mesh>, Index> Editor<Data, Mesh, Index, Point>::getNearestPoint(std::shared_ptr<DataType> data, const glm::vec2 &pos, float &distance2, bool filter_by_if_editable)
{
	std::pair<std::weak_ptr<MeshType>, IndexType> ret;
	distance2 = std::numeric_limits<float>::max();
	auto p = getIn(pos);
	forEachPoint(*data, [&](const PointType &point, IndexType index) {
		if(filter_by_if_editable && !isEditablePoint(*data, index)) {
			return;
		}
		float d2 = glm::distance2(point, p);
		if(d2 < distance2) {
			decltype(ret) tmp{getMeshType(*data), index};
			swap(ret, tmp);
			distance2 = d2;
		}
	});
	return ret;
}

template<typename Data, typename Mesh, typename Index, typename Point>
std::map<std::weak_ptr<Mesh>, std::set<Index>, std::owner_less<std::weak_ptr<Mesh>>> Editor<Data, Mesh, Index, Point>::getPointInsideRect(std::shared_ptr<DataType> data, const ofRectangle &rect, bool filter_by_if_editable)
{
	std::map<std::weak_ptr<MeshType>, std::set<IndexType>, std::owner_less<std::weak_ptr<MeshType>>> ret;
	forEachPoint(*data, [&](const PointType &point, IndexType index) {
		if(filter_by_if_editable && !isEditablePoint(*data, index)) {
			return;
		}
		if(rect.inside(getOut(point))) {
			ret[getMeshType(*data)].insert(index);
		}
	});
	return ret;
}

template<typename Data, typename Mesh, typename Index, typename Point>
typename Editor<Data, Mesh, Index, Point>::OpHover Editor<Data, Mesh, Index, Point>::getHover(const glm::vec2 &screen_pos, bool only_editable_point)
{
	auto &&data = *data_;
	auto &&meshes = data.getData();

	OpHover ret;
	float max_distance = std::numeric_limits<float>::max();
	for(auto &&m : meshes) {
		if(!data.isEditable(m.second)) {
			continue;
		}
		const float threshold = pow(mouse_near_distance_/getScale(), 2);
		float distance;
		auto nearest = getNearestPoint(m.second, mouse_.pos, distance, only_editable_point);
		if(max_distance >= distance && distance < threshold) {
			ret.point = nearest;
			max_distance = distance;
		}
	}
	max_distance = std::numeric_limits<float>::max();
	if(ret.point.first.expired()) {
		for(auto &&m : meshes) {
			if(!data.isEditable(m.second)) {
				continue;
			}
			float distance;
			auto mesh = getIfInside(m.second, mouse_.pos, distance);
			if(mesh && max_distance >= distance) {
				ret.mesh = mesh;
				max_distance = distance;
			}
		}
	}
	return ret;
}

template<typename Data, typename Mesh, typename Index, typename Point>
typename Editor<Data, Mesh, Index, Point>::OpRect Editor<Data, Mesh, Index, Point>::getRectHover(const ofRectangle &screen_rect, bool only_editable_point)
{
	auto &&data = *data_;
	auto &&meshes = data.getData();
	OpRect ret;
	for(auto &&m : meshes) {
		if(!data.isEditable(m.second)) {
			continue;
		}
		auto tmp = getPointInsideRect(m.second, screen_rect, only_editable_point);
		ret.point.insert(begin(tmp), end(tmp));
	}
	return ret;
}

template<typename Data, typename Mesh, typename Index, typename Point>
bool Editor<Data, Mesh, Index, Point>::isHoveredMesh(const DataType &data) const
{
	return getMeshType(data) == op_hover_.mesh.lock();
}
template<typename Data, typename Mesh, typename Index, typename Point>
bool Editor<Data, Mesh, Index, Point>::isHoveredPoint(const DataType &data, IndexType index) const
{
	return getMeshType(data) == op_hover_.point.first.lock() && index == op_hover_.point.second;
}
template<typename Data, typename Mesh, typename Index, typename Point>
bool Editor<Data, Mesh, Index, Point>::isRectHoveredPoint(const DataType &data, IndexType index) const
{
	auto mesh = getMeshType(data);
	auto mesh_found = op_rect_.point.find(mesh);
	if(mesh_found == end(op_rect_.point)) {
		return false;
	}
	auto &&indices = mesh_found->second;
	return find(begin(indices), end(indices), index) != end(indices);
}
template<typename Data, typename Mesh, typename Index, typename Point>
bool Editor<Data, Mesh, Index, Point>::isSelectedMesh(const DataType &data) const
{
	return any_of(begin(op_selection_.mesh), end(op_selection_.mesh), [&](std::weak_ptr<MeshType> ptr) {
		return getMeshType(data) == ptr.lock();
	});
}
template<typename Data, typename Mesh, typename Index, typename Point>
bool Editor<Data, Mesh, Index, Point>::isSelectedPoint(const DataType &data, IndexType index) const
{
	return any_of(begin(op_selection_.point), end(op_selection_.point), [&](std::pair<std::weak_ptr<MeshType>, std::set<IndexType>> ptr) {
		return getMeshType(data) == ptr.first.lock() && ptr.second.find(index) != ptr.second.end();
	});
}

template<typename Data, typename Mesh, typename Index, typename Point>
bool Editor<Data, Mesh, Index, Point>::selectMesh(const DataType &data, bool with_points)
{
	bool ret = false;
	std::shared_ptr<MeshType> ptr = getMeshType(data);
	ret |= op_selection_.addMesh(ptr);
	if(with_points) {
		ret |= op_selection_.addPoints(ptr, getIndices(ptr));
	}
	return ret;
}
template<typename Data, typename Mesh, typename Index, typename Point>
bool Editor<Data, Mesh, Index, Point>::deselectMesh(const DataType &data, bool with_points)
{
	bool ret = false;
	std::shared_ptr<MeshType> ptr = getMeshType(data);
	ret |= op_selection_.removeMesh(ptr);
	if(with_points) {
		ret |= op_selection_.removePoints(ptr, getIndices(ptr));
	}
	return ret;
}

template<typename Data, typename Mesh, typename Index, typename Point>
void Editor<Data, Mesh, Index, Point>::forEachMesh(std::function<void(std::shared_ptr<DataType>)> func) const
{
	auto &&data = *data_;
	auto &&meshes = data.getData();
	for(auto &&m : meshes) {
		func(m.second);
	}
}


template<typename Data, typename Mesh, typename Index, typename Point>
ofMesh Editor<Data, Mesh, Index, Point>::makeMeshFromPoint(const PointType &point, const ofColor &color, float point_size) const
{
	ofMesh ret;
	ret.setMode(OF_PRIMITIVE_TRIANGLES);
	const int resolution = 16;
	float angle = TWO_PI/(float)resolution;
	for(int i = 0; i < resolution; ++i) {
		ret.addIndex((ofIndexType)ret.getNumVertices());
		ret.addVertex(glm::vec3(point+glm::vec2(cos(angle*i), sin(angle*i))*point_size,0));
		ret.addIndex((ofIndexType)ret.getNumVertices());
		ret.addVertex(glm::vec3(point,0));
		ret.addIndex((ofIndexType)ret.getNumVertices());
		ret.addVertex(glm::vec3(point+glm::vec2(cos(angle*(i+1)), sin(angle*(i+1)))*point_size,0));
		ret.addColor(color);
		ret.addColor(color);
		ret.addColor(color);
	}
	return ret;
}

template<typename Data, typename Mesh, typename Index, typename Point>
std::pair<bool, glm::vec2> Editor<Data, Mesh, Index, Point>::gui2DPanel(const std::string &label_str, const float v_min[2], const float v_max[2], const std::vector<std::pair<std::string, std::vector<ImGui::DragScalarAsParam>>> &params) const
{
	using namespace ImGui;
	glm::vec2 diff{0,0};
	bool edited = false;
	
	const char *label = label_str.c_str();
	static ImVec2 diff_drag{0,0};
	auto diff_prev = diff_drag;
	if(DragFloatNAs(label, &diff_drag.x, 2, v_min, v_max, nullptr, nullptr, params, ImGuiSliderFlags_NoRoundToFormat)) {
		diff += diff_drag - diff_prev;
		edited |= true;
	}
	if(!IsItemActive()) {
		diff_drag = {0,0};
	}
	
	// getting data_type declared in DragFloatNAs 
	PushID(label);
	auto storage = ImGui::GetStateStorage();
	ImGuiID type_index_id = ImGui::GetCurrentWindow()->GetID("data_type");
	PopID();
	int type_index = storage->GetInt(type_index_id, 0);
	if(type_index >= params.size()) {
		type_index = 0;
	}
	auto p = params[type_index].second;
	assert(!p.empty());
	if(p.size() < 2) {
		p.push_back(p[0]);
	}
	for(int i = 0; i < 2; ++i) {
		ofStringReplace(p[i].format, "%d", "%.0f");
	}

	static ImVec2 diff_button{0,0};
	diff_prev = diff_button;
	if(Drag2DButton(label_str+"_dragbutton", diff_button, {p[0].speed, p[1].speed}, {p[0].format, p[1].format})) {
		for(int i = 0; i < 2; ++i) {
			diff[i] += ofMap(diff_button[i] - diff_prev[i], p[i].getMin<float>(), p[i].getMax<float>(), v_min[i], v_max[i]);
		}
		edited |= true;
	}
	if(!IsItemActive()) {
		diff_button = {0,0};
	}
	return {edited, diff};
}
