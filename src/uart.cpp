#include "uart.hpp"
#include <algorithm>
#include <cctype>
#include <vector>
#include <string>

void info_to_MCU(UART& uart, const std::vector<std::string>& info) {

    std::string messages = "";
    for (size_t i = 0; i < info.size(); i++) {
        if (i > 0){messages += " ";}
        messages += info[i];
    }
    
    messages = messages + "\n";
    uart.send_string(messages);
    usleep(2500); // 2.5ms
}



std::vector<std::string> UART::receive_string() {
    if (fd == -1) {
        return {};
    }

    std::string data;
    char buffer[256];

    while (true) {
        int available = 0;
        if (ioctl(fd, FIONREAD, &available) == -1 || available <= 0) {
            break;
        }

        int chunk_size = std::min(available, static_cast<int>(sizeof(buffer)));
        ssize_t length = read(fd, buffer, chunk_size);
        if (length > 0) {
            data.append(buffer, length);
        }

        if (length <= 0 || length < chunk_size) {
            break;
        }
    }

    std::vector<std::string> result;
    std::string token;

    for (unsigned char ch : data) {
        if (std::isalnum(ch) || ch == '_') {
            token.push_back(static_cast<char>(ch));
            continue;
        }

        if (!token.empty()) {
            result.push_back(token);
            token.clear();
        }
    }

    if (!token.empty()) {
        result.push_back(token);
    }

    return result;
}

std::vector<uint8_t> UART::receive_uint8_vector(size_t len, int timeout_ms) {
    if (fd == -1) {
        return {};
    }

    std::vector<uint8_t> datas;
    datas.reserve(len);  // 预分配但不固定大小
    
    size_t total_read = 0;
    auto start = std::chrono::steady_clock::now();
    
    // 设置串口为非阻塞或使用 select/poll 检查数据
    while (total_read < len) {
        auto elapsed = std::chrono::steady_clock::now() - start;
        auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
        if (elapsed_ms >= timeout_ms) {
            if (total_read > 0) {
                std::cout << "超时，已读取 " << total_read << " 字节" << std::endl;
                return datas;  // 返回已读取的数据
            }
            std::cout << "超时，无数据" << std::endl;
            return {};
        }
        
        // 使用 select 检查是否有数据可读（避免阻塞）
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(fd, &read_fds);
        struct timeval tv = {0, 10000};  // 10ms 超时
        int ret = select(fd + 1, &read_fds, NULL, NULL, &tv);
        
        if (ret > 0 && FD_ISSET(fd, &read_fds)) {
            uint8_t buffer[256];
            ssize_t n = ::read(fd, buffer, sizeof(buffer));
            if (n > 0) {
                // 将读取的数据追加到 vector
                datas.insert(datas.end(), buffer, buffer + n);
                total_read += n;
                // std::cout << "读取到 " << n << " 字节，累计 " << total_read << " 字节" << std::endl;
            } else if (n == -1) {
                std::cout << "读取错误: " << strerror(errno) << std::endl;
                return {};
            }
        }
    }
    
    // 如果读取到 len 字节，截断到 len
    if (datas.size() > len) {
        datas.resize(len);
    }
    return datas;
}