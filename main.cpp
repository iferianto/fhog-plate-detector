#include "PlateDetector.h"
#include <dlib/gui_widgets.h> // image_window
#include <chrono>


int main(){

	// Start video capture
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
	std::string video_directory = "C:/Users/s_madeligoz/Desktop/KOTON/";

	std::vector<cv::String> filenames;
	cv::String folder = video_directory;
	glob(folder, filenames);

	//std::string video_directory = "C:/Users/raistlin/Desktop/";
	
	//std::string video_directory = "C:/Users/s_madeligoz/Desktop/PlateRecognition/SampleVideos/";
	PlateDetector pd;
	dlib::image_window win;
	// read all files in KOTON directory
	for (size_t i = filenames.size()-4; i < filenames.size(); ++i){

		cv::VideoCapture video(filenames[i]);
		if (!video.isOpened()){
			std::cout << "Cannot open the video file" << std::endl;
			return -1;
		}

		cv::Mat frame;
		
		
		while (1){
			try{
				std::cout << "\r frame: " << video.get(CV_CAP_PROP_POS_FRAMES) << std::flush;
				video >> frame;
				cv::imshow("frame", frame);
				dlib::array2d<uchar> result = pd.detect(frame, 0);

				// if result is not empty, show it
				if (result.size()){
					win.set_image(result);
					system("pause");
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



	}



	return 0;
}
