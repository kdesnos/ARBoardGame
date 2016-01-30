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

#define THRESHOLD 0.4
#define STEP 0.2

#define NB_IMAGE_PATHS 8
String image_paths[][2] = {
    {"blaster","D:/ARBoardGame/data/blaster.png"},
    {"G1","D:/ARBoardGame/data/gatling1.png"},
    {"G2","D:/ARBoardGame/data/gatling2.png"},
    {"BR","D:/ARBoardGame/data/batteringram.png"},
    {"Bow","D:/ARBoardGame/data/bow.png"},
    {"CB","D:/ARBoardGame/data/crossbow.png"},
    {"FG","D:/ARBoardGame/data/flaregun.png"},
    {"Mine","D:/ARBoardGame/data/landmine.png"}
};

typedef struct Image {
    String name;
    Mat img;
    Mat descriptors_object;
    std::vector<KeyPoint> keypoints_object;
    float detected;
    Point2f position;
} Image;

void detectObject(String name, Mat& frame, Mat& img_scene, Mat& descriptors_scene, std::vector<KeyPoint>& keypoints_scene, Image &im){

    //-- Step 2: Matching descriptor vectors using FLANN matcher
    FlannBasedMatcher matcher;
    std::vector< DMatch > matches;

    matcher.match( im.descriptors_object, descriptors_scene, matches );

    double max_dist = 0; double min_dist = 100;

    //-- Quick calculation of max and min distances between keypoints
    for( int i = 0; i < im.descriptors_object.rows; i++ )
    { double dist = matches[i].distance;
        if( dist < min_dist ) min_dist = dist;
        if( dist > max_dist ) max_dist = dist;
    }

    //-- Localize the object
    std::vector< DMatch > good_matches;
    for( int i = 0; i < im.descriptors_object.rows; i++ )
    { if( matches[i].distance < 4*min_dist )
        { good_matches.push_back( matches[i]); }
    }

    std::vector<Point2f> obj;
    std::vector<Point2f> scene;

    for( size_t i = 0; i < good_matches.size(); i++ )
    {
        //-- Get the keypoints from the good matches
        obj.push_back( im.keypoints_object[ good_matches[i].queryIdx ].pt );
        scene.push_back( keypoints_scene[ good_matches[i].trainIdx ].pt );
    }

    Mat Hinv = findHomography( scene, obj, RANSAC );
    Mat img_corrected;
    warpPerspective(img_scene,img_corrected,Hinv,Size(im.img.cols, im.img.rows));

    Mat isDetected;
    matchTemplate(img_corrected, im.img, isDetected,TM_CCOEFF_NORMED);

    if (isDetected.at<float>(0,0) > THRESHOLD){
        if(im.detected<1.0 ) im.detected+=STEP;
    } else {
        if(im.detected>0.0) im.detected-=STEP;
    }

    // Get image position in frame
    Mat H = findHomography( obj, scene, RANSAC );
    std::vector<Point2f> obj_corners(4);
    obj_corners[0] = cvPoint(0,0);
    obj_corners[1] = cvPoint( im.img.cols, 0 );
    obj_corners[2] = cvPoint( im.img.cols, im.img.rows );
    obj_corners[3] = cvPoint( 0, im.img.rows );
    std::vector<Point2f> scene_corners(4);
    perspectiveTransform( obj_corners, scene_corners, H);

    if(im.detected > 0.5 && isDetected.at<float>(0,0) > THRESHOLD){
        Mat mean_;
        reduce(scene_corners, mean_, 2, CV_REDUCE_AVG);
        // convert from Mat to Point - there may be even a simpler conversion,
        // but I do not know about it.
        im.position =  Point2f(mean_.at<float>(0,0), mean_.at<float>(0,1));
    }
    // std::cout << name << " - " << im.detected << " " << isDetected.at<float>(0,0) << std::endl;

#define DISPLAY
#ifdef DISPLAY
    // Display found image
    //-- Get the corners from the image_1 ( the object to be "detected" )

    Scalar color = (im.detected > 0.5)? Scalar(0, 255, 0): Scalar(0, 0, 255);
    Mat img_corrected_colored;
    cvtColor(img_corrected, img_corrected_colored, COLOR_GRAY2BGR);

    line( img_corrected_colored, obj_corners[0] , obj_corners[1], color, 4 );
    line( img_corrected_colored, obj_corners[1], obj_corners[2], color, 4 );
    line( img_corrected_colored, obj_corners[2], obj_corners[3], color, 4 );
    line( img_corrected_colored, obj_corners[3], obj_corners[0], color, 4 );
    imshow(name, img_corrected_colored);

    // Update main frame
    // Display points in the frame
    for(int i =0; i< scene.size(); i++){
        circle(frame, scene[i], 10,  Scalar(128, 128, 128));
    }

    if (im.detected > 0.5 && isDetected.at<float>(0,0)) {
        line( frame, scene_corners[0] , scene_corners[1], color, 4 );
        line( frame, scene_corners[1], scene_corners[2], color, 4 );
        line( frame, scene_corners[2], scene_corners[3], color, 4 );
        line( frame, scene_corners[3], scene_corners[0], color, 4 );
    }
    if (im.detected > 0.5) {
        circle(frame, im.position, 5, color, 3);
    }

#endif
}

int main(int, char**)
{
    VideoCapture cap(0); // open the default camera
    //sleep(1);

    if(!cap.isOpened())  // check if we succeeded
        return -1;

    int minHessian = 400;
    Ptr<SURF> detector = SURF::create( minHessian );
    std::vector<Image> images;
    for(int i =0; i< NB_IMAGE_PATHS; i++){
        Image im;

        // Reference picture
        im.detected = 0.0;
        im.name = image_paths[i][0];
        im.img = imread( image_paths[i][1], IMREAD_GRAYSCALE );

        if( !im.img.data )
        {
            std::cout<< " --(!) Error reading image: " << image_paths[i] << std::endl;
            continue;
        }

        // Detect the keypoints and extract descriptors using SURF
        detector->detectAndCompute( im.img, Mat(), im.keypoints_object, im.descriptors_object );
        images.push_back(im);
    }


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

        for(int i = 0; i< images.size();i++){
            try{
                detectObject(images.at(i).name, frame, img_scene, descriptors_scene, keypoints_scene, images.at(i));
            } catch(...){
                continue;
            }
        }

        // display frame with detected objects
        imshow("edges", frame);


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
