#include "ofApp.h"
/*
                    .   ?
                              ._/

                        .
    x                           ,
 
       '                                 "      */
void ofApp::setup() {
    /*        ___
     :'["_'                              *.
     '                           -_____
     
     O--___________
     
     .*
     */
    mainOutputSyphonServer.setName("Screen Output");
    
    fontSize = 15;
    
    font.load("Arial Unicode.ttf", fontSize, true, true);
    
    for(int i =0; i < 4; i++){
        blob b;
        blobs.push_back(b);
    }
    
    loadXml("paths.xml");
    
    ofSetVerticalSync(true);
    ofSetWindowShape(projectionWidth, projectionHeight);
    
    blobColors[0] = ofColor(255, 0, 0);
    blobColors[1] = ofColor(0, 255, 0);
    blobColors[2] = ofColor(0, 0, 255);
    blobColors[3] = ofColor(255, 255, 0);
    
    // set up kinect
    kinect.setRegistration(true);
    kinect.init();
    kinect.open();
    
    cout << "width" << ofGetWidth() << endl;
    
    grayImage.allocate(kinect.width, kinect.height);
    grayThreshNear.allocate(kinect.width, kinect.height);
    grayThreshFar.allocate(kinect.width, kinect.height);
    
    kpt.loadCalibration("calibration.xml");
    // kpt.setContourSmoothness(4);
    
    // setup gui
    gui.setup("parameters");
    gui.add(nearThreshold.set("nearThresh", 230, 0, 255));
    gui.add(farThreshold.set("farThresh", 10, 0, 255));
    gui.add(minArea.set("minArea", 1000, 0, 5000));
    gui.add(maxArea.set("maxArea", 70000, 15000, 150000));
    gui.add(threshold.set("threshold", 15, 1, 100));
    gui.add(persistence.set("persistence", 15, 1, 100));
    gui.add(maxDistance.set("maxDistance", 32, 1, 100));
    
    
    gui.add(step.set("step", 5, 1, 20));
    
    gui.add(opacity.set("opacity", 255, 0, 255));
    
    gui.add(displayText.set("displayText", true));
    gui.add(displayTexture.set("displayTexture", true));
    gui.add(drawBg.set("drawBg", true));
    
    //set up fbo
    fbo.allocate(projectionWidth, projectionHeight, GL_RGB);
    
    cout << "kinect " << kinect.width << " " << kinect.height << endl;
    cout << "prop " << projectionWidth / kinect.width << endl;
    
    numCountorFinder = 20;
    for(int i = 0; i < numCountorFinder; i++){
        ofxCv::ContourFinder cf;
        contourFinders.push_back(cf);
    }
    
    fbo.begin();
    ofClear(255,255,255, 0);
    fbo.end();
    
    noProjector = true;
    ;
}

void ofApp::loadXml(string xmlPath){
    
    xml.loadFile(xmlPath);
    
    xml.pushTag("root");
    
    xml.pushTag("config");
    settingsPath =xml.getValue("settingsPath", "");
    
    xml.pushTag("projectorResolution");
    projectionWidth  =xml.getValue("width", 1280);
    ofLog(projectionWidth);
    projectionHeight =xml.getValue("height", 960);
    ofLog(projectionHeight);
    xml.popTag();
    
    xml.popTag();
    
    int total = xml.getNumTags("path");
    
    ofLog() << "loading itens! total = " << total;
    // go through each stage
    for (int i = 0; i < total; i++) {
        xml.pushTag("path", i);
        string url = xml.getValue("url", "");
        paths.push_back(url);
        xml.popTag();
    }
    xml.popTag();
    
}

