#ifndef F32C_HPP
#define F32C_HPP

#include <stdint.h>
#include <thread>
#include <stddef.h>
#include <vector>
#include <optional>
#include <string>
#include <stdio.h>
#include <iostream>
#include "uart.hpp"


/*!
 * \brief F32C类，用于控制F32C电机
 * \brief 该类提供了对F32C电机的初始化、使能、控制模式选择、速度设置、角度设置、加速度设置、参数保存、零点设置、恢复出厂设置等功能的封装。
 * \brief 使用位置控制前，先设置速度（若初始化过速度可省略）
 */
class F32C {
protected:
    UART uart_f32c;
    void F32C_SendData(const std::vector<uint8_t>& data);
    std::vector<uint8_t> F32C_ReceiveData(uint8_t start=0x7A,uint8_t end=0x7B,size_t length=9, uint32_t timeout_ms=10);
    uint8_t Bcc(const std::vector<uint8_t>& data);

public:
    void F32C_Init();
    void F32C_Enable(uint8_t addr);
    void F32C_Disable(uint8_t addr);
    void Control_Mode_Chose(uint16_t mode, uint8_t addr);
    void Speed_set(uint16_t speed, uint8_t addr);
    void Multiple_Angle_set(uint32_t position, uint8_t addr);
    void Single_Angle_set(uint16_t position, uint8_t addr);
    void Acceleration_set(uint16_t acc, uint8_t addr);
    void Parameter_Save(uint8_t addr);
    void Zero_Position(uint8_t addr);
    void Single_Angle_Zero_Position(uint8_t addr);
    void Factory_Reset(uint8_t addr);
    void Address_Set(uint8_t addr, uint8_t new_addr);
    void Data_Feedback(uint8_t addr, uint8_t data_type);
};

#endif // F32C_HPP