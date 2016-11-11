/**
* \brief This file contains the definition of the VisionEngine Class
*/
#pragma once

#include <iostream>
#include <list>
#include <atomic>
#include <mutex>

#include "opencv2/opencv.hpp"
#include "opencv2/xfeatures2d.hpp"

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

	/**
	* Mutex used to prevent concurrent accesses and modifications of the _patterns
	* attribute.
	*/
	std::mutex _patternsMutex;

	/**
	* Minimum Hessian distance used by the SURF _detector.
	* After a few experiments, 400 was found to be a good value.
	* But it is still a magic number that should be refined.
	*/
	static const int MIN_HESSIAN = 400;

	/**
	* Detection threshold used in the detection loop.
	* A pattern is detected if the correllation calculus between the pattern
	* image and the pattern extracted from a frame is higher than the given 
	* threshold.
	*/
	static const int DETECTION_CORRELATION_THRESHOLD = 0.4;

	/**
	* SURF detector used by the VisionEngine
	*/
	cv::Ptr<cv::xfeatures2d::SURF> _detector;

	/**
	* This method uses the _descriptor of the VisionEngine to compute keypoints
	* and descriptors of the given SURFDetectable object.
	*
	* \param[in,out] detectable the SURFDetectable object whose keypoints and
	* descriptors are computed.
	*
	*/
	void _computeKeypointsAndDescriptors(SURFDetectable & detectable) const;

	/**
	* This method matches the given pattern in the given scene.
	*
	* When succeeding, the position and detection attributes of the pattern 
	* are updated.
	*/
	void _matchPatternInFrame(Pattern & pattern, SURFDetectable & scene) const;

	/**
	* This boolean value is used to exit the detectionLoop
	* when the value of this variable is set to true.
	* If this boolean value is set to true before calling the detectionLoop,
	* the loop will execute once and exit (do {} while(_exitDetectionLoop)).
	*/
	atomic_bool _exitDetectionLoop;


	/**
	* Thread used to run the detectionLoop when startDetection loop is called.
	*/
	std::thread * _detectionLoopThread;

	/**
	* Mutex used to prevent multiple launches of the detection loop in parallel.
	*/
	std::mutex _detectionLoopLaunchMutex;

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
	* The method will wait for the _patternMutex to be unlocked to actually
	* modify the _pattern attribute.
	*
	* \param[in] pattern a reference to the Pattern to detect.
	*
	* \return true if the registeration was successful (or was already done
	* before), false otherwise. Possible reasons for failing: none listed yet.
	*/
	bool registerPattern(Pattern & pattern);

	/**
	* Execute the detectionLoop() once in the current thread.
	*
	* \return true if the thread was successfully started, false otherwise.
	* Possible reason for failing:
	* - detectionLoop is already running.
	* - there is a concurrent attempt to launch the loop.
	* - visionEngine is not initialized.
	*/
	bool executeDetectionLoopOnce();

	/**
	* Start the detectionLoop method in a dedicated separate std::thread.
	*
	* This method sets _exitDetectionLoop to false;
	*
	* \return true if the thread was successfully started, false otherwise.
	* Possible reason for failing:
	* - detectionLoop is already running.
	* - there is a concurrent attempt to launch the loop.
	* - visionEngine is not initialized.
	*/
	bool startDetectionThread();

	/**
	* Stop the previously launched detectionThread.
	*
	* This method will set _exitDetectionLoop to true, and wait for thread
	* completion.
	*
	* \return true if the thread was successfully terminated, false otherwise.
	* Possible reasons for failing:
	* - No thread was running.
	*/
	bool stopDetectionThread();

	/**
	* Unregister a Pattern from detection by the VisionEngine in frames captured
	* by the _camera.
	*
	* The method will wait for the _patternMutex to be unlocked to actually
	* modify the _pattern attribute.
	*
	* \param[in] pattern a reference to the Pattern whose detection is no
	* longer wanted.
	*
	* \return true if the unregisteration was successful false otherwise.
	* Possible reasons for failing: pattern is not in the list _patterns to
	* detect.
	*/
	bool unregisterPattern(const Pattern & pattern);

private:
	/**
	* Mutex used to know if the detectionLoop() is currently running in any
	* thread.
	*/
	std::mutex _detectionLoopMutex;

	/**
	* Set a new value for the _exitDetectionLoop attribute.
	*
	* \param[in] exit the new value for the _exitDetectionLoop attribute.
	*/
	void setExitDetectionLoop(const bool exit);


	/**
	* Retrieve the current value of the _exitDetectionLoop attribute.
	*
	* \return the boolean value of _exitDetectionLoop.
	*/
	bool doesExitDetectionLoop() const;

	/**
	* Detection loop of the VisionEngine.
	* This loop iteratively:
	* - acquires a new frame from the camera.
	* - search for all registered patterns in the capture frame.
	* - update the status and position of the detected patterns.
	*
	* Concurrent call to this method are prevented with the
	* _detectionLoopMutex. Consequently, this method may block if the
	* mutex is already locked by another thread. This method is
	* called using startDetectionThread() or executeDetectionLoop().
	*
	* The method will wait for the _patternMutex to be unlocked iterate on the
	* the patterns to detect. (and will lock it during iterations).
	*
	* The loop of this method is an infinite loop that can only be broken by
	* setting the _exitDetectionLoop to true.
	* The method will immediately terminate if it is called on a unitialized
	* VisionEngine.
	*/
	void detectionLoop();
};