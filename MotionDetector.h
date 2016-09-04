#ifndef MOTION_DETECTOR_H
#define MOTION_DETECTOR_H

#include <assert.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include <dlib/config_reader.h>



const cv::Mat SE5x5 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));

class MotionDetector{

private:
	cv::Mat m_frame;
	cv::Mat m_binary_image;
	cv::Mat m_fgMaskMOG2;
	cv::Ptr<cv::BackgroundSubtractor> m_pMOG2;
	std::vector<cv::Rect> m_bounding_rectangles;

	double MAX_BR_ASPECT_RATIO;
	double MIN_BR_ASPECT_RATIO;
	double MAX_BR_HEIGHT;
	double MIN_BR_HEIGHT;
	double MAX_BR_WIDTH;
	double MIN_BR_WIDTH;
	double MIN_BR_AREA;
	double MAX_BINARY_THRESHOLD;
	double MIN_BINARY_THRESHOLD;

	void read_config_file();
 
public:
	MotionDetector();
	//MotionDetector(const cv::Mat &frame);
	~MotionDetector();
	std::vector<cv::Rect> get_bounding_rectangles();
	void set_frame(const cv::Mat &frame);
	cv::Mat get_frame();
	void clear_bounding_rectangles(); // clear br after each frame

};

#endif