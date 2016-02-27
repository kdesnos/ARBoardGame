#include <stdio.h>
#include <iostream>

#ifdef __GNUC__
#include <unistd.h>
#endif

#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/features2d.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/calib3d.hpp"
#include "opencv2/xfeatures2d.hpp"
#include "opencv2/opencv.hpp"
#include <windows.h>

#ifndef M_PI
#define M_PI 3.1415926535
#endif

#define DISPLAY
#define DISPLAY_CAM
//#define DISPLAY_SMALL
// #define INFINITE_MODE
#define THRESHOLD 0.4
#define STEP 0.2
#define SCREENWIDTH 900
#define SCREENHEIGHT 900
#define SLOT_RADIUS 0.15

#define DEBUG std::cout << "Line " << __LINE__ << std::endl
#define POL_TO_X(w, r) ((r)*cos((w)))
#define POL_TO_Y(w, r) ((r)*sin((w)))
#define DEG_TO_RAD(d) ((d)/180*M_PI)

using namespace cv;
using namespace cv::xfeatures2d;

#define NB_WEAPONS 6
#define ROOTPATH "../data/"
String image_paths[][2] = {
    {"blaster", ROOTPATH "blaster.png"},
    {"G1",      ROOTPATH "gatling1.png"},
    {"BR",      ROOTPATH "batteringram.png"},
    {"Bow",     ROOTPATH "bow.png"},
    {"CB",      ROOTPATH "crossbow.png"},
    {"FG",      ROOTPATH "flaregun.png"}
};



// TOOLS

double getRand()
{
    return (rand()%1000)/1000.0;
}

double getRandAngle(){
    return -M_PI + getRand()*M_PI*2.0;
}



// STRUCTS

typedef struct Image {
    String name;
    Mat img;
    Mat descriptors_object;
    std::vector<KeyPoint> keypoints_object;
    double detected;
    Point2f position;
} Image;

typedef struct Entity {
    String name;
    Mat img;
	double w;
	double r;
    Entity(String _name){ name = _name; w = 0.0; r = 0.1; }
} Entity;

typedef struct Ennemy {
    Mat img;
	double r;
	double w;
	double hp;
	double dr;
	double dw;
    Ennemy(){ r = 0.9; w = getRandAngle(); hp = 100.0; dr = 0.02; dw = 0.0; }
} Ennemy;

typedef struct Weapon {
	double dps;
	double angle;
	double range;
    Weapon(){ dps = 30.0; angle = DEG_TO_RAD(60.0); range = 0.5; }
} Weapon;



// GLOBAL VARS

int startTime = GetTickCount();
int lastTime = startTime;
std::vector<Image> images;
std::vector<Entity> ents;
std::vector<Ennemy> ennemies;
std::vector<Weapon> weaps;
Mat frame;
Mat display(SCREENHEIGHT, SCREENWIDTH, CV_8UC4);
bool quit = false;



// METHODS

int getSlot(double x, double y)
{
    int slot = (y / frame.rows) > 0.5 ? 3 : 0;
    slot += (x / frame.cols) > 0.66 ? 3 : ((x / frame.cols) > 0.33 ? 2 : 1);
    int lut[] = {0, 6, 5, 4, 1, 2, 3};
    return lut[slot];
}

