
#include "logger.hpp"
#include "visionEngine.hpp"

using namespace std;

void VisionEngine::_computeKeypointsAndDescriptors(SURFDetectable & detectable) const
{
	_detector->detectAndCompute(detectable._image, cv::Mat(), detectable._keypoints, detectable._descriptors);
}

void VisionEngine::_matchPatternInFrame(Pattern & pattern, SURFDetectable & scene) const
{
	//-- Step 1: Matching descriptor vectors using FLANN matcher
	cv::FlannBasedMatcher matcher;
	std::vector< cv::DMatch > matches;

	matcher.match(pattern._descriptors, scene._descriptors, matches);

	double max_dist = 0; double min_dist = 100;

	//-- Quick calculation of max and min distances between keypoints
	for (int i = 0; i < pattern._descriptors.rows; i++)
	{
		double dist = matches[i].distance;
		if (dist < min_dist) min_dist = dist;
		if (dist > max_dist) max_dist = dist;
	}

	//-- Localize the object
	std::vector<cv::Point2f> objectGoodKeypoints;
	std::vector<cv::Point2f> sceneGoodKeypoints;
	for (int i = 0; i < pattern._descriptors.rows; i++)
	{
		if (matches[i].distance < 4 * min_dist)
		{
			//-- Get the keypoints for the good matches
			objectGoodKeypoints.push_back(pattern._keypoints[matches[i].queryIdx].pt);
			sceneGoodKeypoints.push_back(scene._keypoints[matches[i].trainIdx].pt);
		}
	}

	// Apply transform to scene image to clip the detected pattern position
	cv::Mat patternFromScene;
	cv::Mat inversedHomography = cv::findHomography(sceneGoodKeypoints,
		objectGoodKeypoints, cv::RANSAC);
	warpPerspective(scene._image,
		patternFromScene,
		inversedHomography,
		cv::Size(pattern._image.cols, pattern._image.rows));

	// Compute correlation of original pattern image with patternFromScene
	cv::Mat isDetected;
	cv::matchTemplate(patternFromScene, pattern._image, isDetected, cv::TM_CCOEFF_NORMED);

	// Is there a positive detection
	double detectionConfidence = (isDetected.at<float>(0, 0) > DETECTION_CORRELATION_THRESHOLD) ?
		pattern.increaseDetectionConfidence() :
		pattern.decreaseDetectionConfidence();


	// Get image position in frame
	cv::Mat homography = findHomography(objectGoodKeypoints, sceneGoodKeypoints, cv::RANSAC);
	std::vector<cv::Point2f> obj_corners(4);
	obj_corners[0] = cvPoint(0, 0);
	obj_corners[1] = cvPoint(pattern._image.cols, 0);
	obj_corners[2] = cvPoint(pattern._image.cols, pattern._image.rows);
	obj_corners[3] = cvPoint(0, pattern._image.rows);
	std::vector<cv::Point2f> scene_corners(4);
	perspectiveTransform(obj_corners, scene_corners, homography);

	if (detectionConfidence > Pattern::DETECTION_CONFIDENCE_THRESHOLD 
			&& isDetected.at<float>(0, 0) > DETECTION_CORRELATION_THRESHOLD) {
		cv::Mat patternCenter;
		reduce(scene_corners, patternCenter, 2, CV_REDUCE_AVG);
		// convert from Mat to Point - there may be even a simpler conversion,
		// but I do not know about it.
		im.position = Point2f(patternCenter.at<float>(0, 0), patternCenter.at<float>(0, 1));
	}


	//	void detectObject(String name, Mat& frame, Mat& img_scene, Mat& descriptors_scene, std::vector<KeyPoint>& keypoints_scene, Image &im) {
	//
	//		//-- Step 2: Matching descriptor vectors using FLANN matcher
	//		FlannBasedMatcher matcher;
	//		std::vector< DMatch > matches;
	//
	//		matcher.match(im.descriptors_object, descriptors_scene, matches);
	//
	//		double max_dist = 0; double min_dist = 100;
	//
	//		//-- Quick calculation of max and min distances between keypoints
	//		for (int i = 0; i < im.descriptors_object.rows; i++)
	//		{
	//			double dist = matches[i].distance;
	//			if (dist < min_dist) min_dist = dist;
	//			if (dist > max_dist) max_dist = dist;
	//		}
	//
	//		//-- Localize the object
	//		std::vector< DMatch > good_matches;
	//		for (int i = 0; i < im.descriptors_object.rows; i++)
	//		{
	//			if (matches[i].distance < 4 * min_dist)
	//			{
	//				good_matches.push_back(matches[i]);
	//			}
	//		}
	//
	//		std::vector<Point2f> obj;
	//		std::vector<Point2f> scene;
	//
	//		for (size_t i = 0; i < good_matches.size(); i++)
	//		{
	//			//-- Get the keypoints from the good matches
	//			obj.push_back(im.keypoints_object[good_matches[i].queryIdx].pt);
	//			scene.push_back(keypoints_scene[good_matches[i].trainIdx].pt);
	//		}
	//
	//		Mat Hinv = findHomography(scene, obj, RANSAC);
	//		Mat img_corrected;
	//		warpPerspective(img_scene, img_corrected, Hinv, Size(im.img.cols, im.img.rows));
	//
	//		Mat isDetected;
	//		matchTemplate(img_corrected, im.img, isDetected, TM_CCOEFF_NORMED);
	//
	//		float currentDetected = 0.0;
	//		if (isDetected.at<float>(0, 0) > THRESHOLD) { currentDetected = 1.0; }
	//		im.detected = (1.0 - STEP) * im.detected + STEP * currentDetected;
	//
	//		// Get image position in frame
	//		Mat H = findHomography(obj, scene, RANSAC);
	//		std::vector<Point2f> obj_corners(4);
	//		obj_corners[0] = cvPoint(0, 0);
	//		obj_corners[1] = cvPoint(im.img.cols, 0);
	//		obj_corners[2] = cvPoint(im.img.cols, im.img.rows);
	//		obj_corners[3] = cvPoint(0, im.img.rows);
	//		std::vector<Point2f> scene_corners(4);
	//		perspectiveTransform(obj_corners, scene_corners, H);
	//
	//		if (im.detected > 0.5 && isDetected.at<float>(0, 0) > THRESHOLD) {
	//			Mat mean_;
	//			reduce(scene_corners, mean_, 2, CV_REDUCE_AVG);
	//			// convert from Mat to Point - there may be even a simpler conversion,
	//			// but I do not know about it.
	//			im.position = Point2f(mean_.at<float>(0, 0), mean_.at<float>(0, 1));
	//		}
	//		// std::cout << name << " - " << im.detected << " " << isDetected.at<float>(0,0) << std::endl;
	//
	//#ifdef DISPLAY
	//		// Display found image
	//		//-- Get the corners from the image_1 ( the object to be "detected" )
	//
	//		Scalar color = (im.detected > 0.5) ? Scalar(0, 255, 0) : Scalar(0, 0, 255);
	//		Mat img_corrected_colored;
	//		cvtColor(img_corrected, img_corrected_colored, COLOR_GRAY2BGR);
	//
	//		line(img_corrected_colored, obj_corners[0], obj_corners[1], color, 4);
	//		line(img_corrected_colored, obj_corners[1], obj_corners[2], color, 4);
	//		line(img_corrected_colored, obj_corners[2], obj_corners[3], color, 4);
	//		line(img_corrected_colored, obj_corners[3], obj_corners[0], color, 4);
	//#ifdef DISPLAY_SMALL
	//		imshow(name, img_corrected_colored);
	//#endif
	//
	//		// Update main frame
	//		// Display points in the frame
	//		for (int i = 0; i< scene.size(); i++) {
	//			circle(frame, scene[i], 10, Scalar(128, 128, 128));
	//		}
	//
	//		if (im.detected > 0.5 && isDetected.at<float>(0, 0)) {
	//			line(frame, scene_corners[0], scene_corners[1], color, 4);
	//			line(frame, scene_corners[1], scene_corners[2], color, 4);
	//			line(frame, scene_corners[2], scene_corners[3], color, 4);
	//			line(frame, scene_corners[3], scene_corners[0], color, 4);
	//		}
	//		if (im.detected > 0.5) {
	//			circle(frame, im.position, 5, color, 3);
	//		}
	//
	//#endif
	//	}
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
		LOG(_logger, "Acquiring a new frame.");
		cv::Mat frame;
		_camera->retrieve(frame);

		SURFDetectable currentFrame(frame);

		// 2. Compute keypoints and descriptor for the frame
		_computeKeypointsAndDescriptors(currentFrame);

		// 3. Search for all registered patterns in the capture frame.
		unique_lock<mutex> lockPatternMutex(_patternsMutex);
		LOG(_logger, "Detecting " << (int)_patterns.size() << " patterns in the frame.");
		for (auto it = _patterns.begin(); it != _patterns.end(); it++) {
			Pattern & pattern = *it;
			LOG(_logger, "Detecting patterns " + pattern.getName());

			// 3.1 Match 
			_matchPatternInFrame(pattern, currentFrame);

			// 3.2 Update the status and position of the detected patterns.
		}
	} while (!_exitDetectionLoop);
}

bool VisionEngine::doesExitDetectionLoop() const
{
	return _exitDetectionLoop;
}

