# Library Setup Guideling

## OpenCV 

1. Download OpenCV v3.0.0 (or greater) pre-built binaries for Windows from http://opencv.org/
2. Extract the archive (downloaded *.exe file) in "ARBoardGame/lib/" (Requires 2.4 GB).
3. Open the following file "ARBoardGame/lib/opencv/sources/cmake/OpenCVCompilerOptions.cmake" and comment the following line
	> # add_extra_compiler_option(-Werror=non-virtual-dtor) 
4. Open CMakeGUI and select "ARBoardGame/lib/opencv/sources" as the location of the source code, and "ARBoardGame/lib/opencv/build-mingw" as the location of the binaries.
5. In CMakeGUI, click on configure. This step should complete without error, otherwise, the most probable error is that CMake cannot find Mingw in the PATH environment variable.
6. Once the configuration is over, in the list of displayed variable, deactivate "WITH_IPP".
7. Click "Configure" once again, this will end with an error that you can ignore.
8. Click on "generate".
9. In the opened Wizard, select MinGW Makefile.
10. Go to the "ARBoardGame/lib/opencv/mingw-build/" directory using windows command line terminal.
11. Type the following command and wait for 20 minutes for the build to run to completion
	> mingw32-make
12. If you encounter any error.. Your favorite search engine will be your best friend. (and possibly StackOverflow).