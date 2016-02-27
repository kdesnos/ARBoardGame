/**
* \brief This file contains the definition of the VisionEngine Class
*/
#pragma once

#include <iostream>
#include <list>

#include "opencv2/opencv.hpp"

#include "pattern.hpp"

using namespace std;

/**
* \brief The purpose of the VisionEngine class is to be the interface between the game
* engine and the computer vision functionnalities.
* \authot kdesnos
*/
class VisionEngine : public NonAssignable {

protected:
	/**
	* \brief Logger that will be used to output a log of the behavior, event
	* and state of the class.
	*/
	std::ostream& _logger;

	/**
	* Camera used to captur image for the VisionEngine.
	* Initialized in the initialize method.
	*/
	cv::VideoCapture * _camera;

	/**
	* \brief Boolean attribute used to check whether the VisionEngine was
	* successfully initialized. {@link VisionEngine::initialize()}.
	*/
	bool _initialized;

	/**
	* Vector containing the references to the Pattern to detect in the images 
	* captured from the _camera.
	* std::reference_wrapper<Pattern> is used instead of <Pattern> because 
	* references cannot be directly stored in c++ container, and because 
	* Pattern derives from NonAssignable, which prevents it from being used
	* directly as the list template type argument.
	*/
	std::list<std::reference_wrapper<Pattern>> _patterns;

public:
	/**
	* \brief Default constructor for the VisionEngine.
	*
	* \param[in] logger
	* Logger that will be used to output a log of the behavior, event and
	* state of the object. Default value is std::cout.
	*/
	VisionEngine(std::ostream& logger = std::cout);

	/**
	* Destructor of the VisionEngine.
	*
	* Ensure that:
	* - _camera attribute is freed.
	* - logger is flushed.
	*/
	~VisionEngine();

	/**
	* \brief Initialize the vision engine.
	*
	* Initialization steps are:
	* <ul><li>Start the camera</li>
	* <li>Set {@link VisionEngine::_initialized} to <code>true</code> on
	* success of all previous steps, <code>false</code> otherwise.</li></ul>
	*
	* @return true if the initialization was successful, false otherwise.
	*/
	bool initialize();

	/**
	* Check whether there was a successful call to {@link initialize()} for
	* the current object.
	*/
	bool isInitialized() const;

	/**
	* Register a Pattern for detection by the VisionEngine in frames captured by
	* the _camera.
	*
	* The method will immediately compute the keypoints associated to the given 
	* Pattern and add it to _patterns.
	*
	* \param[in] pattern a reference to the Pattern to detect.
	*
	* \return true if the registeration was successful (or was already done 
	* before), false otherwise. Possible reasons for failing: none listed yet.
	*/
	bool registerPattern(Pattern & pattern);

	/**
	* Unregister a Pattern from detection by the VisionEngine in frames captured
	* by the _camera.
	*
	* \param[in] pattern a reference to the Pattern whose detection is no 
	* longer wanted.
	*
	* \return true if the unregisteration was successful false otherwise. 
	* Possible reasons for failing: pattern is not in the list _patterns to 
	* detect.
	*/
	bool unregisterPattern(const Pattern & pattern);
};