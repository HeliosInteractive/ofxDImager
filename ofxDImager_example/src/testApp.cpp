#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup()
{

	ofSetLogLevel( OF_LOG_VERBOSE ) ; 
	dimager.setup() ; 
	setupUI() ; 
	ofSetFrameRate( 30 ) ;
}

//--------------------------------------------------------------
void testApp::update(){
	dimager.update() ;
}

//--------------------------------------------------------------
void testApp::draw(){
	ofSetColor( 255 ) ; 
	dimager.draw( 330 ,  15 ) ;

}

	 //SETUP UI
void testApp::setupUI( ) 
{
	float dim = 24; 
	float xInit = OFX_UI_GLOBAL_WIDGET_SPACING; 
    float length = 320-xInit; 

    
    depthGui = new ofxUICanvas(0, 0, length+xInit, ofGetHeight());

	depthGui->addButton( "SAVE SETTINGS" , false ) ; 
	depthGui->addButton( "LOAD SETTINGS" , false ) ; 

	dimager.setupGUI( depthGui ) ; 
	
	//minHeightForTrigger
	ofAddListener(depthGui->newGUIEvent,this,&testApp::guiEvent);	
	depthGui->loadSettings( "GUI/dimagerSettings.xml" ) ;

}


void testApp::guiEvent(ofxUIEventArgs &e){
	string name = e.widget->getName(); 
	int kind = e.widget->getKind(); 

	if ( name == "SAVE SETTINGS" ) 
	{
		depthGui->saveSettings( ofToDataPath( "GUI/dimagerSettings.xml" ) ) ; 
	}

	if ( name == "LOAD SETTINGS" ) 
	{
		depthGui->loadSettings( ofToDataPath( "GUI/dimagerSettings.xml" ) ) ; 
	}

	if ( name == "DEPTH RANGE" ) 
	{
		ofxUIRangeSlider * slider = (ofxUIRangeSlider*)e.widget ; 

		dimager.nearThreshold = slider->getScaledValueLow() ; 
		dimager.farThreshold = slider->getScaledValueHigh() ;
	}

	if ( name == "BLOB SIZE RANGE" ) 
	{
		ofxUIRangeSlider * slider = (ofxUIRangeSlider*)e.widget ; 

		int maxPixels= 160 * 120 ; 
		dimager.maxBlobSize = slider->getScaledValueHigh() * maxPixels ; 
		dimager.minBlobSize = slider->getScaledValueLow() * maxPixels ; 
	}
	
	if ( name == "BLOBS BLUR" ) 
	{
		ofxUISlider * slider = ( ofxUISlider* )e.widget ; 
		int _blurAmount = (int)slider->getScaledValue() ;
		if ( _blurAmount % 2 == 0 ) 
			_blurAmount++ ; 

		dimager.grayBlobsBlurAmount = _blurAmount ; 
	}

	//Change the Threshold settings
	if ( name == "DEPTH THRESHOLD" || name == "GRAYSCALE THRESHOLD" )
	{
		dimager.changeHardwareThresholds(  dimager.grayscaleThreshold , dimager.depthThreshold ) ; 
	}

	
	if ( name == "IR FREQUENCY" ) 
	{
		ofxUISlider * slider = ( ofxUISlider* )e.widget ; 
		int val = (int)slider->getScaledValue() ; 
		dimager.changeOperatingFrequency( val ) ; 
	}
	
	if ( name ==  "USE RECORDED VIDEO" ) 
	{
		ofxUIToggle * toggle   = (ofxUIToggle*)e.widget ; 

		dimager.bUseVideo = toggle->getValue() ; 

		if ( toggle->getValue() == true ) 
		{
			if ( dimager.recordedVideo.isLoaded() == false ) 
			{	
				ofFileDialogResult result = ofSystemLoadDialog( "Pick your quicktime movie !" , false ) ; 
				if ( result.bSuccess == true ) 
				{
					string path = result.getPath() ; 
					dimager.loadDepthVideo( path ) ; 
				}
			}

		}
	}
}


//--------------------------------------------------------------
void testApp::keyPressed(int key){

}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}
