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
	shared_ptr<ofAppBaseWindow> mainWindow = ofCreateWindow(settings);

	settings.setSize(1920,1080);
	settings.setPosition({100, 0});
	settings.resizable = true;
	settings.decorated = true;
	settings.multiMonitorFullScreen = false;
	settings.shareContextWith = mainWindow;
	shared_ptr<ofAppBaseWindow> editorWindow = ofCreateWindow(settings);

	shared_ptr<MainApp> mainApp(new MainApp);
	shared_ptr<GuiApp> guiApp(new GuiApp);
	guiApp->setMainApp(mainApp);
	
	ofRunApp(mainWindow, mainApp);
	ofRunApp(editorWindow, guiApp);
	ofRunMainLoop();
}
