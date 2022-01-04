#pragma once

#include "ofxEditorFrame.h"
#include "Models.h"
#include "ofTexture.h"
#include "GuiFunc.h"

template<typename MeshType, typename IndexType, typename PointType=glm::vec2>
class Editor : public ofxEditorFrame
{
public:
	void setup();
	void update();
	void setTexture(ofTexture tex) { tex_ = tex; }
	void draw() const;
	
	void handleMouse(const ofxEditorFrame::MouseEventArg &arg) { mouse_.set(arg); }
private:
	ofTexture tex_;
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
	struct OpHover {
		std::weak_ptr<MeshType> quad;
		std::pair<std::weak_ptr<MeshType>, IndexType> point;
	} op_hover_;
	struct OpSelection {
		std::set<std::weak_ptr<MeshType>, std::owner_less<std::weak_ptr<MeshType>>> quad;
		std::map<std::weak_ptr<MeshType>, std::set<IndexType>, std::owner_less<std::weak_ptr<MeshType>>> point;
		bool is_grabbing = false;
	} op_selection_;
	bool isOpAdd() const;
	bool isOpAlt() const;
	bool isOpDefault() const;
	
	OpHover getHover(const glm::vec2 &screen_pos);
	OpSelection updateGrab(const OpSelection &selection, const OpHover &hover);
	void moveSelected(const glm::vec2 &delta);
	void moveMesh(MeshType &mesh, const glm::vec2 &delta);
	void movePoint(MeshType &mesh, IndexType index, const glm::vec2 &delta);
	
	std::shared_ptr<MeshType> getMeshType(const Data::Mesh &data) const;
	
	bool isHoveredMesh(const Data::Mesh &data) const;
	bool isHoveredPoint(const Data::Mesh &data, IndexType index) const;
	bool isSelectedMesh(const Data::Mesh &data) const;
	bool isSelectedPoint(const Data::Mesh &data, IndexType index) const;
	
	void forEachMesh(std::function<void(std::shared_ptr<Data::Mesh>)> func) const;
	void forEachPoint(const Data::Mesh &data, std::function<void(const PointType&, IndexType)> func) const;
	
	ofMesh makeMeshFromMesh(const Data::Mesh &mesh, const ofColor &color) const;
	ofMesh makeWireFromMesh(const Data::Mesh &mesh, const ofColor &color) const;
	ofMesh makeMeshFromPoint(const PointType &point, const ofColor &color, float point_size) const;
	ofMesh makeBackground() const { return ofMesh(); }

	std::pair<std::weak_ptr<MeshType>, IndexType> getNearestPoint(std::shared_ptr<Data::Mesh> data, const glm::vec2 &pos, float &distance2);
	std::shared_ptr<MeshType> getIfInside(std::shared_ptr<Data::Mesh> data, const glm::vec2 &pos, float &distance);
};

template<typename MeshType, typename IndexType, typename PointType>
inline bool Editor<MeshType, IndexType, PointType>::isOpAdd() const
{
	return ImGui::IsModKeyDown(ImGuiKeyModFlags_Shift);
}
template<typename MeshType, typename IndexType, typename PointType>
inline bool Editor<MeshType, IndexType, PointType>::isOpAlt() const
{
#ifdef TARGET_OSX
	return ImGui::IsModKeyDown(ImGuiKeyModFlags_Super);
#else
	 return ImGui::IsModKeyDown(ImGuiKeyModFlags_Ctrl);
#endif
}
template<typename MeshType, typename IndexType, typename PointType>
inline bool Editor<MeshType, IndexType, PointType>::isOpDefault() const
{
	return !(isOpAdd() || isOpAlt());
}

template<typename MeshType, typename IndexType, typename PointType>
inline void Editor<MeshType, IndexType, PointType>::setup()
{
	ofAddListener(on_mouse_event_, this, &Editor::handleMouse);
	ofxEditorFrame::setup();
}

