#include "PlateDetector.h"
#include <chrono>

//#define DEBUG

PlateDetector::PlateDetector(){
	read_config_file(); // PlateDetectorConfig.txt

	if (DETECTION_RECTANGLE_BOT == 0 &&
		DETECTION_RECTANGLE_BOT == 0 &&
		DETECTION_RECTANGLE_BOT == 0 &&
		DETECTION_RECTANGLE_BOT == 0 ){
		
		USER_DEFINED_ROI = false; 
	}
	else{
		USER_DEFINED_ROI = true;
	}


	m_detectors = load_detectors(true, true, true); // all 3 detectors are active
	reset_nomotion(); // reset best_frame and previous rectangle
}

PlateDetector::~PlateDetector(){



}

void PlateDetector::resize_frame(cv::Mat &frame){
	// Input frame is resized to 800x640, if it has a higher resolution
	if (frame.size().width > CONVERTED_FRAME_WIDTH){
		cv::resize(frame, m_resized_frame, cv::Size(CONVERTED_FRAME_WIDTH, CONVERTED_FRAME_HEIGHT));
	}
	else{
		m_resized_frame = frame; // else do nothing
	}


}

/*
* reset_nomotion
*
* @brief:
*	- if there is no motion in the stream, reset the previous_det,
*	since there is no other plates to compare with
*
*/
void PlateDetector::reset_nomotion(){

	// BEST CODE IN THE WHOLE UNIVERSE
	m_previous_det.set_bottom(STUPID_CONSTANT_FOR_RESETTING_RECTANGLE);
	m_previous_det.set_top(STUPID_CONSTANT_FOR_RESETTING_RECTANGLE);
	m_previous_det.set_left(STUPID_CONSTANT_FOR_RESETTING_RECTANGLE);
	m_previous_det.set_right(STUPID_CONSTANT_FOR_RESETTING_RECTANGLE);

	m_best_frame.release();

}

/*
* load_detectors
*
* TODO: There may be more than 3 detectors, make it generic.
*
* @brief:
*	- Load a previously trained object detector located in svm_directory
*
* @params:
*	-d1, d2, d3: true as default setting, load all detectors
*
*/
std::vector<dlib::object_detector<image_scanner_type>> PlateDetector::load_detectors(bool d1, bool d2, bool d3){

	std::vector<dlib::object_detector<image_scanner_type>> my_detectors;

	if (d1){
		std::ifstream fDetector1(svm_directory + "sam_bil_akinci_201422_bil_odtu_90_38_mult_kare_c1000_resize_large.xml_.svm", std::ios::binary);
		dlib::object_detector<image_scanner_type> detector;
		dlib::deserialize(detector, fDetector1);
		my_detectors.push_back(detector);
	}
	if (d2){
		std::ifstream fDetector2(svm_directory + "sam_bil_akinci_201422_bil_odtu_90_38_mult_kare_c1000.xml_.svm", std::ios::binary);
		dlib::object_detector<image_scanner_type> detector2;
		dlib::deserialize(detector2, fDetector2);
		my_detectors.push_back(detector2);
	}
	if (d3){
		std::ifstream fDetector3(svm_directory + "sam_bil_akinci_201422_bil_odtu_120_50_mult_kare_c1000_resize_original.xml_.svm", std::ios::binary);
		dlib::object_detector<image_scanner_type> detector3;
		dlib::deserialize(detector3, fDetector3);
		my_detectors.push_back(detector3);
	}

	return my_detectors;

}


/*
* find_in_original_frame:
*
* @brief:
*	Since it is faster/cheaper to process, the plate detection runs on a resized image.
*	However, to recognize characters in the plate, higher resolution image is preferred.
*
* @params:
*	- cv::Rect bounding_rect: the rectangle where the motion occurs
*	- dlib::rectangle det: the rectangle where the plate detected
*
* @returns:
*	- corresponding detection rectangle in the original frame
*
*/
dlib::rectangle PlateDetector::find_in_original_frame(cv::Rect &bounding_rect, dlib::rectangle &det){

	dlib::rectangle newDet;

	m_w_ratio = m_frame.size().width / CONVERTED_FRAME_WIDTH;
	m_h_ratio = m_frame.size().height / CONVERTED_FRAME_HEIGHT; 

	if (m_w_ratio < 1){
		m_w_ratio = 1;
		m_h_ratio = 1;
	}

	newDet.set_left((det.tl_corner().x() + bounding_rect.tl().x) * m_w_ratio);
	newDet.set_right((det.br_corner().x() + bounding_rect.tl().x) * m_w_ratio);
	newDet.set_top((det.tl_corner().y() + bounding_rect.tl().y)* m_h_ratio);
	newDet.set_bottom((det.br_corner().y() + bounding_rect.tl().y) * m_h_ratio);

	return newDet;


}


dlib::rectangle PlateDetector::openCVRectToDlib(cv::Rect &r){
	return dlib::rectangle((long)r.tl().x, (long)r.tl().y, (long)r.br().x - 1, (long)r.br().y - 1);
}

