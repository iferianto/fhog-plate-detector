#ifndef PLATE_DETECTOR_H
#define PLATE_DETECTOR_H

#include <dlib/string.h>
#include <dlib/image_processing.h>
#include <dlib/data_io.h>
#include <dlib/opencv.h>
#include <dlib/gui_widgets.h> // image_window
#include <fstream>

#include "MotionDetector.h"

typedef dlib::scan_fhog_pyramid<dlib::pyramid_down<6>> image_scanner_type;

class PlateDetector{

public:
	PlateDetector();
	~PlateDetector();
	dlib::array2d<uchar> detect(cv::Mat &frame, int mode);
	

private:

	int m_detection_mode;

	MotionDetector m_md; // motion detection object
	dlib::array2d<uchar> get_best_plate();
	// Detectors
	std::vector<dlib::object_detector<image_scanner_type>> m_detectors;

	// Frames
	cv::Mat m_frame; // if original input has a higher resolution
	cv::Mat m_resized_frame; // used just for plate detection, has a lower resolution

	// Directories
	std::string svm_directory; // directory of pretrained .svm files
	std::vector<std::string> svm_file_names;


	// Detection Rectangles
	dlib::rectangle m_current_det; // current detected rectangle coming from evaluate_detectors()
	dlib::rectangle m_previous_det; // remember the previous detected rectangle

	// Best Detection Properties
	dlib::rectangle m_best_det; // coordinates of best detected rectangle for the moving object
	cv::Mat m_best_frame; // the best frame for the moving object, cv::Mat since dlib is not allow to copy images
	std::string m_best_dir; // to which directory you want to save (high/medium/low)
	double m_current_confidence;
	double m_best_confidence;

	// read from config
	const double CONVERTED_FRAME_WIDTH = 800;
	const double CONVERTED_FRAME_HEIGHT = 640;
	const int STUPID_CONSTANT_FOR_RESETTING_RECTANGLE = 9999; // NO COMMENT

	// Functions
	bool check_same_plate(dlib::rectangle &r1, dlib::rectangle &r2, dlib::rectangle &bounding_rectangle);
	void reset_nomotion();
	std::vector<dlib::object_detector<image_scanner_type>> load_detectors(bool d1, bool d2, bool d3);
	dlib::rectangle find_in_original_frame(cv::Rect &bounding_rect, dlib::rectangle &det);
	dlib::rectangle openCVRectToDlib(cv::Rect &r);
	void update_best_plate(cv::Mat &best_detected_frame,
		dlib::rectangle &best_detected_det,	double &best_detection_confidence,
		std::string best_detected_dir);
	void read_config_file();
	void resize_frame(cv::Mat &frame);
	dlib::array2d<uchar> convert_cv_2_dlib_image(const cv::Mat &cv_im);
	
};

#endif
