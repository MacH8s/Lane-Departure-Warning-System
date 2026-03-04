#include "StaticImage.h"

// Default constructor
StaticImage::StaticImage(string path):LDWS(path)
{}

// Method applying the LDWS algorithm
void StaticImage::applyAlgorithm()
{
	//Store and reading original image in img matrix
	sourceImg = imread(inputPath);
	
	// Check for invalid image
	if(!sourceImg.data)                              
	{
		cout <<  "Could not open or find the image" << std::endl;
		return;
	}
	else
	{
		imageTransformation = ImageTransformation(sourceImg);
		roi.setROI(imageTransformation.getWidth(), imageTransformation.getHeight());
		roi.computeHomography();
		roi.computeInverseHomography();
		while(1)
		{
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

			// Wait until user exit program by pressing a key
			waitKey(0);
		}
	}
}
