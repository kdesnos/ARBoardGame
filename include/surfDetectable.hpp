#pragma once

#include <opencv2/core.hpp>

#include "nonAssignable.hpp"

/**
* This class is a base class for all classes that are detected by the SURF 
* detector.
*/
class SURFDetectable : public virtual NonAssignable {

	/**
	* VisionEngine class is a friend, it can access attributes of SURFDetectable.
	*/
	friend class VisionEngine;
	
protected:
	/**
	* Detected image for a SURF detector
	*/
	const cv::Mat _image;

	/**
	* Keypoints of the SURFDetectable object.
	* This vector is to be filled by the VisionEngine::_detector
	*/
	std::vector<cv::KeyPoint> _keypoints;

	/**
	* Descriptors of the _keypoints of the SURFDetectable object.
	* This vector are to be filled by the VisionEngine::_detector
	*/
	cv::Mat _descriptors;

	/**
	* Constructor for the SURFDetectable Class
	* Initialize the const _image attribute with the given image
	*/
	SURFDetectable(cv::Mat image);
};