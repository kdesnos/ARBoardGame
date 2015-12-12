# Library Setup Guideling

## OpenCV 

1. Download OpenCV v3.0.0 (or greater) pre-built binaries for Windows from http://opencv.org/
2. Extract the archive (downloaded *.exe file) in "ARBoardGame/lib/" (Requires 2.4 GB).
3. Download extra opencv modules from https://github.com/Itseez/opencv_contrib/tree/3.0.0 (download the repository as a zip file).
4. Extract files in "ARBoardGame/lib/opencv_contrib-3.0.0/..."
5. Do the following changes in downloader files:
	a. Open the following file "ARBoardGame/lib/opencv_contrib-3.0.0/mudules/datasets/src/utils.cpp" and add the following line at the beginning
		> #undef __GNUC__
	b. Open the following file "ARBoardGame/lib/opencv_contrib-3.0.0/modules/line_descriptor/src/bitops.hpp" and replace
		> #ifdef _WIN32
		> # include <intrin.h>
		> # define popcnt __builtin_popcount
		> # pragma warning( disable : 4267 )
		> #else
		> # define popcnt __builtin_popcount
		> #endif
		with
		> # define popcnt __builtin_popcount
	c. Open the following file "ARBoardGame/lib/opencv/sources/cmake/OpenCVCompilerOptions.cmake" and comment the following line
		> # add_extra_compiler_option(-Werror=non-virtual-dtor)
6. Open CMakeGUI and select "ARBoardGame/lib/opencv/sources" as the location of the source code, and "ARBoardGame/lib/opencv/build-mingw" as the location of the binaries.
7. In CMakeGUI, click on configure. This step should complete without error, otherwise, the most probable error is that CMake cannot find Mingw in the PATH environment variable.
8. Once the configuration is over, in the list set "OPENCV_EXTRA_MODULES_PATH" to the location of "xxx/ARBoardGame/lib/opencv_contrib_master/modules"
9. Press configure once again. (Should run smoothly)
10. Once the configuration is over, in the list of displayed variable, deactivate "WITH_IPP".
11. Click "Configure" once again, this will end with an error that you can ignore.
12. Click on "generate".
13. In the opened Wizard, select MinGW Makefile.
14. Go to the "ARBoardGame/lib/opencv/mingw-build/" directory using windows command line terminal.
15. Type the following command and wait for 20 minutes for the build to run to completion
	> mingw32-make
16. If you encounter any error.. Your favorite search engine will be your best friend. (and possibly StackOverflow).