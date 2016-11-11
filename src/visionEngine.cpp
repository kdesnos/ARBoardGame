
#include "logger.hpp"
#include "visionEngine.hpp"

using namespace std;

void VisionEngine::_computeKeypointsAndDescriptors(SURFDetectable & detectable) const
{
	_detector->detectAndCompute(detectable._image, cv::Mat(), detectable._keypoints, detectable._descriptors);
}

VisionEngine::VisionEngine(ostream & logger) :
	_logger(logger), _camera(NULL), _initialized(false),
	_exitDetectionLoop(false), _detectionLoopMutex(),
	_detectionLoopLaunchMutex(), _detectionLoopThread(NULL),
	_patternsMutex(),
	_detector(cv::xfeatures2d::SURF::create(MIN_HESSIAN))
{
	LOG(_logger, "Instantiate VisionEngine");
}

VisionEngine::~VisionEngine() {
	LOG(_logger, "Delete Vision Engine");

	// Need to wait for thread completion (and hope this won't stall the 
	// application)
	stopDetectionThread();

	// Delete the camera (if it was initialized)
	if (isInitialized()) {
		delete _camera;
	}

	// Flush the logger.
	_logger.flush();
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

	// Get the mutex for modifying the _patterns attribute
	lock_guard<mutex> lockPatternMutex(_patternsMutex);

	// Put the pattern in _pattern list
	_patterns.push_back(pattern);

	return true;

	// Automatically unlock the mutex.
}

bool VisionEngine::executeDetectionLoopOnce()
{
	LOG(_logger, "Starting detectionLoop once.");

	// Get both detectionLoop and launcherMutex simultaneously
	unique_lock<mutex> lockDetectionLoopLaunchMutex = unique_lock<mutex>(_detectionLoopLaunchMutex, defer_lock);
	unique_lock<mutex> lockDetectionLoopMutex = unique_lock<mutex>(_detectionLoopMutex, defer_lock);

	// Try to lock both mutexes simultaneously
	if (try_lock(lockDetectionLoopLaunchMutex, lockDetectionLoopMutex) != -1) {
		// Lock failed
		LOG(_logger, "Failed to start detection thread because detectionLoop is already being launched or running.");
		return false;
	}

	// Unlock detectionLoopMutex
	lockDetectionLoopMutex.unlock();

	// Set exitDetectionLoop to true
	setExitDetectionLoop(true);

	// Execute the detection loop.
	detectionLoop();

	LOG(_logger, "DetectionLoop was successfully executed once.");

	return true;
}

void VisionEngine::setExitDetectionLoop(const bool exit)
{
	_exitDetectionLoop = exit;
}

bool VisionEngine::startDetectionThread()
{
	LOG(_logger, "Starting detection thread.");

	// Get both detectionLoop and launcherMutex simultaneously
	unique_lock<mutex> lockDetectionLoopLaunchMutex = unique_lock<mutex>(_detectionLoopLaunchMutex, defer_lock);
	unique_lock<mutex> lockDetectionLoopMutex = unique_lock<mutex>(_detectionLoopMutex, defer_lock);

	// Try to lock both mutexes simultaneously
	if (try_lock(lockDetectionLoopLaunchMutex, lockDetectionLoopMutex) != -1) {
		// Lock failed
		LOG(_logger, "Failed to start detection thread because detectionLoop is already being launched or running.");
		return false;
	}

	// Unlock detectionLoopMutex
	lockDetectionLoopMutex.unlock();

	// Check initialization
	if (!isInitialized()) {
		LOG(_logger, "Failed to start detection thread because VisionEngine was not initialized.");
		return false;
	}

	// Check that loop is not already running
	if (_detectionLoopThread != NULL) {
		// Should never happen thanks to mutexes
		LOG(_logger, "Failed to start detection thread because detectionLoop is already running.");
		return false;
	}

	// Set exitDetectionLoop to false
	setExitDetectionLoop(false);

	// Start the thread
	_detectionLoopThread = new thread(&VisionEngine::detectionLoop, this);

	LOG(_logger, "Detection thread started successfully.");

	// Everything went well
	return true;

	// _detectionLoopLaunchMutex automatically unlocked thanks to unique_lock destruction
}

bool VisionEngine::stopDetectionThread()
{
	LOG(_logger, "Stopping detection thread.");

	// Checkups
	if (_detectionLoopThread == NULL) {
		LOG(_logger, "Failed to stop detection thread because no thread was running.");
		return false;
	}

	// Set exitDetectionLoop to true 
	setExitDetectionLoop(true);

	// Wait for thread completion (possibly forever...)
	_detectionLoopThread->join();

	// Delete the thread.
	delete _detectionLoopThread;
	_detectionLoopThread = NULL;

	LOG(_logger, "Detection thread stopped successfully.");
	return true;
}

bool VisionEngine::unregisterPattern(const Pattern & pattern)
{
	// Get the mutex for modifying the _patterns attribute
	lock_guard<mutex> lockPatternMutex(_patternsMutex);

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

	// Automatically unlock the mutex.
}

void VisionEngine::detectionLoop()
{
	// Acquire the _detectionLoopMutex (automatically released when exiting 
	// this context).
	std::unique_lock<mutex> lockDetectionLoopMutex(_detectionLoopMutex);

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

bool VisionEngine::doesExitDetectionLoop() const
{
	return _exitDetectionLoop;
}