void populateGame()
{
    // CASTLE
    ents.push_back(Entity("castle"));
    ents.back().w = 0.0;
    ents.back().r = 0.0;
    ents.back().img = imread( ROOTPATH "castle.png", IMREAD_UNCHANGED );

    // SLOTS
	double radius = SLOT_RADIUS;
	double angle = M_PI/6.0;
    ents.push_back(Entity("slot1"));
    ents.back().w = angle;
    ents.back().r = radius;
    ents.back().img = imread( ROOTPATH "slot1.png", IMREAD_UNCHANGED );
    angle += M_PI/3.0;
    ents.push_back(Entity("slot1"));
    ents.back().w = angle;
    ents.back().r = radius;
    ents.back().img = imread( ROOTPATH "slot2.png", IMREAD_UNCHANGED );
    angle += M_PI/3.0;
    ents.push_back(Entity("slot1"));
    ents.back().w = angle;
    ents.back().r = radius;
    ents.back().img = imread( ROOTPATH "slot3.png", IMREAD_UNCHANGED );
    angle += M_PI/3.0;
    ents.push_back(Entity("slot1"));
    ents.back().w = angle;
    ents.back().r = radius;
    ents.back().img = imread( ROOTPATH "slot4.png", IMREAD_UNCHANGED );
    angle += M_PI/3.0;
    ents.push_back(Entity("slot1"));
    ents.back().w = angle;
    ents.back().r = radius;
    ents.back().img = imread( ROOTPATH "slot5.png", IMREAD_UNCHANGED );
    angle += M_PI/3.0;
    ents.push_back(Entity("slot1"));
    ents.back().w = angle;
    ents.back().r = radius;
    ents.back().img = imread( ROOTPATH "slot6.png", IMREAD_UNCHANGED );

    // WEAPONS
    image_paths; // See order here
    weaps.push_back(Weapon());
    weaps.back().dps = 30.0;
    weaps.back().range = 0.6;
    weaps.back().angle = DEG_TO_RAD(60.0);
    weaps.push_back(Weapon());
    weaps.back().dps = 60.0;
    weaps.back().range = 0.6;
    weaps.back().angle = DEG_TO_RAD(90.0);
    weaps.push_back(Weapon());
    weaps.back().dps = 300.0;
    weaps.back().range = 0.25;
    weaps.back().angle = DEG_TO_RAD(30.0);
    weaps.push_back(Weapon());
    weaps.back().dps = 30.0;
    weaps.back().range = 1.0;
    weaps.back().angle = DEG_TO_RAD(30.0);
    weaps.push_back(Weapon());
    weaps.back().dps = 60.0;
    weaps.back().range = 0.6;
    weaps.back().angle = DEG_TO_RAD(30.0);
    weaps.push_back(Weapon());
    weaps.back().dps = 30.0;
    weaps.back().range = 0.3;
    weaps.back().angle = DEG_TO_RAD(30.0);

}

typedef struct WaveElement {
	double time;
    int wave;
    String path;
	double w;
	double r;
	double hp;
	double dr;
	double dw;
} WaveElement;

#define NB_WAVES 5
#define NB_WAVE_ELTS 20
WaveElement waves[] = {
    { 0.0f, 1, ROOTPATH "roucoups.png", DEG_TO_RAD(120.0), 0.9, 50.0, 0.05, 0.0},
    { 1.0f, 2, ROOTPATH "roucoups.png", DEG_TO_RAD( 50.0), 0.9, 50.0, 0.05, 0.0},
    { 2.1f, 4, ROOTPATH "triopikeur.png", DEG_TO_RAD(285.0), 0.3, 100.0, 0.001, 0.005},
    { 2.5f, 2, ROOTPATH "saquedeneu.png", DEG_TO_RAD(160.0), 0.9, 50.0, 0.01, 0.1},
    { 3.0f, 1, ROOTPATH "roucoups.png", DEG_TO_RAD(280.0), 0.9, 50.0, 0.05, 0.0},

    { 3.0f, 2, ROOTPATH "roucoups.png", DEG_TO_RAD(285.0), 0.9, 50.0, 0.05, 0.0},
    { 5.0f, 1, ROOTPATH "roucoups.png", DEG_TO_RAD(160.0), 0.9, 50.0, 0.05, 0.0},
    { 5.5f, 4, ROOTPATH "triopikeur.png", DEG_TO_RAD(0.0), 0.3, 100.0, 0.001, 0.005},
    { 5.5f, 4, ROOTPATH "triopikeur.png", DEG_TO_RAD(40.0), 0.3, 100.0, 0.001, 0.005},
    { 5.5f, 4, ROOTPATH "triopikeur.png", DEG_TO_RAD(80.0), 0.3, 100.0, 0.001, 0.005},

    { 5.5f, 4, ROOTPATH "triopikeur.png", DEG_TO_RAD(120.0), 0.3, 100.0, 0.001, 0.005},
    { 5.5f, 4, ROOTPATH "triopikeur.png", DEG_TO_RAD(160.0), 0.3, 100.0, 0.001, 0.005},
    { 6.5f, 2, ROOTPATH "saquedeneu.png", DEG_TO_RAD(260.0), 0.9, 50.0, 0.01, -0.1},
    { 7.5f, 2, ROOTPATH "saquedeneu.png", DEG_TO_RAD(160.0), 0.9, 50.0, 0.01, 0.1},
    { 9.0f, 3, ROOTPATH "rhinoferos.png", DEG_TO_RAD(100.0), 0.9, 300.0, 0.02, 0.0},

    { 9.5f, 4, ROOTPATH "triopikeur.png", DEG_TO_RAD(285.0), 0.3, 100.0, 0.001, 0.005},
    {10.0f, 1, ROOTPATH "roucoups.png", DEG_TO_RAD(300.0), 0.9, 50.0, 0.05, 0.0},
    {15.0f, 1, ROOTPATH "roucoups.png", DEG_TO_RAD(350.0), 0.9, 50.0, 0.05, 0.0},
    {16.0f, 3, ROOTPATH "rhinoferos.png", DEG_TO_RAD(180.0), 0.9, 300.0, 0.02, 0.0},
    {25.0f, 5, ROOTPATH "magicarpe.png", DEG_TO_RAD(0.0), 0.7, 3000.0, 0.01, 0.1}
};

