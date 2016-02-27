/**
* This files contains the declarations of Exceptions used throughout the
* ARBoardGame project.
*/

#include <stdexcept>

/**
* FileNotFoundException should be used when failing to open a file from a path.
* \author kdesnos
*/
class FileNotFoundException : public std::runtime_error {
public:
	/**
	* Constructor of the FileNotFoundException
	* 
	* \param[in] msg the message associated to the constructed FileNotFoundException.
	*/
	FileNotFoundException(std::string msg) throw() : std::runtime_error(msg){
	}
};