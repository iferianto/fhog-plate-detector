#include "PlateDetector.h"


//#define DEBUG

PlateDetector::PlateDetector(){
	read_config_file(); // PlateDetectorConfig.txt
	m_detectors = load_detectors(true, true, true); // all 3 detectors are active
	reset_nomotion(); // reset best_frame and previous rectangle
}

PlateDetector::~PlateDetector(){



}

void PlateDetector::resize_frame(cv::Mat &frame){
	// Input frame is resized to 800x640, if it has a higher resolution
	if (frame.size().width > 800){
		cv::resize(frame, m_resized_frame, cv::Size(CONVERTED_FRAME_WIDTH, CONVERTED_FRAME_HEIGHT));
	}
	else{
		m_resized_frame = frame; // else do nothing
	}


}


void PlateDetector::reset_nomotion(){

	// BEST CODE IN THE WHOLE UNIVERSE
	m_previous_det.set_bottom(STUPID_CONSTANT_FOR_RESETTING_RECTANGLE);
	m_previous_det.set_top(STUPID_CONSTANT_FOR_RESETTING_RECTANGLE);
	m_previous_det.set_left(STUPID_CONSTANT_FOR_RESETTING_RECTANGLE);
	m_previous_det.set_right(STUPID_CONSTANT_FOR_RESETTING_RECTANGLE);

	m_best_frame.release();

}

// TODO: read file paths from config file
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

dlib::rectangle PlateDetector::find_in_original_frame(cv::Rect &bounding_rect, dlib::rectangle &det){

	dlib::rectangle newDet;

	double w_ratio = m_frame.size().width / CONVERTED_FRAME_WIDTH; // must be greater than 1
	double h_ratio = m_frame.size().height / CONVERTED_FRAME_HEIGHT; // must be greater than 1

	newDet.set_left((det.tl_corner().x() + bounding_rect.tl().x) * w_ratio);
	newDet.set_right((det.br_corner().x() + bounding_rect.tl().x) * w_ratio);
	newDet.set_top((det.tl_corner().y() + bounding_rect.tl().y)* h_ratio);
	newDet.set_bottom((det.br_corner().y() + bounding_rect.tl().y) * h_ratio);

	return newDet;


}

dlib::rectangle PlateDetector::openCVRectToDlib(cv::Rect &r){
	return dlib::rectangle((long)r.tl().x, (long)r.tl().y, (long)r.br().x - 1, (long)r.br().y - 1);
}


void PlateDetector::update_best_plate(cv::Mat &current_frame,
	dlib::rectangle &current_det,
	double &current_confidence, std::string current_dir){

	m_best_frame = current_frame;
	m_best_det = current_det;
	m_best_confidence = current_confidence;
	m_best_dir = current_dir;

}

bool PlateDetector::check_same_plate(dlib::rectangle &r1, dlib::rectangle &r2, dlib::rectangle &bounding_rectangle){
	int delta_x = 100;
	int delta_y = 100;
	// distance between 2 rectangles is less than deltas
	if ((abs(r1.tl_corner().x() - r2.tl_corner().x()) < delta_x && abs(r1.tl_corner().y() - r2.tl_corner().y()) < delta_y) ||
		(bounding_rectangle.contains(r1.tl_corner()) && bounding_rectangle.contains(r2.tl_corner()))
		)
	{

		return true;
	}

	return false;

}

void PlateDetector::read_config_file(){

#ifdef DEBUG
	std::cout << "PlateDetectorConfig.txt Loaded!" << std::endl;
#endif 

	dlib::config_reader cr("PlateDetectorConfig.txt");

	svm_directory = dlib::get_option(cr, "svm_directory", "");

	// TODO: Read unlimited number of svm files
	svm_file_names.push_back(dlib::get_option(cr, "svm_files.file1", ""));
	svm_file_names.push_back(dlib::get_option(cr, "svm_files.file2", ""));
	svm_file_names.push_back(dlib::get_option(cr, "svm_files.file3", ""));




}

dlib::array2d<uchar> PlateDetector::convert_cv_2_dlib_image(const cv::Mat &cv_im){
	dlib::array2d<unsigned char> dlib_im;
	dlib::assign_image(dlib_im, dlib::cv_image<uchar>(cv_im));
	return dlib_im;
}

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