void spawnEnnemies(double totalTime)
{
#ifndef INFINITE_MODE
    static int idx = 0;
    static double init_wave_time = 0.0;
    static int wave = 1;

    if(idx >= NB_WAVE_ELTS) {
        bool allDead = true;
        for(int i = 0; i< ennemies.size(); i++){
            if(ennemies.at(i).hp > 0.0){
                allDead = false;
                break;
            }
        }
        if(allDead) {
            ennemies.clear();
            wave++;
            std::cout << "Starting Wave " << wave << " in 5 seconds." << std::endl;
            init_wave_time = totalTime + 5.0;

            idx=0;


        }

        return;
    }

    if(wave > NB_WAVES) {
        Mat endgame = imread( ROOTPATH "victory.png", IMREAD_UNCHANGED );
        imshow("Victory", endgame);
        std::cout << "totalTime: " << totalTime << std::endl;
        for(;;){ if(waitKey(30) >= 0) break; }
        quit = true;
    }

    if(waves[idx].time < (totalTime - init_wave_time)) {
        ennemies.push_back(Ennemy());
        ennemies.back().img = imread( waves[idx].path, IMREAD_UNCHANGED );
        ennemies.back().r = waves[idx].r;
        ennemies.back().w = waves[idx].w;
        ennemies.back().hp = waves[idx].hp;
        ennemies.back().dr = waves[idx].dr;
        ennemies.back().dw = waves[idx].dw;
        idx++;
    }

    while(waves[idx].wave > wave){
        idx++;
    };

#else
	double multiplier = totalTime/100.0;

    if(getRand()<multiplier*0.03){
        ennemies.push_back(Ennemy());
        ennemies.back().img = imread( ROOTPATH "roucoups.png", IMREAD_UNCHANGED );
        ennemies.back().hp = 50.0;
        ennemies.back().dr = 0.05;
    }
    if(getRand()<multiplier*0.1){
        ennemies.push_back(Ennemy());
        ennemies.back().img = imread( ROOTPATH "saquedeneu.png", IMREAD_UNCHANGED );
        ennemies.back().dw = getRand()*0.1;
        ennemies.back().dr = 0.01;
    }
    if(getRand()<multiplier*0.01){
        ennemies.push_back(Ennemy());
        ennemies.back().img = imread( ROOTPATH "rhinoferos.png", IMREAD_UNCHANGED );
        ennemies.back().hp = 300.0;
    }
#endif
}

void drawImage(Mat &src, double x, double y)
{
    int px = (int)(( x / 2.0 + 0.5) * SCREENWIDTH  - src.cols / 2.0);
    int py = (int)((-y / 2.0 + 0.5) * SCREENHEIGHT - src.rows / 2.0);
    src.copyTo(display(Rect(px, py, src.cols, src.rows)));
}

void drawLine(double x1, double y1, double x2, double y2, const Scalar& bgr, int thickness = 1){
	double px1 = ( x1 / 2.0 + 0.5) * SCREENWIDTH;
	double py1 = (-y1 / 2.0 + 0.5) * SCREENHEIGHT;
	double px2 = ( x2 / 2.0 + 0.5) * SCREENWIDTH;
	double py2 = (-y2 / 2.0 + 0.5) * SCREENHEIGHT;
    line(display, Point((int)px1, (int)py1), Point((int)px2, (int)py2), bgr, thickness);
}

