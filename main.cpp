#include "uart.hpp"
#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <cstdint>

int main()
{
    UART uart("/dev/ttyAMA0", 115200);

    if (!uart.open())
    {
        std::cerr << "UART open failed" << std::endl;
        return 1;
    }

    std::cout << "UART opened successfully" << std::endl;

    // =========================
    // 1. 发送字符串
    // =========================
    while (true) {
        uint8_t buffer[100];
        int len = uart.receive(buffer, sizeof(buffer), 500);
        if(len <= 0) {
            uart.send("Hello UART!\r\n");
        }
        else{
            std::cout << "Received data: ";
            for (int i = 0; i < len; ++i) {
                std::cout << static_cast<char>(buffer[i]);
            }
            std::cout << std::endl;
        }
        
        // std::this_thread::sleep_for(std::chrono::seconds(1));
   
    }
    // =========================
    // 2. 接收数据
    // =========================

    // uint8_t buffer[100];

    // int n = uart.receive(buffer, sizeof(buffer), 1000);

    // if (n < 0)
    // {
    //     std::cerr << "UART receive failed" << std::endl;
    // }
    // else if (n == 0)
    // {
    //     std::cout << "Receive timeout" << std::endl;
    // }
    // else
    // {
    //     std::cout << "Received " << n << " bytes: ";

    //     for (int i = 0; i < n; ++i)
    //     {
    //         std::cout << static_cast<char>(buffer[i]);
    //     }

    //     std::cout << std::endl;
    // }

    // =========================
    // 3. 关闭 UART
    // =========================

    uart.close();

    return 0;
}