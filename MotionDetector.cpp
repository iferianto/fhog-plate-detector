#include "MotionDetector.h"

//#define DEBUG

MotionDetector::MotionDetector(){
	read_config_file();
	m_pMOG2 = cv::createBackgroundSubtractorMOG2(300, 32, true);
}


MotionDetector::~MotionDetector(){


}


// return list of bounding rectangles
std::vector<cv::Rect> MotionDetector::get_bounding_rectangles(){
	assert(!m_frame.empty());

	m_pMOG2->apply(m_frame, m_fgMaskMOG2);
	cv::morphologyEx(m_fgMaskMOG2, m_binary_image, CV_MOP_CLOSE, SE5x5);
	cv::threshold(m_binary_image, m_binary_image, MIN_BINARY_THRESHOLD, MAX_BINARY_THRESHOLD, CV_THRESH_BINARY);

	std::vector< std::vector< cv::Point> > contours;

	cv::findContours(m_binary_image, contours,
		CV_RETR_EXTERNAL, // retrieve the external contours
		CV_CHAIN_APPROX_SIMPLE); // all pixels of each contours
	std::vector< std::vector< cv::Point> >::iterator itc = contours.begin();
	

	// Filter Bounding Rectangles
	while (itc != contours.end()) {
		cv::Rect br = cv::boundingRect(cv::Mat(*itc));

		if (//br.area() > MIN_BR_AREA &&
			//br.width / br.height > MIN_BR_ASPECT_RATIO &&
			//br.width / br.height < MAX_BR_ASPECT_RATIO &&
			br.height > MIN_BR_HEIGHT &&
			br.height < MAX_BR_HEIGHT &&
			br.width > MIN_BR_WIDTH &&
			br.width < MAX_BR_WIDTH // 
			)
		{
			m_bounding_rectangles.push_back(br); // Add appropriate rectangles to the vector
			
		}

		++itc;
	}
	
	return m_bounding_rectangles;
}

void MotionDetector::read_config_file(){

#ifdef DEBUG
	std::cout << "MotionDetectorConfig.txt Loaded!" << std::endl;
#endif

	dlib::config_reader cr("MotionDetectorConfig.txt");
	try{
		MAX_BR_ASPECT_RATIO = dlib::get_option(cr, "br_options.MAX_BR_ASPECT_RATIO", 3.0);
		MIN_BR_ASPECT_RATIO = dlib::get_option(cr, "br_options.MIN_BR_ASPECT_RATIO", 0.5);
		MAX_BR_HEIGHT = dlib::get_option(cr, "br_options.MAX_BR_HEIGHT", 300);
		MIN_BR_HEIGHT = dlib::get_option(cr, "br_options.MIN_BR_HEIGHT", 100);
		MAX_BR_WIDTH = dlib::get_option(cr, "br_options.MAX_BR_WIDTH", 300);
		MIN_BR_WIDTH = dlib::get_option(cr, "br_options.MIN_BR_WIDTH", 100);
		MIN_BR_AREA = dlib::get_option(cr, "br_options.MIN_BR_AREA", 5000);
		MAX_BINARY_THRESHOLD = dlib::get_option(cr, "threshold.MAX_BINARY_THRESHOLD", 255);
		MIN_BINARY_THRESHOLD = dlib::get_option(cr, "threshold.MIN_BINARY_THRESHOLD", 100);	
	}

	catch (std::exception &e){
		std::cout << e.what() << std::endl;
	}
	



}

void MotionDetector::set_frame(const cv::Mat &frame){
	m_frame = frame;
}

cv::Mat MotionDetector::get_frame(){

	return m_frame;
}

void MotionDetector::clear_bounding_rectangles(){
	
	m_bounding_rectangles.clear();

}