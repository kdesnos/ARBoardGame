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

#include "surfDetectable.hpp"
#include "nonAssignable.hpp"

/**
* Pattern class used for detecting images with the VisionEngine
*/
class Pattern : public SURFDetectable, virtual NonAssignable
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
	* Threshold used to determine whether a detectionConfidence of a pattern 
	* is positive or negative.
	*/
	static const double DETECTION_CONFIDENCE_THRESHOLD ;

	/**
	* Step used to increase (or decrease) the value of the detectionConfidence.
	*/
	static const double DETECTION_CONFIDENCE_STEP;

	/**
	* Get the _name of the Pattern
	*/
	const std::string & getName() const throw() ;

	/**
	* Get the current _detectionConfidence of the Pattern
	*
	* \return double value of the _detectionConfidence
	*/
	double getDetectionConfidence() const;

	/**
	* Increase the current value of the detectionConfidence for the Pattern.
	*
	* \return double value of the _detectionConfidence
	*/
	double increaseDetectionConfidence();

	/**
	* Decrease the current value of the detectionConfidence for the Pattern.
	*
	* \return double value of the _detectionConfidence
	*/
	double decreaseDetectionConfidence();

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
	* Detection confidence for the pattern.
	*
	* A detectionConfidence set to 0 means that the pattern is not detected at all.
	* A detectionConfidence set to 1 means that the pattern is fully detected.
	* A detectionScore in between means that pattern detection is building (or 
	* decreasing confidence). 
	*
	* Threshold between positive and negative detection is set by the 
	* Pattern::DETECTION_CONFIDENCE_THRESHOLD constant.
	*/
	double _detectionConfidence;

	create position class

private:
	static std::list<std::reference_wrapper<Pattern &>> _allPatterns;
};

