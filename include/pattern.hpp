/**
* This file contains the class used to characterize image patterns that are 
* detected by the VisionEngine class
*
* \author kdesnos
*/

#include <string>

#include "opencv2/opencv.hpp"

/**
* Pattern class used for detecting images with the VisionEngine
*/
class Pattern
{

public:
	/**
	* Constructor of the Pattern class receiving a path to an image of the 
	* pattern to track.
	* 
	* The constructors opens the image corresponding. Failing that, the 
	* constructor will throw an IOException.
	*
	* \param[in] path string containing the path to the image to load in the 
	* pattern
	*
	* \throws an IOException if the given path is invalid.
	*/
	Pattern(std::string path);

protected:

	/**
	* Image loaded as the test pattern.
	*/
	cv::Mat image;
};