void ofApp::update() {
    kinect.update(); // - // - /231
    // --- // -- // -- // -- // -- // -- //
    if(kinect.isFrameNew()) {  //                           fazer comentários no código, deixando-o
        for(int c = 0; c < numCountorFinder; c++){ //       trazer seres para baixo, num mapa ordinário
            // process kinect depth image//                 podemos dizer que sim
            grayImage.setFromPixels(kinect.getDepthPixels().getData(), kinect.width, kinect.height);
            grayThreshNear = grayImage;
            grayThreshFar = grayImage;
            grayThreshNear.threshold(nearThreshold  - c*step, true);
            grayThreshFar.threshold(farThreshold);
            cvAnd(grayThreshNear.getCvImage(), grayThreshFar.getCvImage(), grayImage.getCvImage(), NULL);
            grayImage.flagImageChanged();
            
            // set contour tracker parameters
            contourFinders[c].setMinArea(minArea);
            contourFinders[c].setMaxArea(maxArea);
            contourFinders[c].setThreshold(threshold);
            contourFinders[c].getTracker().setPersistence(persistence);
            contourFinders[c].getTracker().setMaximumDistance(maxDistance);
            
            // determine found contours
            contourFinders[c].findContours(grayImage);
        }
    }
    
    fbo.readToPixels(pixels);
    
    int w = projectionWidth;
    int h = projectionHeight;
    
    fbo.begin();
    // MAIN WINDOW
    if(drawBg) ofBackground(0, 1);
    // grayImage.draw(0, 0);
    
    float paddingW = projectionWidth/24;
    float paddingH = projectionHeight/14;
    
    ofTranslate(-paddingW, -paddingH);
    
    for(int c = 0; c < numCountorFinder; c++){
        RectTracker& tracker = contourFinders[c].getTracker();
        
        cout << c << " size " << contourFinders[c].size() << endl;
        
        for(int i = 0; i < contourFinders[c].size(); i++) {
            // get contour, label, center point, and age of contour
            vector<cv::Point> points = contourFinders[c].getContour(i);
            
            // int label = contourFinder.getLabel(i);
            // ofPoint center = toOf(contourFinders[c].getCenter(i));
            // int age = tracker.getAge(label);
            
            // map contour using calibration and draw to main window
            ofSetColor(blobColors[i % 4], opacity);
            ofSetPolyMode(OF_POLY_WINDING_ODD);    // this is the normal mode
            ofFill();
            //
            ofBeginShape();
            // cout << "points.size " << points.size() << endl;
            for (int j=0; j<points.size(); j++) {
                ofVec3f worldPoint = kinect.getWorldCoordinateAt(points[j].x, points[j].y);
                ofVec2f projectedPoint = kpt.getProjectedPoint(worldPoint);
                // draw vertex
                ofVertex(points[j].x * ((projectionWidth + paddingW*2) / kinect.width), points[j].y * ((projectionHeight + paddingH*2) / kinect.height));
                // ofVertex( projectionWidth * projectedPoint.x, projectionHeight * projectedPoint.y );

            }
            ofEndShape();
        }
        
    }
    
    fbo.end();
}

/*
  \           /             |    \
    \        /              |     \
      \     /      a        |      \
       \   /    a  A  A     |       |       *
        | |   a A  a  a a   |       |       *
        | |     a  A  a     |      /
       /   \       a        |     /
     /      \               |    /
    /        \              |   /
 */

void ofApp::draw() {
    
    // GUI
    //ofBackground(0);
    ofSetColor(255);
    
    if(!noProjector) {
        ofPushMatrix();
        kinect.draw(0, 0);
        ofTranslate(640, 0);
        grayImage.draw(0, 0);
        ofTranslate(-640, 480);
        contourFinder.draw();
        ofTranslate(640, 0);
        ofPopMatrix();
    } else {
        ofBackground(0);
        if(displayTexture) fbo.draw(0, 0);
        mainOutputSyphonServer.publishScreen();
    }
    
    gui.draw();
    
}

void ofApp::drawSecondWindow(ofEventArgs &args){
    
    if(!noProjector) {
        ofBackground(0);
        ofSetColor(255);
        
        if(displayTexture) fbo.draw(0, 0);
        
    }
    
    // mainOutputSyphonServer.publishScreen();
}

void ofApp::keyPressed(int key) {
   
}