/*
* update_best_plate
*
* @brief:
*	- in the same bounding rectangle, if better plate is found (more confident one) 
*updates the best plate.
*
* @returns:
*	- m_best_frame
*	- m_best_det   
*	- m_best_confidence
*	-
*
*/
void PlateDetector::update_best_plate(cv::Mat &current_frame,
	dlib::rectangle &current_det,
	double &current_confidence, std::string current_dir){

	m_best_frame = current_frame;
	m_best_det = current_det;
	m_best_confidence = current_confidence;
	m_best_dir = current_dir;

}


/*
* check_same_plate
*
* @brief:
*	We do want to track the plate after detection is finished.
*	If distance between two detected plates is small enough (depending on camera view /resolution), probably it is the same plate.
*	
*
*
* @params:
*	-r1: previous detected plate rectangle
*	-r2: current detected plate rectangle
*	-bounding_rectangle: current region of interest
* @returns:
*	true: if two rectangles are close to each other
*
*/
bool PlateDetector::check_same_plate(dlib::rectangle &r1, dlib::rectangle &r2, dlib::rectangle &bounding_rectangle){
	int delta_x = 50;
	int delta_y = 50;



	if (USER_DEFINED_ROI){
		if ((abs(r1.tl_corner().x() - r2.tl_corner().x()) < delta_x && abs(r1.tl_corner().y() - r2.tl_corner().y()) < delta_y)){
			return true;
		}

	}

	else{

		if ((abs(r1.tl_corner().x() - r2.tl_corner().x()) < delta_x && abs(r1.tl_corner().y() - r2.tl_corner().y()) < delta_y) ||
			(bounding_rectangle.contains(r1.tl_corner()) && bounding_rectangle.contains(r2.tl_corner()))
			)
		{

			return true;
		}

		
	}
	
	return false;

}

/*
* read_config_file
* 
* @brief:
*	- Called once when PlateDetector object is created.
*	- Reads PlateDetectorConfig.txt which contains svm files and user-defined detection rectangle
*
* @return:
*	- svm files (defalut " "), user-defiend detection rectangle (default 0)
*
* 
*/
void PlateDetector::read_config_file(){

	dlib::config_reader cr("PlateDetectorConfig.txt");

	svm_directory = dlib::get_option(cr, "svm_directory", "");

	// TODO: Read unlimited number of svm files
	svm_file_names.push_back(dlib::get_option(cr, "svm_files.file1", ""));
	svm_file_names.push_back(dlib::get_option(cr, "svm_files.file2", ""));
	svm_file_names.push_back(dlib::get_option(cr, "svm_files.file3", ""));

	// user-defined bounding rectangle
	DETECTION_RECTANGLE_TOP = dlib::get_option(cr, "DETECTION_RECTANGLE_TOP", 0);
	DETECTION_RECTANGLE_BOT = dlib::get_option(cr, "DETECTION_RECTANGLE_BOT", 0);
	DETECTION_RECTANGLE_LEFT = dlib::get_option(cr, "DETECTION_RECTANGLE_LEFT", 0);
	DETECTION_RECTANGLE_RIGHT = dlib::get_option(cr, "DETECTION_RECTANGLE_RIGHT", 0);
	 
}

/*
* convert_cv_2_dlib_image
*
* @brief: 
*	- Dlib's fhog evaluator works on dlib's rectangle and dlib's image (array2d),
* we use openCV to get frames.
*	 
*/
dlib::array2d<uchar> PlateDetector::convert_cv_2_dlib_image(const cv::Mat &cv_im){
	dlib::array2d<unsigned char> dlib_im;
	dlib::assign_image(dlib_im, dlib::cv_image<uchar>(cv_im));
	return dlib_im;
}

/*
* get_best_plate
*
* @brief:
*	- Called when returning the final result.
*	- Extracts the best detection rectangle from the frame (if exist)
* 
* @returns:
*	- array2d<uchar> output_rect
*
*
*/
dlib::array2d<uchar> PlateDetector::get_best_plate(){
	dlib::array2d<uchar> output_rect;

	if (m_best_det.is_empty() || m_best_frame.empty()){
		std::cerr << "best frame and best rectangle is empty" << std::endl;
		return output_rect;
	}

	dlib::array2d<uchar> dlib_img = convert_cv_2_dlib_image(m_best_frame);
	dlib::extract_image_chip(dlib_img, m_best_det, output_rect);
	return output_rect;
}

