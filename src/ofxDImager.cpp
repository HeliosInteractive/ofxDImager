
#include "ofxDImager.h"

ofxDImager::ofxDImager()
{ 
	bMirrorX = false; 
	bMirrorY = false ; 
	bUseVideo = false ;
}

ofxDImager::~ofxDImager() 
{
	disconnect() ; 

	delete[] depthBuffer ; 
	delete[] grayscaleBuffer ; 
}

void ofxDImager::setup( ) 
{
	ofLog( OF_LOG_VERBOSE , "DLL version : " + ofToString( GetDLLversion() ) ) ; 
	ofLog( OF_LOG_VERBOSE , "Hardware Version : " + ofToString( GetHardversion() ) ) ; 

	bUseVideo = false ; 

	connect() ; 

	if ( !bConnected ) 
		disconnect() ; 

	nearThreshold = 5 ;
	farThreshold = 240 ; 

	grayImage.allocate(160 , 120 );
	grayThreshNear.allocate(160 , 120 );
	grayThreshFar.allocate(160 , 120 );
	grayBlobs.allocate(160 , 120 );
	
	//grayBg.allocate( 160 , 120 ) ; 
	//grayDiff.allocate( 160 , 120 ) ;

	depthBuffer=new unsigned short[160*120];
	grayscaleBuffer=new unsigned short[160*120];

	totalSentFrames = 0 ; 
	roiRect = ofRectangle( 20  , 10 , 40 , 105 ) ; 
	connectionTimer.setup( 1000 , "DIMAGER CONNECTION" ) ;
	ofAddListener( connectionTimer.TIMER_COMPLETE , this , &ofxDImager::connectTimerCompleteHandler ) ; 
}

void ofxDImager::connectTimerCompleteHandler( int &args ) 
{
	stringstream ss ; 
	ss << "Attempting to connect @... : " << ofGetElapsedTimef() << endl; 
	ofLog ( OF_LOG_VERBOSE , ss.str() ) ; 
	connect( ) ; 
}

void ofxDImager::changeOperatingFrequency( int mode ) 
{
	int result = Freqmode( ) ; 

	if ( operatingFrequency == mode ) 
	{
		ofLog( OF_LOG_WARNING , "Dimager Frequency Mode is already " + ofToString( mode ) ) ; 
		return ; 
	}

	stringstream ss ; 
	switch ( result ) 
	{
		case 0 : 
		case 1 : 
		case 2 : 
			ss << "Operating Frequency is now " << mode << endl ; 
			operatingFrequency = mode ; 
			break ; 
		case 4 : 
			ss << "Setting operating mode to : " << mode << " failed." << endl ;
			break ; 

		default :
			ss << "unhandled operating mode of : " << mode << endl ; 
			break ;
	}

	

	ofLog( OF_LOG_WARNING , ss.str() ) ; 
}


int ofxDImager::getOperatingFrequency() 
{
	int result = Freqmode( ) ;
	return result ; 
}


