#include "PlateDetector.h"
#include <chrono>

int main(){

	// Start video capture
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

	std::string video_directory = "C:/Users/raistlin/Desktop/";
	cv::VideoCapture video(video_directory + "gece.mp4"); // 20160819 
	if (!video.isOpened()){
		std::cout << "Cannot open the video file" << std::endl;
		return -1;
	}

	cv::Mat frame;
	PlateDetector pd;
	dlib::image_window win;
	while (1){
		try{
			video >> frame;
			cv::imshow("frame", frame);
			dlib::array2d<uchar> result = pd.detect(frame);
			
			// if result is not empty, show it
			if (result.size()){
				win.set_image(result);
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
