
#include <time.h>
#include "logger.hpp"
#include "visionEngine.hpp"

using namespace std;

VisionEngine::VisionEngine(ostream & logger) : _logger(logger)
{
	LOG(_logger, "Instantiate VisionEngine");
}

VisionEngine::~VisionEngine() {
	LOG(_logger, "Delete Vision Engine");

	// Flush the logger.
	_logger.flush();

	// Delete the camera (if it was initialized)
	if (isInitialized()) {
		delete _camera;
	}
}

bool VisionEngine::initialize() {

	if (!_initialized) {
		// Open the default camera
		_camera = new cv::VideoCapture(0);

		// Check if we succeeded
		if (_camera->isOpened()) {
			_initialized = true;
		} else {
			delete _camera;
			_initialized = false;
		}
	}

	// Log
	if (_initialized) {
		LOG(_logger, "Initialization of the VisionEngine was successfull.")
	} else {
		LOG(_logger, "Initialization of the VisionEngine failed.")
	}

	return _initialized;
}

bool VisionEngine::isInitialized() const {
	return _initialized;
}