void ofxDImager::update ( ) 
{
	if ( bAutoReconnect == true ) 
		connectionTimer.update() ;
	if  ( !bConnected ) 
	{
		connectionTimer.start( true , false ) ; 
	}
	else
	{
		connectionTimer.stop() ; 
		connectionTimer.reset(); 
	}

	int numPixels = grayImage.getWidth() * grayImage.getHeight();
	unsigned char * depthMap = new unsigned char [numPixels] ; 

	int averageInt  = 0 ; 
	if ( bUseVideo == true && recordedVideo.isLoaded() == true ) 
	{
		recordedVideo.update() ; 
		ofPixels pix = recordedVideo.getPixelsRef() ; 
		pix.setImageType( OF_IMAGE_GRAYSCALE ) ; 
		grayImage.setFromPixels( pix ) ; 
	}
	else
	{
		int result = GetImageKN(depthBuffer,grayscaleBuffer) ;
		if ( result == 0 ) 
		{
			
			for ( int i = 0 ; i < numPixels ; i++ ) 
			{
				float maxDepth = 1500 ; 
				depthMap[i] = ofMap( depthBuffer[i] , 0 , maxDepth , 0 , 255 , true ) ; //(depthBuffer[i] * (1.0f/(float)MAX_DEPTH )) * 255 ;
			}
				
			grayImage.setFromPixels( depthMap , 160 , 120 ) ; ; 
			grayImage.mirror( bMirrorX , bMirrorY ) ;

		}
		else
		{
			stringstream ss ; 

			ss << "result of " <<result << "was not valid ! " ;  
			switch ( result )
			{
				case 1 :
					ss << "FAIL - timeout expired" << endl ; 
					break ; 
				case 2 :
					ss << "FAIL - device has not been initialized" << endl ;
					break ; 
				case 3 : 
					ss << "FAIL - capture image is not a grayscale image or range" << endl ;
					break ; 
			}
			disconnect() ; 
			bConnected = false ; 
			ss << "is Dimager connected ? " << ofToString( bConnected ) << endl ; 
			debugString = ss.str() ; 
			delete[] depthMap ; 
			return ; 
		}
	}
	

	/*
	if ( initialGrayBlurAmount > 0 ) 
	{
		grayImage.blurGaussian( initialGrayBlurAmount ) ; 
		grayImage.flagImageChanged() ; 
	}*/


	/*
	if (bLearnBackground == true )
	{
		grayBg = grayImage;		// the = sign copys the pixels from grayImage into grayBg (operator overloading)
		bLearnBackground = false;
		cout << "taking a snapshot! " << endl ; 
	}


	// take the abs value of the difference between background and incoming and then threshold:
	grayDiff.absDiff( grayBg, grayImage );
	grayDiff.threshold( bgThreshold , true ) ; 

	*/
	grayBlobs.setROI( grayImage.getROI() ) ; 
	grayBlobs = grayImage ;
	grayThreshNear = grayImage ; 
	grayThreshFar = grayImage ; 

	
	stringstream ss ; 

	ss << "is Dimager connected ? " << ofToString( bConnected ) << endl ;
		
	grayThreshNear.threshold( nearThreshold , false ) ; 
	grayThreshFar.threshold( farThreshold , true );
	cvAnd( grayThreshNear.getCvImage() , grayThreshFar.getCvImage() , grayBlobs.getCvImage(), NULL ) ; 

	/*
	
	*/
	if ( grayBlobsBlurAmount > 0 ) 
	{
		grayBlobs.blurGaussian( grayBlobsBlurAmount ) ; 
		grayBlobs.flagImageChanged(); 
	}

	
	grayBlobs.setROI( roiRect ) ; 
	contourFinder.findContours( grayBlobs , minBlobSize , maxBlobSize , 1 , bFindHoles , bUseApproximation ) ; 
	delete[] depthMap ;
	debugString = ss.str() ;
}



void ofxDImager::connect() 
{

	int result = InitImageDriver();
	stringstream ss ; 
	ss << "starting up DImager... : " << result << "|" ; 
	if ( result == 0 ) {
		ss <<  "Sucessful! Or device is already open." << endl ; 
		bConnected = true ; 
	}
	if ( result == 1 ) {
		ss <<  "Failed to connect to device" << endl ; 
		FreeImageDriver() ; 
		bConnected = false ; 
	}
	if ( result == 2 ) {
		ss <<  "Failed to connect to end Point" << endl ; 
		FreeImageDriver() ; 
		bConnected = false ;
	}

	
	ofLog( OF_LOG_VERBOSE, ss.str() ) ; 
}

void ofxDImager::disconnect() 
{
	int result = FreeImageDriver();
	stringstream ss ; 

	switch ( result ) 
	{
		case 0 : 
			ss << "successfully disconnected" << endl ; 
			break ; 

		case 1 : 
			ss << "problem disconnecting... " << endl; 
			break ; 

		default : 
			ss << "disconnect() response unhandeled" << endl ;
			break ;
	}
}
	
int ofxDImager::calculateCentroidPixelBrightness( ofPoint centroid ) 
{
	int sum = 0 ; 
	int count = 0 ; 
	sum += getPixelAt( centroid.x , centroid.y , count ) ; 
	sum += getPixelAt( centroid.x - 2 , centroid.y + 2 , count ) ; 
	sum += getPixelAt( centroid.x - 2 , centroid.y - 2 , count ) ; 
	sum += getPixelAt( centroid.x + 2 , centroid.y - 2 , count ) ;
	sum += getPixelAt( centroid.x + 2 , centroid.y + 2 , count ) ;

	int average = ( 1.0f / (float)count ) * sum ; 
	return average ; 
}

int ofxDImager::getPixelAt( int x , int y, int & counter  ) 
{
	if ( x >= 0 && x < grayImage.getWidth() &&
		 y >= 0 && y < grayImage.getHeight() ) 
	{
		
		int brightness = grayImage.getPixelsRef().getColor( x , y ).b ; 
		if ( brightness > 5 ) 
		{
			counter++ ; 
			return brightness ; 
		}
		else
		{
			return 0 ; 
		}
	}
	else
	{
		return 0 ; 
	}
}

