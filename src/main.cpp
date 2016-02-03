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

#define DISPLAY
//#define DISPLAY_CAM
//#define DISPLAY_SMALL
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



// TOOLS

float getRand()
{
    return (rand()%1000)/1000.0;
}

float getRandAngle(){
    return -M_PI + getRand()*M_PI*2.0;
}



// STRUCTS

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
    float w;
    float r;
    Entity(String _name){ name = _name; w = 0.0; r = 0.1; }
} Entity;

typedef struct Ennemy {
    String name;
    Mat img;
    float r;
    float w;
    float hp;
    float dr;
    float dw;
    Ennemy(String _name){ name = _name; r = 0.9; w = getRandAngle(); hp = 50.0 + 100.0*getRand(); dr = 0.05+0.03*getRand(); dw = 0.0; }
} Ennemy;

typedef struct Weapon {
    float dps;
    float angle;
    float range;
    Weapon(){ dps = 30.0; angle = getRandAngle()*0.5; range = 0.3 + 0.5 * getRand(); }
} Weapon;



// GLOBAL VARS

int startTime = GetTickCount();
int lastTime = startTime;
std::vector<Image> images;
std::vector<Entity> ents;
std::vector<Ennemy> ennemies;
std::vector<Weapon> weaps;
Mat frame;
Mat display(SCREENHEIGHT, SCREENWIDTH, CV_8UC3);



// METHODS

int getSlot(float x, float y)
{
    int slot = (y / frame.rows) > 0.5 ? 3 : 0;
    slot += (x / frame.cols) > 0.66 ? 3 : ((x / frame.cols) > 0.33 ? 2 : 1);
    int lut[] = {0, 6, 5, 4, 1, 2, 3};
    return lut[slot];
}

void populateGame()
{
    // VIRGIN
    ents.push_back(Entity("virgin"));
    ents.back().w = 0.0;
    ents.back().r = 0.0;
    ents.back().img = imread( ROOTPATH "virgin.png", IMREAD_COLOR );

    // SLOTS
    float radius = 0.15;
    float angle = M_PI/6.0;
    ents.push_back(Entity("slot1"));
    ents.back().w = angle;
    ents.back().r = radius;
    ents.back().img = imread( ROOTPATH "slot1.png", IMREAD_COLOR );
    angle += M_PI/3.0;
    ents.push_back(Entity("slot1"));
    ents.back().w = angle;
    ents.back().r = radius;
    ents.back().img = imread( ROOTPATH "slot2.png", IMREAD_COLOR );
    angle += M_PI/3.0;
    ents.push_back(Entity("slot1"));
    ents.back().w = angle;
    ents.back().r = radius;
    ents.back().img = imread( ROOTPATH "slot3.png", IMREAD_COLOR );
    angle += M_PI/3.0;
    ents.push_back(Entity("slot1"));
    ents.back().w = angle;
    ents.back().r = radius;
    ents.back().img = imread( ROOTPATH "slot4.png", IMREAD_COLOR );
    angle += M_PI/3.0;
    ents.push_back(Entity("slot1"));
    ents.back().w = angle;
    ents.back().r = radius;
    ents.back().img = imread( ROOTPATH "slot5.png", IMREAD_COLOR );
    angle += M_PI/3.0;
    ents.push_back(Entity("slot1"));
    ents.back().w = angle;
    ents.back().r = radius;
    ents.back().img = imread( ROOTPATH "slot6.png", IMREAD_COLOR );

    // WEAPONS
    for(unsigned int i=0; i<NB_WEAPONS; i++){
        weaps.push_back(Weapon());
    }

    assert(weaps.size() == NB_WEAPONS);

}

void spawnEnnemies(float totalTime)
{
    if(getRand()<0.1){
        ennemies.push_back(Ennemy("roucoups"));
        ennemies.back().img = imread( ROOTPATH "roucoups.png", IMREAD_COLOR );
    }
}

void drawImage(Mat &src, float x, float y)
{
    int px = ( x / 2.0 + 0.5) * SCREENWIDTH  - src.cols / 2.0;
    int py = (-y / 2.0 + 0.5) * SCREENHEIGHT - src.rows / 2.0;
    src.copyTo(display(Rect(px, py, src.cols, src.rows)));
}

void drawLine(float x1, float y1, float x2, float y2, const Scalar& bgr){
    float px1 = ( x1 / 2.0 + 0.5) * SCREENWIDTH;
    float py1 = (-y1 / 2.0 + 0.5) * SCREENHEIGHT;
    float px2 = ( x2 / 2.0 + 0.5) * SCREENWIDTH;
    float py2 = (-y2 / 2.0 + 0.5) * SCREENHEIGHT;
    line(display, Point(px1, py1), Point(px2, py2), bgr);
}

