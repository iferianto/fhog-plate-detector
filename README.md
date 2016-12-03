# Plate Detection using Histogram of Oriented Gradients 

## Prerequisites:
  - Libraries
      1. [dlib] (http://dlib.net/)
      2. [opencv] (http://opencv.org/downloads.html) 
  
  - Additional Resources
      1. Pretrained SVM files for license plates. You can use dlib's [object detector](http://dlib.net/fhog_object_detector_ex.cpp.html) to create such files. In this case you need to collect license plate samples.
      2. For testing you need a sample video or an RTSP stream.  


## 1. Overview

![Overview](http://image.prntscr.com/image/47595346625f41838f94134f382bb97e.png)

## Pre-Processing
The first step is to convert the BGR* color space to grayscale since the detector works on just gradients.  No color information is needed.
Processing high resolution images are time consuming and CPU expensive, therefore the next step is to resize the input frame. Also we don’t lose any important information regarding a possible motion in the frame

The last step is using Gaussian Blur with kernel 5x5 in order to reduce the noise and unnecessary details. I preferred Gaussian over median, mean or bilateral filters because it performs faster than others and edge preservation was not too important.

# Detecting Motion
Before running the HOG detectors, we need to reduce the region of interest as possible. Otherwise, detecting a plate in the whole frame would take more time which is not suitable for a real-time application.
I decided to use OpenCV’s MOG2 background subtraction method among MOG-MOG2-GMG since MOG2 is more sensitive to shadows.
Morphological close operation with 5x5 structuring element is applied in order to fill the gaps in the image.
Since the detector’s interest is automobiles, I picked the bounding rectangles that satisfies certain conditions like min-max height/width and aspect ratio. Therefore, if a pedestrian/cat/bird enters to the frame the motion detector will not notify the plate detector. 

Properties of bounding rectangles (area, width, height) can be affected by the camera’s positioning. Therefore, minimum and maximum values are in MotionDetectorConfig file which can be handled manually during installation. 

If user wants to focus only on a specific region (like just one traffic lane), he/she can declare the region’s coordinates in MotionDetectorConfig file.

Background subtraction and other operations are not needed in night mode. A simple thresholding can reduce the region of interests significantly.  



# Detecting Plates
![Detecting Plates](http://image.prntscr.com/image/304ef94fc4ce43549f595e6a722ec7ef.png)

Since the Optical Character Recognition (OCR) is much slower than detection, we need to perform a very effective detection:
-	~23 fps
-	Avoid duplicate plates
-	Pick the best view of the plate for OCR       

We already reduced the region of interest by performing motion detection. Instead of processing whole frame (1280x720 pixels), we just run the detector on the bounding rectangle (~300x300 pixels). 

Dlib’s evaluate_detectors function returns the detection rectangles and detection confidences in the given bounding rectangle.

After selecting the most confident rectangle, tracking must be performed since we don’t want to send the same plate to OCR. 

The first detection rectangle includes unnecessary information besides the plate itself. Eventually, we will send this rectangle unless there is a better rectangle. 

After the first detection, in each frame we calculate the Euclidian distance between last two detected plates. If the distance is less than a threshold (~50 pixel), we see that these plates belong to the same automobile. Therefore, we just compare their detection confidence levels and pick the most confident one.
The last step is to find corresponding detection rectangle in the HD frame. I extract rectangle from the HD frame in order to send better quality images to OCR module.

## Useful Links:
  1. [Dalal's HOG Paper](https://lear.inrialpes.fr/people/triggs/pubs/Dalal-cvpr05.pdf)
  2. [Wikipedia HOG] (https://en.wikipedia.org/wiki/Histogram_of_oriented_gradients)


  
