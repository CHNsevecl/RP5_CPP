#ifndef ZMQ_PUBLISHER_HPP
#define ZMQ_PUBLISHER_HPP

#include <zmq.hpp>
#include "opencv4/opencv2/opencv.hpp"
#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <chrono>

class ZmqPublisher {
public:
    ZmqPublisher(const std::string& address = "tcp://*:5556", int io_threads = 1);
    ~ZmqPublisher();

    // 禁止拷贝构造和赋值(配置保护)
    ZmqPublisher(const ZmqPublisher&) = delete;
    ZmqPublisher& operator=(const ZmqPublisher&) = delete;

    // 初始化连接
    bool initialize();
    
    // 发送图像 (OpenCV Mat格式)
    bool sendImage(const cv::Mat& image, int quality = 80);
    
    // 发送原始字节数据
    bool sendRawData(const std::vector<unsigned char>& data);
    
    // 发送字符串消息
    bool sendString(const std::string& message);
    
    // 设置图像编码质量 (1-100)
    void setImageQuality(int quality);
    
    // 获取连接状态
    bool isConnected() const;
    
    // 获取发布地址
    std::string getAddress() const;
    
    // 设置发布主题 (用于多主题发布)
    void setTopic(const std::string& topic);

private:
    // ZMQ上下文和socket
    std::unique_ptr<zmq::context_t> context_;
    std::unique_ptr<zmq::socket_t> socket_;
    
    // 配置参数
    std::string address_;
    int io_threads_;
    int image_quality_;
    std::string topic_;
    
    // 状态标志
    bool initialized_;
    bool connected_;
    
    // 内部辅助函数
    bool encodeImage(const cv::Mat& image, std::vector<unsigned char>& buffer, const std::string& format = ".jpg", int quality = 80);
    
    // 发送多部分消息的辅助函数
    // bool sendMultipartMessage(const std::vector<zmq::message_t>& parts);
    
    // 日志函数
    void log(const std::string& message) const;

};

#endif
