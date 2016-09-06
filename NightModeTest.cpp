#include "PlateDetector.h"
#include <chrono>
#include <dlib/gui_widgets.h> // image_window
int main(){

	// Start video capture
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
	
	std::string video_directory = "rtsp://10.0.5.155/live1.sdp";
	std::string save_directory = "C:/Users/s_madeligoz/Desktop/rtsp155/";

	cv::VideoCapture video(video_directory); // 20160819 gunduz gece
	if (!video.isOpened()){
		std::cout << "Cannot open the video file" << std::endl;
		return -1;
	}

	std::cout << "Stream has started" << std::endl;

	cv::Mat frame;
	PlateDetector pd;
	dlib::image_window win;
	while (1){
		try{
			video >> frame;
			//cv::imshow("frame", frame);
			
			dlib::array2d<uchar> result = pd.detect(frame);
			
			// if result is not empty, show it
			if (result.size()){
				std::ostringstream output_name;
				output_name << video.get(CV_CAP_PROP_POS_MSEC);
								
				std::string str = output_name.str();
								
				dlib::save_jpeg(result, save_directory + str + ".jpg");
				win.set_image(result);
				std::cout << "Plate Found\n";
								
			}
			

			// read frames until the video ends
			if (video.get(CV_CAP_PROP_POS_FRAMES) == video.get(CV_CAP_PROP_FRAME_COUNT) - 1){
				std::cout << "No more frames!" << std::endl;
				break;
			}
			if (cv::waitKey(30) >= 0) break;

		}
		catch (std::exception &e){
			std::cout << e.what() << std::endl;
		}
	}
	return 0;
}
