#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/features2d.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/calib3d.hpp"
#include "opencv2/xfeatures2d.hpp"
#include "opencv2/opencv.hpp"

#define DEBUG std::cout << "Line " << __LINE__ << std::endl

using namespace cv;
using namespace cv::xfeatures2d;

int main(int, char**)
{
    VideoCapture cap(0); // open the default camera
    //sleep(1);

    if(!cap.isOpened())  // check if we succeeded
        return -1;

    // Reference picture
    Mat img_object = imread( "D:/ARBoardGame/data/CarteToDetect2.png", IMREAD_GRAYSCALE );

    // Detect the keypoints and extract descriptors using SURF
    int minHessian = 400;
    Ptr<SURF> detector = SURF::create( minHessian );
    std::vector<KeyPoint> keypoints_object;
    Mat descriptors_object;

    detector->detectAndCompute( img_object, Mat(), keypoints_object, descriptors_object );

    if( !img_object.data )
    { std::cout<< " --(!) Error reading images " << std::endl; return -1; }

    namedWindow("edges",1);
    for(;;)
    {
        Mat frame, img_scene;
        cap >> frame; // get a new frame from camera
        cvtColor(frame, img_scene, COLOR_BGR2GRAY);

        //-- Step 1: Detect the keypoints and extract descriptors using SURF
        std::vector<KeyPoint> keypoints_scene;
        Mat descriptors_scene;
        detector->detectAndCompute( img_scene, Mat(), keypoints_scene, descriptors_scene );

        //-- Step 2: Matching descriptor vectors using FLANN matcher
        FlannBasedMatcher matcher;
        std::vector< DMatch > matches;
        try {
            matcher.match( descriptors_object, descriptors_scene, matches );
        } catch(...) {
            continue;
        }
        double max_dist = 0; double min_dist = 100;



        //-- Quick calculation of max and min distances between keypoints
        for( int i = 0; i < descriptors_object.rows; i++ )
        { double dist = matches[i].distance;
            if( dist < min_dist ) min_dist = dist;
            if( dist > max_dist ) max_dist = dist;
        }

        //-- Localize the object
        std::vector< DMatch > good_matches;
        for( int i = 0; i < descriptors_object.rows; i++ )
        { if( matches[i].distance < 4*min_dist )
            { good_matches.push_back( matches[i]); }
        }

        Mat img_matches;

        std::vector<Point2f> obj;
        std::vector<Point2f> scene;

        for( size_t i = 0; i < good_matches.size(); i++ )
        {
            //-- Get the keypoints from the good matches
            obj.push_back( keypoints_object[ good_matches[i].queryIdx ].pt );
            scene.push_back( keypoints_scene[ good_matches[i].trainIdx ].pt );
        }

        Mat Hinv = findHomography( scene, obj, RANSAC );

        Mat img_corrected;
        try {
            warpPerspective(img_scene,img_corrected,Hinv,Size(img_object.cols, img_object.rows));
        } catch(...) {
            continue;
        }

        imshow("Detection", img_corrected);

        Mat test;
        matchTemplate(img_corrected, img_object, test,TM_CCOEFF_NORMED);

        String detected = (test.at<float>(0,0) > 0.5)?"ok":"nope";
        std::cout << test.at<float>(0,0) << " - " << detected << std::endl;

 #define AFFICHE
#ifdef AFFICHE

        Mat H = findHomography( obj, scene, RANSAC );

        //-- Get the corners from the image_1 ( the object to be "detected" )
        std::vector<Point2f> obj_corners(4);
        obj_corners[0] = cvPoint(0,0);
        obj_corners[1] = cvPoint( img_object.cols, 0 );
        obj_corners[2] = cvPoint( img_object.cols, img_object.rows );
        obj_corners[3] = cvPoint( 0, img_object.rows );
        std::vector<Point2f> scene_corners(4);


        try {
            perspectiveTransform( obj_corners, scene_corners, H);
        } catch(...) {
            continue;
        }

        for(int i =0; i< scene.size(); i++){
            circle(img_scene, scene[i], 10,  Scalar(0, 255, 0));
        }

        line( img_scene, scene_corners[0] , scene_corners[1], Scalar(0, 255, 0), 4 );
        line( img_scene, scene_corners[1], scene_corners[2], Scalar( 0, 255, 0), 4 );
        line( img_scene, scene_corners[2], scene_corners[3], Scalar( 0, 255, 0), 4 );
        line( img_scene, scene_corners[3], scene_corners[0], Scalar( 0, 255, 0), 4 );

        imshow("edges", img_scene);
#endif


        if(waitKey(30) >= 0) break;


        //-- Draw lines between the corners (the mapped object in the scene - image_2 )
        // line( img_matches, scene_corners[0] + Point2f( img_object.cols, 0), scene_corners[1] + Point2f( img_object.cols, 0), Scalar(0, 255, 0), 4 );
        // line( img_matches, scene_corners[1] + Point2f( img_object.cols, 0), scene_corners[2] + Point2f( img_object.cols, 0), Scalar( 0, 255, 0), 4 );
        // line( img_matches, scene_corners[2] + Point2f( img_object.cols, 0), scene_corners[3] + Point2f( img_object.cols, 0), Scalar( 0, 255, 0), 4 );
        // line( img_matches, scene_corners[3] + Point2f( img_object.cols, 0), scene_corners[0] + Point2f( img_object.cols, 0), Scalar( 0, 255, 0), 4 );



        // Do inverse transform for points
        // Mat h_inv = H.inv();

        //        std::vector<Point2f> scene_keypoints_inverted;
        //        Mat invIMG;
        //        try {
        //            warpPerspective( img_scene, invIMG, H,  Size(1000,1000) );
        //        } catch (...) {
        //            continue;
        //        }

        //        std::vector<Point2f> scene_keypoints_inverted;
        //        Mat invIMG;
        //        try {
        //            perspectiveTransform(scene, scene_keypoints_inverted, H);
        //        } catch (...) {
        //            continue;
        //        }


        //        std::cout << "pts"<<std::endl;
        //        for(int i = 0; i< scene.size(); i++){
        //            std::cout << " " << scene[i];
        //        }
        //        std::cout << std::endl;

        //        // Count points in
        //        int nb =0;
        //        for(int i = 0; i< scene_keypoints_inverted.size(); i++){
        //            Point2f pt = scene_keypoints_inverted[i];
        //            if(pt.x > obj_corners[0].x && pt.x < obj_corners[2].x && pt.y > obj_corners[0].y && pt.y < obj_corners[2].y) {
        //                nb++;
        //            }
        //            std::cout << " " << scene_keypoints_inverted[i];
        //        }
        //        std::cout << std::endl << "NB pts in = " << nb << "/" << scene_keypoints_inverted.size() << std::endl;




    }
    // the camera will be deinitialized automatically in VideoCapture destructor
    return 0;
}



/*
#include <iostream>

#include <QApplication>


#include "launcherdialog.hpp"


// @function main
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
*/