template<typename MeshType, typename IndexType, typename PointType>
typename Editor<MeshType, IndexType, PointType>::OpSelection Editor<MeshType, IndexType, PointType>::updateGrab(const OpSelection &selection, const OpHover &hover)
{
	OpSelection ret = selection;
	bool is_grabbing = false;
	if(hover.point.first.expired()) {
		ret.point.clear();
	}
	else {
		is_grabbing = true;
		auto &point = ret.point[hover.point.first];
		auto result = point.insert(hover.point.second);
		bool is_new = result.second;
		if(!is_new) {
			if(isOpAlt()) {
				point.erase(result.first);
				is_grabbing = false;
			}
		}
		else if(isOpDefault()) {
			ret.point.clear();
			ret.point.insert(std::make_pair(hover.point.first, std::set<IndexType>{hover.point.second}));
		}
	}
	if(hover.quad.expired()) {
		ret.quad.clear();
	}
	else {
		is_grabbing = true;
		auto result = ret.quad.insert(hover.quad);
		bool is_new = result.second;
		if(!is_new) {
			if(isOpAlt()) {
				ret.quad.erase(result.first);
				is_grabbing = false;
			}
		}
		else if(isOpDefault()) {
			ret.quad = {hover.quad};
		}
	}
	ret.is_grabbing = is_grabbing;
	return ret;
}


template<typename MeshType, typename IndexType, typename PointType>
void Editor<MeshType, IndexType, PointType>::moveSelected(const glm::vec2 &delta)
{
	if(!op_selection_.point.empty()) {
		for(auto &&qp : op_selection_.point) {
			if(auto ptr = qp.first.lock()) {
				for(auto i : qp.second) {
					movePoint(*ptr, i, delta);
				}
			}
		}
	}
	if(!op_selection_.quad.empty()) {
		for(auto &&q : op_selection_.quad) {
			if(auto ptr = q.lock()) {
				moveMesh(*ptr, delta);
			}
		}
	}
}

template<typename MeshType, typename IndexType, typename PointType>
void Editor<MeshType, IndexType, PointType>::update()
{
	ofxEditorFrame::update();
	auto &&data = Data::shared();
	auto &&meshes = data.getMesh();
	mouse_.update();
	if(mouse_.isFrameNew()) {
		glm::vec2 tex_uv{tex_.getTextureData().tex_t, tex_.getTextureData().tex_u};
		auto proc = [&]() {
			if(mouse_.isReleased(OF_MOUSE_BUTTON_LEFT)) {
			}
			if(mouse_.isPressed(OF_MOUSE_BUTTON_LEFT)) {
				op_selection_ = updateGrab(op_selection_, op_hover_);
				op_hover_ = OpHover();
			}
			else if(!mouse_.isPressing(OF_MOUSE_BUTTON_LEFT)) {
				op_hover_ = getHover(mouse_.pos);
			}
			if(mouse_.isDragged(OF_MOUSE_BUTTON_LEFT)) {
				if(op_selection_.is_grabbing) {
					moveSelected(mouse_.delta);
				}
				else {
					translate(mouse_.delta);
				}
			}
			if(mouse_.isScrolledY()) {
				scale(pow(2, mouse_.scroll.y/10.f), mouse_.pos);
			}
		};
		proc();
	}
}

template<typename MeshType, typename IndexType, typename PointType>
void Editor<MeshType, IndexType, PointType>::draw() const
{
	auto &&tex = tex_;
	pushMatrix();
	pushScissor();
	{
		ofMesh mesh;
		mesh.setMode(OF_PRIMITIVE_TRIANGLES);
		mesh.append(makeBackground());
		forEachMesh([&](std::shared_ptr<Data::Mesh> m) {
			if(isSelectedMesh(*m)) {
				mesh.append(makeMeshFromMesh(*m, ofColor::white));
			}
			if(isHoveredMesh(*m)) {
				mesh.append(makeMeshFromMesh(*m, {ofColor::yellow, 128}));
			}
			mesh.append(makeMeshFromMesh(*m, {ofColor::gray, 128}));
		});
		tex.bind();
		mesh.draw();
		tex.unbind();
	}
	{
		ofMesh mesh;
		mesh.setMode(OF_PRIMITIVE_LINES);
		forEachMesh([&](std::shared_ptr<Data::Mesh> m) {
			mesh.append(makeWireFromMesh(*m, ofColor::white));
		});
		mesh.draw();
	}
	{
		float point_size = 10/getScale();
		ofMesh mesh;
		mesh.setMode(OF_PRIMITIVE_TRIANGLES);
		forEachMesh([&](std::shared_ptr<Data::Mesh> m) {
			forEachPoint(*m, [&](const PointType &point, IndexType i) {
				if(isSelectedPoint(*m, i)) {
					mesh.append(makeMeshFromPoint(point, ofColor::white, point_size));
				}
				if(isHoveredPoint(*m, i)) {
					mesh.append(makeMeshFromPoint(point, {ofColor::yellow, 128}, point_size));
				}
				mesh.append(makeMeshFromPoint(point, {ofColor::gray, 128}, point_size));
			});
		});
		mesh.draw();
	}
	popScissor();
	popMatrix();
}


