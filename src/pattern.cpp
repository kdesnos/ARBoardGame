#include "arExceptions.hpp"
#include "pattern.hpp"

Pattern::Pattern(const std::string & name, const std::string & path) throw(...) :
	_name(name), SURFDetectable(cv::imread(path, cv::IMREAD_GRAYSCALE))
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

bool Pattern::operator==(const Pattern & other) const
{
	return _name == other._name;
}
