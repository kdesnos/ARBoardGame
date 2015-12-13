
#include <iostream>

#include <QApplication>


#include "launcherdialog.hpp"


/* @function main */
int main( int argc, char** argv )
{
    // Create a display window in full screen mode
    std::cout << "Open launcher QT window" << std::endl;

    QApplication app(argc, argv);

    LauncherDialog launcherDialog;

    launcherDialog.show();

    // TODO: put this in a separate class
    // 1. Compute the init image descriptors
    // 2. Display init image in full screen mode
    // 3. Capture image fromy camera
    // 4. If the image is not fully detected (check image borders especially)
    // 4.1 Set a smaller (-5%) new size for the init image
    // 4.2 Display the init image
    // 4.3 Go back to step 4
    // 5. Init image was fully detected: Return the detected display area

    return app.exec();
}
