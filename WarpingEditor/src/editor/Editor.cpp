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
