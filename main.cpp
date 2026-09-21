#include "GPIO.hpp"
#include <chrono>
#include <thread>
#include <iostream>
#include <atomic>
#include <csignal>

static std::atomic<bool> g_running{true};

static void on_sigint(int) {
    g_running = false;   // 只设置标志，不做别的
}

int main() {
    // 注册 Ctrl+C 处理
    std::signal(SIGINT, on_sigint);

    GPIO gpio;
    if (!gpio.setup(3, GPIO::Direction::Out)) {
        std::cerr << "Failed to setup GPIO\n";
        return 1;
    }

    while (g_running) {
        gpio.output(3, GPIO::Level::High);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        gpio.output(3, GPIO::Level::Low);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    // 退出前把引脚拉低，再清理
    gpio.output(3, GPIO::Level::Low);
    gpio.cleanup();
    std::cout << "cleanup 完成\n";
    return 0;
}