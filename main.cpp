//#include "PlateDetector.h"
//#include <dlib/gui_widgets.h> // image_window
//#include <chrono>
//
//
//int main(){
//
//	// Start video capture
//	
//	std::string video_directory = "C:/Users/s_madeligoz/Desktop/KOTON/";
//	std::string save_directory = "C:/Users/s_madeligoz/Desktop/KOTON/Plates/";
//
//	std::vector<cv::String> filenames;
//	cv::String folder = video_directory;
//	glob(folder, filenames);
//
//	//std::string video_directory = "C:/Users/raistlin/Desktop/";
//
//	//std::string video_directory = "C:/Users/s_madeligoz/Desktop/PlateRecognition/SampleVideos/";
//	PlateDetector pd;
//	dlib::image_window win;
//	// read all files in KOTON directory
//	for (size_t i = 64; i < filenames.size(); ++i){
//		std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
//
//		cv::VideoCapture video(filenames[i]);
//		if (!video.isOpened()){
//			std::cout << "Cannot open the video file" << std::endl;
//			return -1;
//		}
//
//		cv::Mat frame;
//
//
//		while (1){
//			try{
//				//std::cout << "\r frame: " << video.get(CV_CAP_PROP_POS_FRAMES) << std::flush;
//				video >> frame;
//				//cv::imshow("frame", frame);
//				dlib::array2d<uchar> result = pd.detect(frame);
//
//				// if result is not empty, show it
//				if (result.size()){
//					std::ostringstream output_name;
//					output_name << i;
//					output_name << "_";
//					output_name << video.get(CV_CAP_PROP_POS_MSEC);
//					
//					std::string str = output_name.str();
//					
//					dlib::save_jpeg(result, save_directory + str + ".jpg");
//					win.set_image(result);
//					std::cout << "Plate Found\n";
//					
//				}
//
//
//				std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
//				int timeElapsed = std::chrono::duration_cast<std::chrono::seconds>(end - begin).count();
//
//			/*	if (timeElapsed > 0)
//					std::cout << "\r" << video.get(CV_CAP_PROP_POS_FRAMES) / timeElapsed << " FPS" << std::flush;
//*/
//
//				// read frames until the video ends
//				if (video.get(CV_CAP_PROP_POS_FRAMES) == video.get(CV_CAP_PROP_FRAME_COUNT) - 1){
//					std::cout << "No more frames!" << std::endl;
//					break;
//				}
//				if (cv::waitKey(30) >= 0) break;
//
//			}
//			catch (std::exception &e){
//				std::cout << e.what() << std::endl;
//			}
//		}
//
//
//
//	}
//
//
//
//	return 0;
//}