// TODO: define detection mode
dlib::array2d<uchar> PlateDetector::detect(cv::Mat &frame, int detection_mode){

	dlib::array2d<uchar> result;


	static int counter = 0;


	m_frame = frame; // cv::Mat m_frame
	cv::cvtColor(m_frame, m_frame, CV_BGR2GRAY);

	// Resize to ease detection if possible
	resize_frame(frame); // then use m_resized_frame

	// Adjust color and blur etc.
	cv::cvtColor(m_resized_frame, m_resized_frame, CV_BGR2GRAY);
	cv::GaussianBlur(m_resized_frame, m_resized_frame, cv::Size(5, 5), 0);

	std::vector<dlib::rectangle> dets;
	std::vector<dlib::rect_detection> confidences;
	dlib::array2d<unsigned char> dlib_frame = convert_cv_2_dlib_image(m_resized_frame); // keep the original frame for extraction


	// 1- Detect Motion on processed frame
	std::vector<cv::Rect> cv_bounding_rectangles;

	if (detection_mode == 0){
		m_md.set_frame(m_resized_frame);
		cv_bounding_rectangles = m_md.get_bounding_rectangles();
	}
	// passive motion detector
	else if (detection_mode == 1){
		cv_bounding_rectangles.push_back(cv::Rect(0, 100, 400, 300));
	}

	// 2- Detect Plate (if motion detected)
	if (!cv_bounding_rectangles.empty()){

		for (auto &bounding_rect : cv_bounding_rectangles){
			cv::Mat cv_bounding_rectangle_im = cv::Mat(m_resized_frame, bounding_rect); // crop the image from where motion occurs
			dlib::rectangle dlib_bounding_rectangle = openCVRectToDlib(bounding_rect); // convert current slice to dlib image
			dlib::rectangle dlib_corresponding_bounding_rectangle = find_in_original_frame(cv::Rect(0, 0, 0, 0), dlib_bounding_rectangle); // motion rectangle in the hd frame

			dlib::array2d<unsigned char> dlib_bounding_rectangle_im = convert_cv_2_dlib_image(cv_bounding_rectangle_im); // detectors works on dlib images

			dets = dlib::evaluate_detectors(m_detectors, dlib_bounding_rectangle_im); // returns list of detected rectangles

			dlib::evaluate_detectors(m_detectors, dlib_bounding_rectangle_im, confidences); // get detection confidence


			// Detector finds something 
			if (!dets.empty()){
				m_current_det = dets[0]; // dets[0] is the most confident rectangle. (see: dlib::evaluate_detectors())
				m_current_confidence = confidences[0].detection_confidence;

				if (m_current_confidence < 0.35)
					break;

				dlib::rectangle corresponding_current_det = find_in_original_frame(bounding_rect, m_current_det);
				dlib::rectangle corresponding_previous_det = find_in_original_frame(bounding_rect, m_previous_det);


				// If previous_det exist (you have two rectangles to compare each other)
				if (m_previous_det.left() != STUPID_CONSTANT_FOR_RESETTING_RECTANGLE){


					// If previous and current rectangles are in the same bounding rectangle, compare confidences
					if (check_same_plate(corresponding_previous_det, corresponding_current_det, dlib_corresponding_bounding_rectangle)){

						// If current confidence is better, update best plate
						if (m_current_confidence >= m_best_confidence){
							std::cout << "better confidence detected " << m_current_confidence << std::endl;
							update_best_plate(m_frame, corresponding_current_det, m_current_confidence, ""/*save_dir*/);
							m_previous_det = m_current_det;
							system("pause");
						}
						// If previous confidence is better, do nothing
						else{
							std::cout << "previous confidence is better" << std::endl;
							system("pause");
						}

					}

					// If previous and current rectangles are different 
					else{

						// send the best plate and update it
						std::cout << "prev != current, save plate" << std::endl;
						result = get_best_plate();
						update_best_plate(m_frame, corresponding_current_det, m_current_confidence, ""/*save_dir*/);
						m_previous_det = m_current_det;

					}

				}

				// previous_det doesn't exist (no motion exist before)
				else{
					std::cout << "prev det doesnt exist" << std::endl;
					update_best_plate(m_frame, corresponding_current_det, m_current_confidence, ""/*save_dir*/);
					m_previous_det = m_current_det;
					system("pause");

				}


				counter = 0;
			}

		}
		// no plates in bounding rectangle
		if (detection_mode == 1){
			
			if (m_previous_det.left() != STUPID_CONSTANT_FOR_RESETTING_RECTANGLE && (++counter > 10) ){
				std::cout << "no plates reset prev det" << std::endl;
				result = get_best_plate();
				reset_nomotion();
				counter = 0;
			}




		}

	}

	else{

		// clear old data , active motion detector
		if (detection_mode == 0){

			if (m_previous_det.left() != STUPID_CONSTANT_FOR_RESETTING_RECTANGLE && (++counter > 10)){
				std::cout << "no plates reset prev det" << std::endl;
				result = get_best_plate();
				reset_nomotion();
				counter = 0;
			}

		}

	}

	// each frame clear old rectangles
	m_md.clear_bounding_rectangles();
	cv_bounding_rectangles.clear();

	return result;

}

