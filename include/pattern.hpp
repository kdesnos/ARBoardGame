/**
* This file contains the class used to characterize image patterns that are 
* detected by the VisionEngine class
*
* \author kdesnos
*/

#pragma once

#include <string>
#include <list>

#include "opencv2/opencv.hpp"

#include "nonAssignable.hpp"

/**
* Pattern class used for detecting images with the VisionEngine
*/
class Pattern : public NonAssignable
{

public:
	/**
	* Constructor of the Pattern class receiving a path to an image of the 
	* pattern to track.
	* 
	* The constructors opens the image corresponding. Failing that, the 
	* constructor will throw an IOException.
	*
	* \param[in] name string containing the name of the created pattern. This
	* name will be stored for further use.
	* \param[in] path string containing the path to the image to load in the 
	* pattern. This path is not stored in the Pattern.
	*
	* \throws FileNotFoundException if the given path is invalid.
	*/
	Pattern(const std::string & name, const std::string & path) throw(...);

	/**
	* Get the _name of the Pattern
	*/
	const std::string & getName() const throw() ;

	/**
	* Operator used for comparing two patterns. Since no two patterns can have the 
	* same name, only names are used for comparison.
	*/
	bool operator==(const Pattern &other) const;

protected:

	/**
	* Name used to uniquely identify the current Pattern.
	* Two patterns cannot be created with the same name 
	* (enforced by the constructor).
	*/
	const std::string _name;

	/**
	* Image loaded as the test pattern.
	*/
	const cv::Mat _image;

private:
	static std::list<std::reference_wrapper<Pattern &>> _allPatterns;
};