template<typename MeshType, typename IndexType, typename PointType>
typename Editor<MeshType, IndexType, PointType>::OpHover Editor<MeshType, IndexType, PointType>::getHover(const glm::vec2 &screen_pos)
{
	auto &&data = Data::shared();
	auto &&meshes = data.getMesh();
	glm::vec2 tex_uv{tex_.getTextureData().tex_t, tex_.getTextureData().tex_u};
	OpHover ret;
	
	float max_distance = std::numeric_limits<float>::max();
	for(auto &&m : meshes) {
		const float threshold = pow(10/getScale(), 2);
		float distance;
		auto nearest = getNearestPoint(m.second, mouse_.pos, distance);
		if(max_distance > distance && distance < threshold) {
			ret.point = nearest;
			max_distance = distance;
		}
	}
	max_distance = std::numeric_limits<float>::max();
	if(ret.point.first.expired()) {
		for(auto &&m : meshes) {
			float distance;
			auto quad = getIfInside(m.second, mouse_.pos, distance);
			if(quad && max_distance > distance) {
				ret.quad = quad;
				max_distance = distance;
			}
		}
	}
	return ret;
}

template<typename MeshType, typename IndexType, typename PointType>
bool Editor<MeshType, IndexType, PointType>::isHoveredMesh(const Data::Mesh &data) const
{
	return getMeshType(data) == op_hover_.quad.lock();
}
template<typename MeshType, typename IndexType, typename PointType>
bool Editor<MeshType, IndexType, PointType>::isHoveredPoint(const Data::Mesh &data, IndexType index) const
{
	return getMeshType(data) == op_hover_.point.first.lock() && index == op_hover_.point.second;
}
template<typename MeshType, typename IndexType, typename PointType>
bool Editor<MeshType, IndexType, PointType>::isSelectedMesh(const Data::Mesh &data) const
{
	return any_of(begin(op_selection_.quad), end(op_selection_.quad), [&](std::weak_ptr<MeshType> ptr) {
		return getMeshType(data) == ptr.lock();
	});
}
template<typename MeshType, typename IndexType, typename PointType>
bool Editor<MeshType, IndexType, PointType>::isSelectedPoint(const Data::Mesh &data, IndexType index) const
{
	return any_of(begin(op_selection_.point), end(op_selection_.point), [&](std::pair<std::weak_ptr<MeshType>, std::set<IndexType>> ptr) {
		return getMeshType(data) == ptr.first.lock() && ptr.second.find(index) != ptr.second.end();
	});
}

template<typename MeshType, typename IndexType, typename PointType>
void Editor<MeshType, IndexType, PointType>::forEachMesh(std::function<void(std::shared_ptr<Data::Mesh>)> func) const
{
	auto &&data = Data::shared();
	auto &&meshes = data.getMesh();
	for(auto &&m : meshes) {
		func(m.second);
	}
}


template<typename MeshType, typename IndexType, typename PointType>
ofMesh Editor<MeshType, IndexType, PointType>::makeMeshFromPoint(const PointType &point, const ofColor &color, float point_size) const
{
	ofMesh ret;
	ret.setMode(OF_PRIMITIVE_TRIANGLES);
	const int resolution = 16;
	float angle = TWO_PI/(float)resolution;
	for(int i = 0; i < resolution; ++i) {
		ret.addVertex(glm::vec3(point+glm::vec2(cos(angle*i), sin(angle*i))*point_size,0));
		ret.addVertex(glm::vec3(point,0));
		ret.addVertex(glm::vec3(point+glm::vec2(cos(angle*(i+1)), sin(angle*(i+1)))*point_size,0));
		ret.addColor(color);
		ret.addColor(color);
		ret.addColor(color);
	}
	return ret;
}
