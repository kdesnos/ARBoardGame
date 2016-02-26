/**
* This files contains the declarations of Exceptions used throughout the
* ARBoardGame project.
*/

#include <stdexcept>

class FileNotFoundException : public std::runtime_error {
public:
	FileNotFoundException(std::string msg) throw() : std::runtime_error(msg){
	}
};