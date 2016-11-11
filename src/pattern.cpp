#include "arExceptions.hpp"
#include "pattern.hpp"

const double Pattern::DETECTION_CONFIDENCE_STEP = 0.2;

const double Pattern::DETECTION_CONFIDENCE_THRESHOLD = 0.5;

Pattern::Pattern(const std::string & name, const std::string & path) throw(...) :
	_name(name), SURFDetectable(cv::imread(path, cv::IMREAD_GRAYSCALE)),
	_detectionConfidence(0.0)
{
	// Check whether the image was successfully opened during initialization.
	if (!_image.data) {
		throw FileNotFoundException("Pattern path not found:" + path );
	}
}

const std::string& Pattern::getName() const throw()
{
	return _name;
}

double Pattern::getDetectionConfidence() const
{
	return _detectionConfidence;
}

double Pattern::decreaseDetectionConfidence()
{
	_detectionConfidence -= DETECTION_CONFIDENCE_STEP;
	_detectionConfidence = (_detectionConfidence < 0.0) ? 0.0 : _detectionConfidence;
	return _detectionConfidence;
}

double Pattern::increaseDetectionConfidence()
{
	_detectionConfidence += DETECTION_CONFIDENCE_STEP;
	_detectionConfidence = (_detectionConfidence > 1.0) ? 1.0 : _detectionConfidence;
	return _detectionConfidence;
}

bool Pattern::operator==(const Pattern & other) const
{
	return _name == other._name;
}
