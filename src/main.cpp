#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main( ){
    ofGLFWWindowSettings settings;

    settings.setSize(1440, 900);

    settings.setPosition(ofVec2f(100, 100));
    settings.resizable = true;
    shared_ptr<ofAppBaseWindow> mainWindow = ofCreateWindow(settings);
    
    ofxXmlSettings xml;

    xml.loadFile("paths.xml");
    
    xml.pushTag("root");
        xml.pushTag("config");
            xml.pushTag("projectorResolution");
                int projectionWidth  =xml.getValue("width", 1280);
                int projectionHeight =xml.getValue("height", 960);
                cout << projectionWidth << projectionHeight;
            xml.popTag();
        xml.popTag();
    xml.popTag();

    settings.setSize(projectionWidth, projectionHeight);
    settings.setPosition(ofVec2f(ofGetScreenWidth(), 0));
    settings.resizable = false;
    settings.decorated = false;
    settings.shareContextWith = mainWindow;
    shared_ptr<ofAppBaseWindow> secondWindow = ofCreateWindow(settings);
    secondWindow->setVerticalSync(false);
    
    shared_ptr<ofApp> mainApp(new ofApp);
    ofAddListener(secondWindow->events().draw, mainApp.get(), &ofApp::drawSecondWindow);
    
    ofRunApp(mainWindow, mainApp);
    ofRunMainLoop();
}
