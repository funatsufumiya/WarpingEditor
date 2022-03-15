#include "Editor.h"

void EditorBase::setup()
{
	ofAddListener(on_mouse_event_, this, &EditorBase::handleMouse);
	ofxEditorFrame::setup();
}

void EditorBase::update()
{
	ofxEditorFrame::update();
	mouse_.update();
	if(mouse_.isFrameNew()) {
		procNewMouseEvent(mouse_);
	}
}

void EditorBase::drawCursor() const
{
	glm::vec2 lt{0,0};
	glm::vec2 rb = getWorkAreaSize();
	auto pos = getIn(mouse_.pos);
	
	ofMesh mesh;
	mesh.setMode(OF_PRIMITIVE_LINES);
	auto cross = [lt,rb](glm::vec2 pos, ofColor color, ofMesh &mesh) {
		auto offset = (ofIndexType)mesh.getNumVertices();
		mesh.addVertices({
			{pos.x, pos.y, 0},
			{lt.x, pos.y, 0}, {rb.x, pos.y, 0},
			{pos.x, lt.y, 0}, {pos.x, rb.y, 0}
		});
		auto transparent = ofColor(color, 32);
		mesh.addColors({color, transparent, transparent, transparent, transparent});
		mesh.addIndices({
			offset, offset+1,
			offset, offset+2,
			offset, offset+3,
			offset, offset+4,
		});
	};
	cross(pos+glm::vec2{1,1}, ofColor::white, mesh);
	cross(pos-glm::vec2{1,1}, ofColor::white, mesh);
	cross(pos, ofColor::black, mesh);
	mesh.draw();
	if(mouse_.isDragged(OF_MOUSE_BUTTON_RIGHT)) {
		ofPushStyle();
		auto rect = mouse_.getDragRect();
		ofSetColor(ofColor::white, 64);
		ofDrawRectangle({getIn(rect.getTopLeft()), getIn(rect.getBottomRight())});
		ofPopStyle();
	}
}
