#pragma once

#define MAX_DEPTH 1500

#include "ofMain.h"
#include "ofxOpenCV.h"
#include "Dimagerdll.h"
#include "ofxSimpleTimer.h"
#include "ofxUI.h"

class ofxDImager
{
	public:

		ofxDImager() ; 
		~ofxDImager() ; 

		
		void setup( ) ;
		void update ( ) ;  
		void draw( float x , float y ) ;
		void drawUserRep( float x , float y ) ;
		void loadDepthVideo( string videoPath ) ; 
		void toggleVideoPause( ) ; 

		void connect() ; 
		void disconnect() ; 
		bool isConnected()
		{
			return bConnected ;
		}

		bool bAutoReconnect ; 

		void changeHardwareThresholds( float grayscaleThreshold, float depthThreshold ) ; 
	
		bool bMirrorX , bMirrorY ;
		unsigned int totalSentFrames ; 

		ofxCvGrayscaleImage grayImage;			// grayscale depth image
		ofxCvGrayscaleImage grayThreshNear;		// the near thresholded image
		ofxCvGrayscaleImage grayThreshFar;		// the far thresholded image 
		ofxCvGrayscaleImage grayBlobs ;			// blobs depth image ( blurred ) 
		
		//If you want to use video
		bool bUseVideo ; 
		int calculateCentroidPixelBrightness( ofPoint centroid ) ; 
		int getPixelAt( int x , int y , int &counter ) ; 
		ofVideoPlayer recordedVideo ; 

		//Depth Feed Parameters
		int nearThreshold , farThreshold , minBlobSize , maxBlobSize ;
		bool bFindHoles , bUseApproximation ; 
		ofxCvContourFinder 	contourFinder;

		ofRectangle roiRect ; 

		bool useOpenCV ;

		float grayscaleThreshold ; 
		float depthThreshold ; 
		
		int initialGrayBlurAmount ; 
		int grayBlobsBlurAmount ; 

		ofxSimpleTimer connectionTimer ;
		void connectTimerCompleteHandler( int &args ) ; 

		int operatingFrequency ; 
		void changeOperatingFrequency( int mode ) ; 
		int getOperatingFrequency() ; 

		void setupGUI( ofxUICanvas *gui ) ; 

		unsigned short *depthBuffer;
		unsigned short *grayscaleBuffer;

		string debugString ; 
		//Background removal
		ofxCvGrayscaleImage 	grayBg;
		ofxCvGrayscaleImage 	grayDiff;
		bool				bLearnBackground;
		float				bgThreshold ; 

	private : 

		bool bConnected ; 
};