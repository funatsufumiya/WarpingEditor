#pragma once

#include <memory>
#include "ofTexture.h"
#include "ofxEditorFrame.h"
#include "ResourceStorage.h"
#include "ofxBlendScreen.h"

namespace maaaaap {
class Source;
class WarpingMesh;
class RenderTexture;
class BlendingMesh;
class Output;

class TextureInspector
{
public:
	void Preview(ofTexture texture);
	void Info(ofTexture texture);
	void EditInfo(ofTexture texture);
};

class SourceInspector
{
public:
	void Info(std::shared_ptr<Source> source);
};

class RenderTextureInspector
{
public:
	void Info(std::shared_ptr<RenderTexture> source);
};

class WarpingMeshInspector
{
public:
	void setup();
	void EditUV(std::shared_ptr<WarpingMesh> mesh, ResourceStorage &storage);
	void EditMesh(std::shared_ptr<WarpingMesh> mesh, ResourceStorage &storage);
	void inactivateInteraction();
protected:
	ofxEditorFrame frame_;
	struct CallbackArg {
		CallbackArg(WarpingMeshInspector *self, std::shared_ptr<WarpingMesh> mesh, ResourceStorage &storage)
		:self(self),mesh(mesh),storage(storage) {}
		WarpingMeshInspector *self;
		std::shared_ptr<WarpingMesh> mesh;
		ResourceStorage &storage;
	};
	std::deque<CallbackArg> callback_arg_;
};

class BlendingMeshInspector
{
public:
	void setup();
	void EditUV(std::shared_ptr<BlendingMesh> mesh, ResourceStorage &storage);
	void EditMesh(std::shared_ptr<BlendingMesh> mesh, ResourceStorage &storage, ofxBlendScreen::Shader &shader);
	void inactivateInteraction();
protected:
	ofxEditorFrame frame_;
	struct CallbackArg {
		CallbackArg(BlendingMeshInspector *self, std::shared_ptr<BlendingMesh> mesh, ResourceStorage &storage, ofxBlendScreen::Shader &shader)
		:self(self),mesh(mesh),storage(storage),shader(shader) {}
		BlendingMeshInspector *self;
		std::shared_ptr<BlendingMesh> mesh;
		ResourceStorage &storage;
		ofxBlendScreen::Shader &shader;
	};
	std::deque<CallbackArg> callback_arg_;
};

class OutputInspector
{
public:
	void setup();
	void EditMesh(std::shared_ptr<Output> output, ResourceStorage &storage, ofxBlendScreen::Shader &shader);
	void inactivateInteraction();
protected:
	ofxEditorFrame frame_;
	struct CallbackArg {
		CallbackArg(OutputInspector *self, std::shared_ptr<Output> output, ResourceStorage &storage, ofxBlendScreen::Shader &shader)
		:self(self),output(output),storage(storage),shader(shader) {}
		OutputInspector *self;
		std::shared_ptr<Output> output;
		ResourceStorage &storage;
		ofxBlendScreen::Shader &shader;
	};
	std::deque<CallbackArg> callback_arg_;
};
}
