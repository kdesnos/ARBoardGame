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
#include <windows.h>

//#define DISPLAY
#define THRESHOLD 0.4
#define STEP 0.2
#define SCREENWIDTH 900
#define SCREENHEIGHT 900

#define DEBUG std::cout << "Line " << __LINE__ << std::endl
#define POL_TO_X(w, r) ((r)*cos((w)))
#define POL_TO_Y(w, r) ((r)*sin((w)))

using namespace cv;
using namespace cv::xfeatures2d;

#define NB_WEAPONS 8
#define ROOTPATH "D:/ARBoardGame/data/"
String image_paths[][2] = {
    {"blaster", ROOTPATH "blaster.png"},
    {"G1",      ROOTPATH "gatling1.png"},
    {"G2",      ROOTPATH "gatling2.png"},
    {"BR",      ROOTPATH "batteringram.png"},
    {"Bow",     ROOTPATH "bow.png"},
    {"CB",      ROOTPATH "crossbow.png"},
    {"FG",      ROOTPATH "flaregun.png"},
    {"Mine",    ROOTPATH "landmine.png"}
};

typedef struct Image {
    String name;
    Mat img;
    Mat descriptors_object;
    std::vector<KeyPoint> keypoints_object;
    float detected;
    Point2f position;
} Image;

typedef struct Entity {
    String name;
    Mat img;
    float x;
    float y;
    float hp;
    float speed;
    Entity(String _name){ name = _name; x = 0.0; y = 0.0; hp = 100.0; speed = 0.03; }
} Entity;

typedef struct Weapon {
    int slot;
    float built;
    float dmgAir;
    float dmgMagic;
    float dmgGround;
    float dps;
    float angle;
    float range;
    Weapon(){ slot = 0; built = 0.0; dmgAir = 0.33; dmgMagic = 0.33; dmgGround = 0.33; dps = 1.0; angle = M_PI/2; range = 0.5;}
} Weapon;



// GLOBAL VARS

int startTime = GetTickCount();
int lastTime = startTime;
std::vector<Entity> ents;
std::vector<Weapon> weaps(NB_WEAPONS);



float getRand()
{
    return (rand()%1000)/1000.0;
}

void populateGame()
{
    // VIRGIN
    ents.push_back(Entity("virgin"));
    ents.back().x = 0.0;
    ents.back().y = 0.0;
    ents.back().img = imread( ROOTPATH "virgin.png", IMREAD_COLOR );

    // SLOTS
    float radius = 0.15;
    float angle = M_PI/6.0;
    ents.push_back(Entity("slot1"));
    ents.back().x = POL_TO_X(angle, radius);
    ents.back().y = POL_TO_Y(angle, radius);
    ents.back().img = imread( ROOTPATH "slot1.png", IMREAD_COLOR );
    angle += M_PI/3.0;
    ents.push_back(Entity("slot1"));
    ents.back().x = POL_TO_X(angle, radius);
    ents.back().y = POL_TO_Y(angle, radius);
    ents.back().img = imread( ROOTPATH "slot2.png", IMREAD_COLOR );
    angle += M_PI/3.0;
    ents.push_back(Entity("slot1"));
    ents.back().x = POL_TO_X(angle, radius);
    ents.back().y = POL_TO_Y(angle, radius);
    ents.back().img = imread( ROOTPATH "slot3.png", IMREAD_COLOR );
    angle += M_PI/3.0;
    ents.push_back(Entity("slot1"));
    ents.back().x = POL_TO_X(angle, radius);
    ents.back().y = POL_TO_Y(angle, radius);
    ents.back().img = imread( ROOTPATH "slot4.png", IMREAD_COLOR );
    angle += M_PI/3.0;
    ents.push_back(Entity("slot1"));
    ents.back().x = POL_TO_X(angle, radius);
    ents.back().y = POL_TO_Y(angle, radius);
    ents.back().img = imread( ROOTPATH "slot5.png", IMREAD_COLOR );
    angle += M_PI/3.0;
    ents.push_back(Entity("slot1"));
    ents.back().x = POL_TO_X(angle, radius);
    ents.back().y = POL_TO_Y(angle, radius);
    ents.back().img = imread( ROOTPATH "slot6.png", IMREAD_COLOR );

}

void spawnEnnemies(float totalTime)
{
    if(getRand()<0.1){
        float radius = 0.9;
        float angle = -M_PI + getRand()*M_PI*2.0;
        ents.push_back(Entity("roucoups"));
        ents.back().x = POL_TO_X(angle, radius);
        ents.back().y = POL_TO_Y(angle, radius);
        ents.back().img = imread( ROOTPATH "roucoups.png", IMREAD_COLOR );
    }
}

void drawImage(Mat &src, Mat &dst, float x, float y)
{
    int px = ( x / 2.0 + 0.5) * SCREENWIDTH  - src.cols / 2.0;
    int py = (-y / 2.0 + 0.5) * SCREENHEIGHT - src.rows / 2.0;
    src.copyTo(dst(Rect(px, py, src.cols, src.rows)));
}

void drawEntities(Mat &dst)
{
    for(unsigned int i=0; i<ents.size(); i++){
        drawImage(ents.at(i).img, dst, ents.at(i).x, ents.at(i).y);
    }
}

void moveEnemies(float deltaTime)
{
    for(unsigned int i=7; i<ents.size(); i++){
        float x = ents.at(i).x;
        float y = ents.at(i).y;
        float n = sqrt(x*x+y*y);
        float newn = n - ents.at(i).speed*deltaTime;
        ents.at(i).x = x * (newn/n);
        ents.at(i).y = y * (newn/n);
    }
}



// GAMEPLAY

void gameplay(std::vector<Image> &images)
{
    int thisTime = GetTickCount();
    float deltaTime = (thisTime-lastTime)/1000.0;
    float totalTime = (thisTime-startTime)/1000.0;

    for(unsigned int i=0; i<images.size(); i++){
        std::cout << images.at(i).name << ": " << images.at(i).detected << std::endl;
    }
    std::cout << "deltaTime: " << deltaTime << " s\n" << std::endl;

    // Create green background
    Mat display(SCREENHEIGHT, SCREENWIDTH, CV_8UC3);
    display = cv::Scalar(100,200,0);

    spawnEnnemies(totalTime);
    moveEnemies(deltaTime);
    drawEntities(display);

    imshow("display", display);
    lastTime = thisTime;
}



// MAIN, etc.

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

    float currentDetected = 0.0;
    if (isDetected.at<float>(0,0) > THRESHOLD){ currentDetected = 1.0; }
    im.detected = (1.0 - STEP) * im.detected + STEP * currentDetected;

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



// MAIN

int main(int, char**)
{
    VideoCapture cap(0); // open the default camera
    //sleep(1);

    if(!cap.isOpened())  // check if we succeeded
        return -1;

    populateGame();

    int minHessian = 400;
    Ptr<SURF> detector = SURF::create( minHessian );
    std::vector<Image> images;
    for(int i =0; i< NB_WEAPONS; i++){
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
#ifdef DISPLAY
        // display frame with detected objects
        imshow("edges", frame);
#endif

        gameplay(images);

        if(waitKey(30) >= 0) break;
    }

    return 0;
}
