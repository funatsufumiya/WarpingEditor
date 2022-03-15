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
	auto cross = [lt,rb](glm::vec2 pos) {
		ofDrawLine(lt.x, pos.y, rb.x, pos.y);
		ofDrawLine(pos.x, lt.y, pos.x, rb.y);
	};
	ofPushStyle();
	ofSetColor(ofColor::white);
	cross(pos+glm::vec2{1,1});
	cross(pos-glm::vec2{1,1});
	ofSetColor(ofColor::black);
	cross(pos);
	if(mouse_.isDragged(OF_MOUSE_BUTTON_RIGHT)) {
		auto rect = mouse_.getDragRect();
		ofSetColor(ofColor::white, 64);
		ofDrawRectangle({getIn(rect.getTopLeft()), getIn(rect.getBottomRight())});
	}
	ofPopStyle();
}
