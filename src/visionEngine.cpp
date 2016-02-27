
#include "logger.hpp"
#include "visionEngine.hpp"

using namespace std;

VisionEngine::VisionEngine(ostream & logger) :
	_logger(logger), _camera(NULL), _initialized(false)
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

bool VisionEngine::registerPattern(Pattern & pattern)
{
	// Log
	LOG(_logger, "Registering pattern " + pattern.getName());

	// Put the pattern in _pattern list
	_patterns.push_back(pattern);

	return true;
}

bool VisionEngine::unregisterPattern(const Pattern & pattern)
{
	const int originalSize = _patterns.size();
	// Lambda expression used because operator== of pattern is masked by
	// std::reference_wrapper.
	_patterns.remove_if([&pattern](const Pattern& pat) {return pat == pattern; });

	// Check success of the remove: positive if exactly one element was removed
	const bool success = _patterns.size() == originalSize - 1;

	LOG(_logger, "Unregistering pattern " 
		+ pattern.getName() 
		+ ": " 
		+ ((success) ? "Success" : "Failure"));

	return success;
}

