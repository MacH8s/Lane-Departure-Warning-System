#include "LiveVideo.h"

// Default constructor
LiveVideo::LiveVideo():LDWS()
{}

// Method applying the LDWS algorithm
void LiveVideo::applyAlgorithm()
{
	//Capturing the live video stream
	VideoCapture video(0);

	//Boolean storing if the first frame has been implemented
	bool isFirstFrame = true;

	while(true)
	{
		//Retreiving and cloning video image
		video >> img;
		sourceImg = img.clone();
	
		//Checking if image exist
		if(sourceImg.empty())
			return;
		
		//Initializing ROI if first video frame
		if(isFirstFrame)
		{
			imageTransformation = ImageTransformation(sourceImg);
			roi.setROI(imageTransformation.getWidth(), imageTransformation.getHeight());
			roi.computeHomography();
			roi.computeInverseHomography();
		}

		laneAnalysis = LaneAnalysis();

		imageTransformation.applyIPM(sourceImg, ipmImage, roi.getIpmHomography());

		imageTransformation.applyGaussianBlur(ipmImage);

		imageTransformation.applyCvtColor(ipmImage);

		imageTransformation.applyCanny(ipmImage);

		laneAnalysis.setSourceLines(imageTransformation.applyHoughLinesP(ipmImage));

		laneAnalysis.sortLinesByAngle(ipmImage.cols);

		laneAnalysis.getSingleLaneSegment();

		laneAnalysis.getLineFromSegment(ipmImage.rows);

		laneAnalysis.addIPMPoints();
		
		//Applying inverse IPM to detected lanes
		if(laneAnalysis.getRightLaneDetected() == true || laneAnalysis.getLeftLaneDetected() == true)
			laneAnalysis.setlinePointsSourceImage(imageTransformation.applyReverseIPM(roi.getInverseHomography(), laneAnalysis.getLinePointsIPMImage()));
		
		laneAnalysis.drawFinalLines(sourceImg);

		//Read GPIO input pins
		bool pin22High = gpioHandler.readPin22();
		bool pin23High = gpioHandler.readPin23();
		bool leftDepartureWarning = false;
		bool rightDepartureWarning = false;

		laneAnalysis.checkAndDrawDeparture(sourceImg, sourceImg.cols, pin22High, pin23High, leftDepartureWarning, rightDepartureWarning);

		//Write GPIO output pins
		gpioHandler.writePin24(leftDepartureWarning && !pin22High);
		gpioHandler.writePin25(rightDepartureWarning && !pin23High);
		
		roi.showROI(sourceImg);

		//Showing original image with detected lanes
		imshow( "FinalImage", sourceImg);
			
		isFirstFrame = false;

		waitKey(1);
	}
}
