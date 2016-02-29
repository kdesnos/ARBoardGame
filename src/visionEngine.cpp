
#include "logger.hpp"
#include "visionEngine.hpp"

using namespace std;

void VisionEngine::_computeKeypointsAndDescriptors(SURFDetectable & detectable) const
{
	_detector->detectAndCompute(detectable._image, cv::Mat(), detectable._keypoints, detectable._descriptors);
}

VisionEngine::VisionEngine(ostream & logger) :
	_logger(logger), _camera(NULL), _initialized(false),
	_exitDetectionLoop(false),
	_detector(cv::xfeatures2d::SURF::create(MIN_HESSIAN))
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

	// Compute Keypoints and Descriptors for this pattern
	_computeKeypointsAndDescriptors(pattern);

	// Put the pattern in _pattern list
	_patterns.push_back(pattern);

	return true;
}

bool VisionEngine::unregisterPattern(const Pattern & pattern)
{
	const size_t originalSize = _patterns.size();
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

void VisionEngine::detectionLoop()
{
	LOG(_logger, "Entering detection loop.");
	// Exit immediately if the VisionEngine was not initialized
	if (!isInitialized()) {
		LOG(_logger, "Unexpected end of detection loop caused by an uninitialized VisionEngine")
		return;
	}

	// Enter the loop
	do {
		// 1. Acquire a new frame from the camera.
		LOG(_logger, "Acquiring a new frame.")
		cv::Mat frame;

		*_camera >> frame;

		// 2. Search for all registered patterns in the capture frame.
		// 3. Update the status and position of the detected patterns.
	} while (!_exitDetectionLoop);
}

