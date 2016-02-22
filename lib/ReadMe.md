# Library Setup Guideling

## OpenCV 
1. Download OpenCV v3.1.0 (or greater) pre-built binaries for Windows from http://opencv.org/
2. Extract the archive (downloaded *.exe file) in "ARBoardGame/lib/" (Requires 2.4 GB).
3. Download extra opencv modules from https://github.com/Itseez/opencv_contrib/tree/3.1.0 (download the repository as a zip file).
4. Extract files in "ARBoardGame/lib/opencv_contrib-3.1.0/..."
5. Do the following changes in downloader files:
	a. Open the following file "ARBoardGame/lib/opencv/sources/cmake/OpenCVCompilerOptions.cmake" and comment the following line
		> # add_extra_compiler_option(-Werror=non-virtual-dtor)

### OpenCV - MINGW

6. Open CMakeGUI and select "ARBoardGame/lib/opencv/sources" as the location of the source code, and "ARBoardGame/lib/opencv/build-mingw" as the location of the binaries.
7. In CMakeGUI, click on configure. This step should complete without error, otherwise, the most probable error is that CMake cannot find Mingw in the PATH environment variable.
8. Once the configuration is over, in the list set "OPENCV_EXTRA_MODULES_PATH" to the location of "xxx/ARBoardGame/lib/opencv_contrib_master/modules"
9. Press configure once again. (Should run smoothly)
10. Click "Configure" once again.
11. Click on "generate".
12. In the opened Wizard, select MinGW Makefile.
13. Go to the "ARBoardGame/lib/opencv/mingw-build/" directory using windows command line terminal.
14. Type the following command and wait for 20 minutes for the build to run to completion
	> mingw32-make
16. If you encounter any error.. Your favorite search engine will be your best friend. (and possibly StackOverflow).

### OpenCV - Visual

(from https://putuyuwono.wordpress.com/2015/04/23/building-and-installing-opencv-3-0-on-windows-7-64-bit/)
6. Open CMakeGUI and select "ARBoardGame/lib/opencv/sources" as the location of the source code, and "ARBoardGame/lib/opencv/build" as the location of the binaries.
7. In CMakeGUI, click on configure, Select Visual Studio 14 Win64. This step should complete without error, otherwise, the most probable error is that CMake cannot find Visual in the PATH environment variable.
8. Once the configuration is over, in the list set "OPENCV_EXTRA_MODULES_PATH" to the location of "xxx/ARBoardGame/lib/opencv_contrib_master/modules"
9. Press configure once again. (Should run smoothly)
10. Click "Configure" once again.
11. Click on "generate".
12. In the opened Wizard, select Visual Studi 14.
13. Go to the "ARBoardGame/lib/opencv/build/" and open the contained *.sln file.
14. Build the "CMakeTargets/INSTALL" project. (Both in Debug and Release).

## Qt 5.5 - Visual 2015 64 only
(from https://github.com/supamii/QttpServer/blob/qs_dev/WINBUILD.md and http://doc.qt.io/qt-5/windows-building.html)
1. Download QT source from http://www.qt.io/download-open-source/#section-5 (Windows source)
2. Unzip downloadeds archive in ARBoardGame/lib/qt-5.5.1
3. In ARBoardGame/lib, create a command file qt.cmd with the following content
REM Set up \Microsoft Visual Studio 2013, where <arch> is \c amd64, \c x86, etc.
CALL "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x64
SET _ROOT=D:\ARBoardGameVS15\lib\qt-5.5.1
SET PATH=%_ROOT%\qtbase\bin;%_ROOT%\gnuwin32\bin;%PATH%
REM Uncomment the below line when using a git checkout of the source repository
REM SET PATH=%_ROOT%\qtrepotools\bin;%PATH%
SET QMAKESPEC=win32-msvc2015
SET _ROOT=
4. Create a shortcut to the qt.cmd file with following settings
Target: %SystemRoot%\system32\cmd.exe /E:ON /V:ON /k <ARBOardGameAbsolutePath>\lib\qt.cmd
Execute in: <ARBOardGameAbsolutePath>\lib\qt-5.5.1
5. Launch the command line with the shortcut
6. enter the following command
> configure.bat -platform win32-msvc2015 -debug -nomake examples -opensource -skip qtwebkit -skip qtwebchannel -skip qtquick1 -skip qtdeclarative
7. Then
> nmake
8. Pray the mighty c++ compilation god for 30 minutes!