void ofxDImager::draw( float x , float y ) 
{
	ofPushStyle() ; 
	ofPushMatrix() ; 
	ofTranslate( x , y ) ; 
		ofSetColor ( 255 , 255 , 255 ) ; 
		grayImage.draw ( 0 , 0 , 160 , 120 ) ;
		recordedVideo.draw( 175 , 0 , 160 , 120 ) ;

		//grayBg.draw( 350 , 0 , 160 , 120 ) ; 
		//grayDiff.draw( 525 , 0 , 160 , 120 ) ; 

		ofSetColor ( 255 , 255 , 255 ) ; 
		ofDrawBitmapStringHighlight ( "DEPTH FEED" , 15 , 140 ) ; 
		ofDrawBitmapStringHighlight ( "DEPTH VIDEO" , 175 , 140 ) ; 
		//ofDrawBitmapStringHighlight ( "BG SNAPSHOT" , 350 , 140 ) ;
		//ofDrawBitmapStringHighlight ( "GRAY DIFF" , 525 , 140 ) ;
			
		ofTranslate( 170 , 0 ) ;

		
		grayBlobs.draw ( 0 , 0 ) ; 
		ofPushMatrix() ;
			ofTranslate( roiRect.x , roiRect.y ) ;
			contourFinder.draw ( 0 , 0  ) ; 
			ofPushStyle() ; 
				ofNoFill() ; 
				ofSetColor( 255 , 212 , 0 ) ;
				ofSetLineWidth ( 2 ) ;
				ofRect( 0 , 0 , roiRect.width , roiRect.height ) ;
			ofPopStyle() ;

			if ( contourFinder.blobs.size() > 0 ) 
			{
				ofPushStyle() ; 
				ofSetColor( 0 , 255 , 0 , 180 ) ; 
				ofCircle( contourFinder.blobs[0].centroid , 6 ) ; 

				ofPopStyle() ;
			}
		ofPopMatrix() ;

		ofDrawBitmapStringHighlight ( "User Blobs" , 0 , 140 ) ; 

	ofPopMatrix() ;
}
	
void ofxDImager::changeHardwareThresholds( float grayscaleThreshold, float depthThreshold ) 
{
	int result = ChangeGrayscalePara( grayscaleThreshold , depthThreshold ) ;

	stringstream ss; 
	if ( result == 0 ) 
		ss << "CAMERA SETTINGS changed successfully! " << endl ;
	if ( result == 1 ) 
		ss << "Parameter change failed! " << endl ;
	if ( result == 2 ) 
		ss << "Parameter out of range! " << endl ;
	
	//CAMERA
	ofLog( OF_LOG_VERBOSE, ss.str() ) ;  
}

void ofxDImager::loadDepthVideo( string videoPath ) 
{
	recordedVideo.loadMovie( videoPath ); 
	bUseVideo = true ; 
	recordedVideo.play() ; 
}

void ofxDImager::toggleVideoPause( ) 
{
	if ( recordedVideo.isPaused() == false ) 
		recordedVideo.setPaused( true ) ; 
	else
	{
		recordedVideo.setPaused( false ) ;
		recordedVideo.play() ; 
	}	
}

void ofxDImager::setupGUI( ofxUICanvas * gui ) 
{
	gui->addLabel("DIMAGER HARDWARE SETTINGS" ) ; 
	gui->addSlider ( "DEPTH THRESHOLD" , 0.0f ,1500.0f , &depthThreshold ) ; 
	gui->addSlider ( "GRAYSCALE THRESHOLD" , 0.0f , 30.0f , &grayscaleThreshold ) ; 
	gui->addSlider ( "IR FREQUENCY" , 	1 , 3  , operatingFrequency ) ; //operatingFrequency
	//gui->addToggle ( "TAKE BG SNAPSHOT" , bLearnBackground ) ; 
	//gui->addSlider ( "BG THRESHOLD" , 0.0f, 100.0f , &bgThreshold ) ; 
	gui->addToggle( "MIRROR X" , &bMirrorX ) ; 
	gui->addToggle( "MIRROR Y" , &bMirrorY ) ; 

	gui->addToggle( "USE RECORDED VIDEO" , false ) ; 
	gui->addToggle( "AUTO RECONNECT" , &bAutoReconnect ) ; 

	gui->addToggle( "FIND CV HOLES" , &bFindHoles ) ; 
	gui->addToggle( "USE CV APPROXIMATION" , &bUseApproximation ) ; 

	gui->addSlider ("ROI RECT X" , 0 , 159 , &roiRect.x ) ; 
	gui->addSlider ("ROI RECT Y" , 0 , 119 , &roiRect.y ) ; 
	gui->addSlider ("ROI RECT WIDTH" , 0 , 159 , &roiRect.width ) ; 
	gui->addSlider ("ROI RECT HEIGHT" , 0 , 119 , &roiRect.height ); 

	gui->addLabel( "DEPTH IMAGE PARAMS" ) ; 
	gui->addRangeSlider( "DEPTH RANGE" , 0.0f , 255 , nearThreshold, farThreshold ) ; 
	gui->addRangeSlider( "BLOB SIZE RANGE" , 0.0f , 1.0f , minBlobSize , maxBlobSize ) ; 
	//gui->addSlider( "INITIAL DEPTH BLUR" , 1 , 33 , initialGrayBlurAmount ) ;
	gui->addSlider( "BLOBS BLUR" , 1 , 33 , grayBlobsBlurAmount ) ;
	
}