/*
* detect:
*
* @brief:
*	-
*
* @params:
*	-cv::Mat &frame: the current frame
*
*
* @returns:
*
*
*
*/
dlib::array2d<uchar> PlateDetector::detect(cv::Mat &frame){
	static int nicevariableman = 0;

	dlib::array2d<uchar> result;

	m_frame = frame; // cv::Mat m_frame, current frame
	cv::cvtColor(m_frame, m_frame, CV_BGR2GRAY); // convert channel to grayscale

	// Resize the frame if possible
	resize_frame(frame); // use m_resized_frame

	cv::cvtColor(m_resized_frame, m_resized_frame, CV_BGR2GRAY);
	cv::GaussianBlur(m_resized_frame, m_resized_frame, cv::Size(5, 5), 0); // blur to eliminate noise

	std::vector<cv::Rect> cv_bounding_rectangles;
	cv::Rect cv_roi_rect;
	cv::Mat cv_roi_im;

	// detect motion in predefined roi
	if (USER_DEFINED_ROI){
		cv_roi_rect = (cv::Rect(cv::Point(DETECTION_RECTANGLE_LEFT, DETECTION_RECTANGLE_TOP), cv::Point(DETECTION_RECTANGLE_RIGHT, DETECTION_RECTANGLE_BOT)));
		cv_roi_im = cv::Mat(m_resized_frame, cv_roi_rect); // crop the image from where motion occurs, assume you have 1 br
		m_md.set_frame(cv_roi_im);

		if (!m_md.get_bounding_rectangles().empty()){
			cv_bounding_rectangles.push_back(cv_roi_rect);
		}
	}
	
	// detect motion in whole frame as default
	else{
		m_md.set_frame(m_resized_frame);
		cv_bounding_rectangles = m_md.get_bounding_rectangles();	
	}
	
	// start plate detection if motion detected
	if (!cv_bounding_rectangles.empty()){
		
		dlib::array2d<unsigned char> dlib_frame = convert_cv_2_dlib_image(m_resized_frame); // keep the original frame for extraction
		for (auto &bounding_rect : cv_bounding_rectangles){
			cv::Mat cv_bounding_rectangle_im = cv::Mat(m_resized_frame, bounding_rect); // crop the image from where motion occurs
			dlib::rectangle dlib_bounding_rectangle = openCVRectToDlib(bounding_rect); // convert current slice to dlib image
			dlib::rectangle dlib_corresponding_bounding_rectangle = find_in_original_frame(cv::Rect(0, 0, 0, 0), dlib_bounding_rectangle); // motion rectangle in the hd frame

			dlib::array2d<unsigned char> dlib_bounding_rectangle_im = convert_cv_2_dlib_image(cv_bounding_rectangle_im); // detectors works on dlib images
			std::vector<dlib::rect_detection> rect_detections;
			std::vector<dlib::rectangle> dets;
			dlib::evaluate_detectors(m_detectors, dlib_bounding_rectangle_im, rect_detections); // get detection confidence

			for (auto &rect_det : rect_detections){
				dets.push_back(rect_det.rect);
			}
	
			// detector founds something
			if (!dets.empty()){
				m_current_det = dets[0]; // dets[0] is the most confident rectangle. (see: dlib::evaluate_detectors())
				m_current_confidence = rect_detections[0].detection_confidence;
				
				// usually plates have at least 0.5 confidence, ignore below 0.5
				if (m_current_confidence < 0.5){
					break;
				}

				dlib::rectangle corresponding_current_det = find_in_original_frame(bounding_rect, m_current_det);
				dlib::rectangle corresponding_previous_det = find_in_original_frame(bounding_rect, m_previous_det);

				// If previous_det exist (you have two rectangles to compare each other)
				if (m_previous_det.left() != STUPID_CONSTANT_FOR_RESETTING_RECTANGLE){


					// If previous and current rectangles are in the same bounding rectangle, compare confidences
					if (check_same_plate(corresponding_previous_det, corresponding_current_det, dlib_corresponding_bounding_rectangle)){

						// If current confidence is better, update best plate
						if (m_current_confidence >= m_best_confidence){

							update_best_plate(m_frame, corresponding_current_det, m_current_confidence, ""/*save_dir*/);
							m_previous_det = m_current_det;
						}
						// If previous confidence is better, do nothing
						else{

						}

					}

					// If previous and current rectangles are different 
					else{

						// send the best plate and update it
						result = get_best_plate();
						update_best_plate(m_frame, corresponding_current_det, m_current_confidence, ""/*save_dir*/);
						m_previous_det = m_current_det;


					}

				}

				// previous_det doesn't exist (no motion exist before)
				else{
					update_best_plate(m_frame, corresponding_current_det, m_current_confidence, ""/*save_dir*/);
					m_previous_det = m_current_det;

				}

				// 9 frames tolerance to detect plate when motion exist
				if (USER_DEFINED_ROI)
					nicevariableman = 0;
			}

			// in userdefinedroi mode, 
			if (USER_DEFINED_ROI){
				if (m_previous_det.left() != STUPID_CONSTANT_FOR_RESETTING_RECTANGLE && (++nicevariableman > 9)){
					if (!result.size())
						result = get_best_plate();
					reset_nomotion();
					nicevariableman = 0;
				}
			}
		}
	} 

	if (!USER_DEFINED_ROI){
		if (m_previous_det.left() != STUPID_CONSTANT_FOR_RESETTING_RECTANGLE && (++nicevariableman > 9)){
			result = get_best_plate();
			reset_nomotion();
			nicevariableman = 0;
		}
	}

	// each frame clear old rectangles
	m_md.clear_bounding_rectangles();

	

	return result;

}


