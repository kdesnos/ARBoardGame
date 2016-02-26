#include "arExceptions.hpp"
#include "pattern.hpp"

Pattern::Pattern(std::string path)
{
	image = cv::imread(path);

	if (!image.data) {
		throw new FileNotFoundException("Pattern path not found:" + path );
	}
}
