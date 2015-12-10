# AR Board Game 

## Project description
*AR Board Game* is a project aiming at creating a software development kit to ease the development of board games with augmented reality features.

## Compilation guidelines

1. Make sure QTCreator, CMake, MinGW is installed on your computer
2. Open QTCreator
3. In the menu bar, select "File/Open File or Project"
4. In the Wizard, select "ARBoardGame/src/CMakeFile.txt" and click open.
5. In the project view, right-click on the project and select "Run CMake".
6. In the wizard, click on "Run CMake".
7. Once the CMake project generation is completed (without errors), click on Finish.
8. Your project is ready to be compiled and run through QTCreator buttons.

The CMake compilation step only need to be repeated manually if major changes in the project organization were applied (such as the addition of a new third-party library).

The default build configuration of the project is for a debug versio,. To compile a release version, open CMakeLists.txt and set DEBUG to 0 at line 14. 

## Repository content

* *bin* - Should contain nothing on this repository. (only commited for developer convenience).
* *doc* - Documentation on the project.
* *lib* - Library and/or guide to ease third-party library installation.
* *src* - C++ source code of the project/
* *ReadMe.md* - This file.