void drawWorld()
{
    for(unsigned int i=0; i<ents.size(); i++){
        float w = ents.at(i).w;
        float r = ents.at(i).r;
        drawImage(ents.at(i).img, POL_TO_X(w,r), POL_TO_Y(w,r));
    }
    for(unsigned int i=0; i<ennemies.size(); i++){
        if(ennemies.at(i).hp > 0.0){
            float w = ennemies.at(i).w;
            float r = ennemies.at(i).r;
            float x = POL_TO_X(w,r);
            float y = POL_TO_Y(w,r);
            float hp = ennemies.at(i).hp;
            drawImage(ennemies.at(i).img, x, y);
            float dy = 1.1*ennemies.at(i).img.rows/SCREENHEIGHT;
            float dx = hp/100.0*ennemies.at(i).img.cols/SCREENWIDTH;
            drawLine(x-dx, y+dy, x+dx, y+dy, Scalar(0,0,255));
        }
    }
}

void moveEnemies(float deltaTime)
{
    for(unsigned int i=0; i<ennemies.size(); i++){
        if(ennemies.at(i).hp > 0.0){
            ennemies.at(i).r = ennemies.at(i).r - ennemies.at(i).dr * deltaTime;
            ennemies.at(i).w = ennemies.at(i).w + ennemies.at(i).dw * deltaTime;
        }
    }
}

void handleWeapons()
{
    for(unsigned int i=0; i<NB_WEAPONS; i++){
        if(images.at(i).detected > 0.5){
            int slot = getSlot(images.at(i).position.x, images.at(i).position.y);
            float w = ents.at(slot).w;
            float r = ents.at(slot).r;
            float angle = weaps.at(i).angle;
            float range = weaps.at(i).range;

            // Draw cone
            float ang1 = w+angle/2;
            float ang2 = w-angle/2;
            float x0 = r * cos(w);
            float y0 = r * sin(w);
            float x1 = x0 + range * cos(ang1);
            float y1 = y0 + range * sin(ang1);
            drawLine(x0,y0,x1,y1, Scalar(255,0,0));
            float x2 = x0 + range * cos(ang2);
            float y2 = y0 + range * sin(ang2);
            drawLine(x0,y0,x2,y2, Scalar(255,0,0));

            // Find nearest ennemy
            Ennemy* nearest = NULL;
            for(unsigned int j=0; j<ennemies.size(); j++){
                if(ennemies.at(j).hp > 0.0){
                    float dw = ennemies.at(j).w - w;
                    while(dw <= -M_PI){ dw += 2*M_PI; }
                    while(dw > M_PI){ dw -= 2*M_PI; }
                    if(-angle/2 <= dw && dw <= angle/2){ // In cone
                        if(!nearest || ennemies.at(j).r < nearest->r){
                            nearest = &ennemies.at(j);
                        }
                    }
                }
            }

            // Hit nearest
            if(nearest && nearest->r < range){
                nearest->hp -= weaps.at(i).dps;
                float x = POL_TO_X(nearest->w, nearest->r);
                float y = POL_TO_Y(nearest->w, nearest->r);
                drawLine(x0,y0,x,y, Scalar(255,255,0));
            }

        }
    }
}

void handleEndGame(){
    for(unsigned int i=0; i<ennemies.size(); i++){
        if(ennemies.at(i).r < 0.0){
            Mat endgame = imread( ROOTPATH "endgame.png", IMREAD_COLOR );
            imshow("GAME OVER", endgame);
            for(;;){ if(waitKey(30) >= 0) break; }
            return;
        }
    }
}

// GAMEPLAY

void gameplay()
{
    int thisTime = GetTickCount();
    float deltaTime = (thisTime-lastTime)/1000.0;
    float totalTime = (thisTime-startTime)/1000.0;

    for(unsigned int i=0; i<images.size(); i++){
        std::cout << images.at(i).name << ": " << images.at(i).detected << std::endl;
    }
    std::cout << "deltaTime: " << deltaTime << " s\n" << std::endl;

    // Create green background
    display = cv::Scalar(100,200,0);

    handleWeapons();
    spawnEnnemies(totalTime);
    moveEnemies(deltaTime);
    handleEndGame();
    drawWorld();

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
#ifdef DISPLAY_SMALL
    imshow(name, img_corrected_colored);
#endif

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
        Mat img_scene;
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
#ifdef DISPLAY_CAM
        // display frame with detected objects
        line(frame,Point(0, frame.rows/2), Point(frame.cols, frame.rows/2), Scalar(255,0,0));
        line(frame,Point(frame.cols/3, 0), Point(frame.cols/3, frame.rows), Scalar(255,0,0));
        line(frame,Point(2*frame.cols/3, 0), Point(2*frame.cols/3, frame.rows), Scalar(255,0,0));
        imshow("edges", frame);
#endif

        gameplay();

        if(waitKey(30) >= 0) break;
    }

    return 0;
}