void drawWorld()
{
    for(unsigned int i=0; i<ents.size(); i++){
		double w = ents.at(i).w;
		double r = ents.at(i).r;
        drawImage(ents.at(i).img, POL_TO_X(w,r), POL_TO_Y(w,r));
    }
    for(unsigned int i=0; i<ennemies.size(); i++){
        if(ennemies.at(i).hp > 0.0){
			double w = ennemies.at(i).w;
			double r = ennemies.at(i).r;
			double x = POL_TO_X(w,r);
			double y = POL_TO_Y(w,r);
			double hp = ennemies.at(i).hp;
            drawImage(ennemies.at(i).img, x, y);
			double dy = 1.1*ennemies.at(i).img.rows/SCREENHEIGHT;
			double dx = hp/100.0;
            dx = sqrt(dx);
            dx = dx*0.05;
            drawLine(x-dx, y+dy, x+dx, y+dy, Scalar(0,0,255), 3);
        }
    }
}

void moveEnemies(double deltaTime)
{
    for(unsigned int i=0; i<ennemies.size(); i++){
        if(ennemies.at(i).hp > 0.0){
            ennemies.at(i).r = ennemies.at(i).r - ennemies.at(i).dr * deltaTime;
            ennemies.at(i).w = ennemies.at(i).w + ennemies.at(i).dw * deltaTime;
        }
    }
}

void handleWeapons(double deltaTime)
{
    for(unsigned int i=0; i<NB_WEAPONS; i++){
        if(images.at(i).detected > 0.5){
            int slot = getSlot(images.at(i).position.x, images.at(i).position.y);
            double w = ents.at(slot).w;
            double r = ents.at(slot).r;
            double angle = weaps.at(i).angle;
            double range = weaps.at(i).range;
            double dps = weaps.at(i).dps;

            // Draw cone
            double ang1 = w+angle/2;
            double ang2 = w-angle/2;
            double x0 = r * cos(w);
            double y0 = r * sin(w);
            double x1 = range * cos(ang1);
            double y1 = range * sin(ang1);
            Scalar color = Scalar(0,0,255);
            if(dps < 70.0) {
                color = Scalar(0,128,255);
            }
            if(dps < 40.0) {
                color = Scalar(0,255,255);
            }
            drawLine(x0,y0,x1,y1, color,2);
            double x2 = range * cos(ang2);
            double y2 = range * sin(ang2);
            drawLine(x0,y0,x2,y2, color,2);

            // Find nearest ennemy
            Ennemy* nearest = NULL;
            for(unsigned int j=0; j<ennemies.size(); j++){
                if(ennemies.at(j).hp > 0.0){
                    double dw = ennemies.at(j).w - w;
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
                nearest->hp -= weaps.at(i).dps*deltaTime;
                double x = POL_TO_X(nearest->w, nearest->r);
                double y = POL_TO_Y(nearest->w, nearest->r);
                drawLine(x0,y0,x,y, Scalar(255,255,0));
            }

        }
    }
}

void handleEndGame(double totalTime){
    for(unsigned int i=0; i<ennemies.size(); i++){
        if(ennemies.at(i).r < 0.0){
            Mat endgame = imread( ROOTPATH "endgame.png", IMREAD_UNCHANGED );
            imshow("GAME OVER", endgame);
            std::cout << "totalTime: " << totalTime << std::endl;
            for(;;){ if(waitKey(30) >= 0) break; }
            quit = true;
        }
    }
}

// GAMEPLAY

void gameplay()
{
    int thisTime = GetTickCount();
    double deltaTime = (thisTime-lastTime)/1000.0;
    double totalTime = (thisTime-startTime)/1000.0;

//    for(unsigned int i=0; i<images.size(); i++){
//        std::cout << images.at(i).name << ": " << images.at(i).detected << std::endl;
//    }
//    std::cout << "deltaTime: " << deltaTime << " s\n" << std::endl;

    // Create green background
    display = cv::Scalar(100,200,0,255);

    //line(display,Point2f(0,0), Point2f(100,100),Scalar(30,30,30));
    handleWeapons(deltaTime);
    spawnEnnemies(totalTime);
    moveEnemies(deltaTime);
    handleEndGame(totalTime);
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

    double currentDetected = 0.0;
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
		im.position = Point2f(mean_.at<float>(0, 0), mean_.at<float>(0, 1));
    }
    // std::cout << name << " - " << im.detected << " " << isDetected.at<double>(0,0) << std::endl;

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
        if(quit) return 0;

        if(waitKey(30) >= 0) break;
    }

    return 0;
}
