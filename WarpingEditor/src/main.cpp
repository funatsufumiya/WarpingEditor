#include "ofMain.h"
#include "ofApp.h"
#include "ofAppGLFWWindow.h"

//========================================================================
int main( ){
	ofGLFWWindowSettings settings;

	settings.setSize(1920,1080);
	settings.setPosition({1920/2,0});
	settings.resizable = false;
	settings.decorated = false;
	settings.multiMonitorFullScreen = true;
	shared_ptr<ofAppBaseWindow> resultWindow = ofCreateWindow(settings);

	settings.setSize(1920,1080);
	settings.setPosition({100, 0});
	settings.resizable = true;
	settings.decorated = true;
	settings.multiMonitorFullScreen = false;
	settings.shareContextWith = resultWindow;
	shared_ptr<ofAppBaseWindow> warpingWindow = ofCreateWindow(settings);

	shared_ptr<ResultView> resultApp(new ResultView);
	shared_ptr<GuiApp> guiApp(new GuiApp);
	guiApp->setResultWindow(resultWindow);
	guiApp->setResultApp(resultApp);
	
	ofRunApp(resultWindow, resultApp);
	ofRunApp(warpingWindow, guiApp);
	ofRunMainLoop();
}
