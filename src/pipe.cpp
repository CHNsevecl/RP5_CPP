#include "pipe.hpp"

Pipe::Pipe() {
    running = false;
}

void Pipe::start() {
    running = true;
   
}

void Pipe::process(){
    std::string line;
    while (std::getline(std::cin, line) && running) {
        std::cout << "got: " << line << std::endl;

        size_t pos = line.find(':');
        if(pos == std::string::npos){
            std::cerr << "无分隔符." << std::endl;
            continue;
        }

        if(line.substr(0, pos) == "speed"){
            try {
                speed = std::stod(line.substr(pos + 1));
            } catch (const std::exception& e) {
                // 处理错误
                std::cerr << "解析失败: " << e.what() << std::endl;
            }
        } 
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }   
}

void Pipe::stop(){
    running = false;
}