#include "zmq_publisher.hpp"

ZmqPublisher::ZmqPublisher(const std::string& address, int io_threads)
    : address_(address),
      io_threads_(io_threads),
      image_quality_(80),
      topic_(""),
      initialized_(false),
      connected_(false)
{
    log("ZmqPublisher created with address: " + address_);
}

ZmqPublisher::~ZmqPublisher()
{
    log("ZmqPublisher destroyed");
}

bool ZmqPublisher::initialize()
{
    try{
        context_ = std::make_unique<zmq::context_t>(io_threads_); // 上下文本
        socket_ = std::make_unique<zmq::socket_t>(*context_, zmq::socket_type::pub);
        int sndhwm = 10; // 设置发送高水位(防止爆内存)

        socket_->set(zmq::sockopt::sndhwm, sndhwm);
        
        //立刻关闭，不等待，未发完的消息丢弃
        int linger = 0; 
        socket_->set(zmq::sockopt::linger, linger);

        socket_->bind(address_);
        initialized_ = true;
        connected_ = true;
        log("ZmqPublisher initialized and bound to: " + address_);
        return true;
    }
    catch(const zmq::error_t& e){
        log("ZmqPublisher initialization failed: " + std::string(e.what()));
        initialized_ = false;
        connected_ = false;
        return false;
    }
    catch(const std::exception& e){
        log("ZmqPublisher initialization failed: " + std::string(e.what()));
        initialized_ = false;
        connected_ = false;
        return false;
    }
}

bool ZmqPublisher::sendImage(const cv::Mat& image, int quality)
{
    if (!initialized_ || !socket_) {
        log("sendImage failed: publisher not initialized");
        return false;
    }

    if (image.empty()) {
        log("sendImage failed: empty image");
        return false;
    }

    // 参数校验
    if (quality < 1)   quality = 1;
    if (quality > 100) quality = 100;

    // 编码为 JPEG
    std::vector<uchar> buffer;
    if (!encodeImage(image, buffer, ".jpg", quality)) {
        log("sendImage failed: encode error");
        return false;
    }

    try {
        // ---- 多部分消息：[topic][header][data] ----
        // 1) topic（用于 Python 端 SUB 过滤）
        zmq::message_t topic_msg(topic_.data(), topic_.size());

        // 2) header：图像元信息
        struct Header {
            int32_t rows;
            int32_t cols;
            int32_t type;
            int32_t channels;
            int32_t data_size;
        } header;

        header.rows      = image.rows;
        header.cols      = image.cols;
        header.type      = image.type();
        header.channels  = image.channels();
        header.data_size = static_cast<int32_t>(buffer.size());

        zmq::message_t header_msg(&header, sizeof(header));

        // 3) data：JPEG 字节流
        zmq::message_t data_msg(buffer.data(), buffer.size());

        // 发送
        socket_->send(topic_msg,  zmq::send_flags::sndmore);
        socket_->send(header_msg, zmq::send_flags::sndmore);
        socket_->send(data_msg,   zmq::send_flags::none);

        return true;
    }
    catch (const zmq::error_t& e) {
        log(std::string("ZMQ error in sendImage: ") + e.what());
        connected_ = false;
        return false;
    }
}

bool ZmqPublisher::encodeImage(const cv::Mat& image,std::vector<uchar>& buffer,const std::string& format,int quality)
{
    // 注意：imencode 的第三个参数是 vector<int>，不是 int
    std::vector<int> params;
    if (format == ".jpg" || format == ".jpeg") {
        params.push_back(cv::IMWRITE_JPEG_QUALITY);
        params.push_back(quality);
    }
    else if (format == ".png") {
        params.push_back(cv::IMWRITE_PNG_COMPRESSION);
        // PNG 是 0-9，把 quality(1-100) 映射过去
        params.push_back(9 - (quality * 9 / 100));
    }

    try {
        if (!cv::imencode(format, image, buffer, params)) {
            return false;
        }
        return true;
    }
    catch (const cv::Exception& e) {
        log(std::string("OpenCV encode error: ") + e.what());
        return false;
    }
}

bool ZmqPublisher::sendRawData(const std::vector<uchar>& data)
{
    if (!initialized_ || !socket_) {
        log("sendRawData failed: publisher not initialized");
        return false;
    }

    try {
        zmq::message_t topic_msg(topic_.data(), topic_.size());
        zmq::message_t data_msg(data.data(), data.size());

        socket_->send(topic_msg, zmq::send_flags::sndmore);
        socket_->send(data_msg,  zmq::send_flags::none);
        return true;
    }
    catch (const zmq::error_t& e) {
        log(std::string("ZMQ error in sendRawData: ") + e.what());
        connected_ = false;
        return false;
    }
}

bool ZmqPublisher::sendString(const std::string& message)
{
    if (!initialized_ || !socket_) {
        log("sendString failed: publisher not initialized");
        return false;
    }

    try {
        zmq::message_t topic_msg(topic_.data(), topic_.size());
        zmq::message_t str_msg(message.data(), message.size());

        socket_->send(topic_msg, zmq::send_flags::sndmore);
        socket_->send(str_msg,   zmq::send_flags::none);
        return true;
    }
    catch (const zmq::error_t& e) {
        log(std::string("ZMQ error in sendString: ") + e.what());
        connected_ = false;
        return false;
    }
}

void ZmqPublisher::setImageQuality(int quality)
{
    if (quality < 1)   quality = 1;
    if (quality > 100) quality = 100;
    image_quality_ = quality;
    log("Image quality set to " + std::to_string(image_quality_));
}

bool ZmqPublisher::isConnected() const
{
    return connected_;
}

std::string ZmqPublisher::getAddress() const
{
    return address_;
}

void ZmqPublisher::setTopic(const std::string& topic)
{
    topic_ = topic;
    log("Topic set to: '" + topic_ + "'");
}

void ZmqPublisher::log(const std::string& message) const
{
    std::cout << "[ZmqPublisher] " << message << std::endl;
}

