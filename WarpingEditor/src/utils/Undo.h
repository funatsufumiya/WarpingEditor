#pragma once

#include "ofxUndoState.h"
#include <memory>

class GuiApp;

using UndoBuf = std::string;
class Undo;
class UndoDescriptor
{
public:
	UndoDescriptor(Undo &undo):undo_(undo){}
	uint32_t getUndoStateDescriptor();
private:
	Undo &undo_;
};
class Undo : public ofxUndoState<UndoBuf>
{
public:
	Undo():descriptor_(*this){}
	void setup(GuiApp *app);
	void enableAuto(float check_interval);
	void disableAuto();
	
	DataType create() const;
	DataType createUndo() const override;
	void loadUndo(const DataType &data) override;
	
	std::size_t getDataSize() const;
private:
	GuiApp *app_;
	mutable UndoBuf cache_;
	mutable UndoDescriptor descriptor_;
};
