#include <iostream>
#include <thread>
#include <chrono>
#include "src/QD4310.hpp"


#define addr1 0x01
#define addr2 0x02

int main(){
    QD4310 motor;
    motor.QD4310_Contol(addr1, QD4310_MODE_ENABLE, 0);
    motor.QD4310_Contol(addr2, QD4310_MODE_ENABLE, 0);
    // motor.QD4310_Contol(addr1, QD4310_MODE_ANGLE, motor.rad(0.0));
    motor.QD4310_Contol(addr2, QD4310_MODE_SPEED, (int)32767);
    motor.QD4310_Contol(addr1, QD4310_MODE_SPEED, (int)-5000);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    while (true) {
        // motor.QD4310_Contol(addr1, QD4310_MODE_REPORT, 0);
        // std::cout << std::dec << "电机1:"<< motor.feedback.angle.value_or(0) << std::endl;
        // motor.QD4310_Contol(addr2, QD4310_MODE_REPORT, 0);
        // std::cout << std::dec << "电机2:"<< motor.feedback.angle.value_or(0) << std::endl;
        // std::cout << motor.feedback.elc_current.value_or(0) << std::endl;
        // std::cout << "C++ 标准版本: " << __cplusplus << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    return 0;
}