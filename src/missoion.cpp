#include "mission.hpp"

void mission1(){
    Pipe pipe;
    pipe.start();
    pipe.process();

}

void mission2(){
    setenv("DISPLAY",":0", 1);

    ZmqPublisher publisher("tcp://*:5555");
    publisher.setTopic("image");
    publisher.setImageQuality(80);

    if (!publisher.initialize()) {
        std::cerr << "[MAIN] initialize failed" << std::endl;
        return;
    }

     std::string pipeline =
        "libcamerasrc camera-name=/base/axi/pcie@1000120000/rp1/i2c@88000/imx708@1a ! "
        "video/x-raw, format=NV12, width=640, height=480 ! "
        "videoconvert ! "
        "video/x-raw, format=BGR ! "
        "appsink drop=true max-buffers=1";

    cv::VideoCapture cap(pipeline, cv::CAP_GSTREAMER);
    if (!cap.isOpened()) {
        std::cerr << "Failed to open camera" << std::endl;
        return;
    }

    while(1){
        cv::Mat frame;
        cap >> frame;
        if(frame.empty()){
            continue;
        }
        
        if (!publisher.sendImage(frame)) {
            std::cerr << "[MAIN] sendImage failed" << std::endl;
        }

        // cv::imshow("frame",frame);
        // if (cv::waitKey(1) == 27) {
        //     break;
        // }
    }
}