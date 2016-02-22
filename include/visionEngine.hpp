/**
* \brief This file contains the definition of the VisionEngine Class
*/

#include <iostream>

#include "opencv2/opencv.hpp"

using namespace std;

/**
* \brief The purpose of the VisionEngine class is to be the interface between the game
* engine and the computer vision functionnalities.
* \authot kdesnos
*/
class VisionEngine {

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
	bool _initialized = false;

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
};