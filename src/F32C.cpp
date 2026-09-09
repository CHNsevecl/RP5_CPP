#include "F32C.hpp"


#define UART_ID "/dev/ttyAMA0"
#define BAUD_RATE 115200

void F32C::F32C_SendData(const std::vector<uint8_t>& data){
    uart_f32c.send_uint8vector(data);
}
std::vector<uint8_t> F32C::F32C_ReceiveData(uint8_t start,uint8_t end,size_t length, uint32_t timeout_ms){
    std::vector<uint8_t> data;
    data.reserve(length);
    bool start_flag = false;
    data = uart_f32c.receive_uint8_vector();
    
    if(!data.empty()){
        if(data[0] == start && data.back() == end){
            return data;
        }
        else{
            std::cout << "数据帧格式错误"<< std::endl;
            return{};
        }
    }
    else{
        std::cout << "空数据"<<std::endl;
        return {};
    }
}

uint8_t F32C::Bcc(const std::vector<uint8_t>& data) {
    uint8_t bcc = 0;
    for (size_t i = 0; i < data.size(); ++i) {
        bcc ^= data[i];
    }
    return bcc;
}

/*!
 * \brief 初始化F32C
 */
void F32C::F32C_Init(){ 
    if (!uart_f32c.init(UART_ID,BAUD_RATE)){
        std::cout << UART_ID << " " << "初始化错误" << std::endl;
    }
}


/*! \brief 使能F32C
 *  \param addr F32C地址
*/
void F32C::F32C_Enable(uint8_t addr){
    std::vector<uint8_t> command;
    command.reserve(5);

    command.push_back(0x7A); //帧头
    command.push_back(addr); //F32C地址
    command.push_back(0x06); //功能码
    uint8_t bcc = Bcc(command);
    command.push_back(bcc); //校验码
    command.push_back(0x7B); //帧尾

    F32C_SendData(command);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

/*!
 * \brief F32C失能
 * \param addr F32C地址
 */
void F32C::F32C_Disable(uint8_t addr){
    std::vector<uint8_t> command;
    command.reserve(5);

    command.push_back(0x7A); //帧头
    command.push_back(addr); //F32C地址
    command.push_back(0x05); //功能码
    command.push_back(0x7D); //校验码
    command.push_back(0x7B); //帧尾

    F32C_SendData(command);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
}


/*! \brief F32C运动方式
 *  \param mode 
 *  0x0000: 速度模式
 *  0x0001: 多圈位置模式（带T型轨迹规划）
 *  0x0002: 单圈绝对位置模式（带T型轨迹规划）
 *  0x0003: 多圈相对位置模式（直通）
 *  0x0004: 单圈相对位置模式（直通）
 *  \param addr F32C地址
 */
void F32C::Control_Mode_Chose(uint16_t mode, uint8_t addr){
    std::vector<uint8_t> command;
    command.reserve(7);

    command.push_back(0x7A); //帧头
    command.push_back(addr); //F32C地址
    command.push_back(0x00); //功能码
    command.push_back(mode >> 8); //模式高位
    command.push_back(mode & 0xFF); //模式低位
    uint8_t bcc = Bcc(command);
    command.push_back(bcc); //校验码
    command.push_back(0x7B); //帧尾
    if(command.size() == 7){
        F32C_SendData(command);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

/*!
 * \brief 速度模式控制
 * \param speed 速度值，范围0x0000-0xFFFF，速度负值为补码
 * \param addr F32C地址
 */
void F32C::Speed_set(uint16_t speed, uint8_t addr){
    std::vector<uint8_t> command;
    command.reserve(7);

    command.push_back(0x7A); //帧头
    command.push_back(addr); //F32C地址
    command.push_back(0x01); //功能码
    command.push_back(speed >> 8); //速度高位
    command.push_back(speed & 0xFF); //速度低位
    uint8_t bcc = Bcc(command);
    command.push_back(bcc); //校验码
    command.push_back(0x7B); //帧尾

    F32C_SendData(command);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

/*!
 * \brief 多圈绝对角度控制
 * \param position 位置值，共32位数据
 * \param addr F32C地址
 */
void F32C::Multiple_Angle_set(uint32_t position, uint8_t addr){
    std::vector<uint8_t> command;
    command.reserve(9);

    command.push_back(0x7A); //帧头
    command.push_back(addr); //F32C地址
    command.push_back(0x02); //功能码
    command.push_back(position >> 24); //位置高位
    command.push_back((position >> 16) & 0xFF); //位置中高位
    command.push_back((position >> 8) & 0xFF); //位置中低位
    command.push_back(position & 0xFF); //位置低位

    uint8_t bcc = Bcc(command);
    command.push_back(bcc); //校验码
    command.push_back(0x7B); //帧尾
    F32C_SendData(command);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

/*! \brief 单圈绝对角度控制
 *  \param position 位置值，共32位数据
 *  \param addr F32C地址
 */
void F32C::Single_Angle_set(uint16_t position, uint8_t addr){
    std::vector<uint8_t> command;
    command.reserve(7);

    command.push_back(0x7A); //帧头
    command.push_back(addr); //F32C地址
    command.push_back(0x03); //功能码
    command.push_back(position >> 8); //位置高位
    command.push_back(position & 0xFF); //位置低位
    uint8_t bcc = Bcc(command);
    command.push_back(bcc); //校验码
    command.push_back(0x7B); //帧尾
    F32C_SendData(command);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

/*! \brief 加速度控制
*  \param acc 加速度值，范围0x00-0xFF
*  \param addr F32C地址
*/
void F32C::Acceleration_set(uint16_t acc, uint8_t addr){
    std::vector<uint8_t> command;
    command.reserve(7);

    command.push_back(0x7A); //帧头
    command.push_back(addr); //F32C地址
    command.push_back(0x07); //功能码
    command.push_back(acc >> 8); //加速度高位
    command.push_back(acc & 0xFF); //加速度低位
    uint8_t bcc = Bcc(command);
    command.push_back(bcc); //校验码
    command.push_back(0x7B); //帧尾
    F32C_SendData(command);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

/*! \brief  参数保存（加速度、地址）
*/
void F32C::Parameter_Save(uint8_t addr){
    std::vector<uint8_t> command;
    command.reserve(5);
    command.push_back(0x7A); //帧头
    command.push_back(addr); //F32C地址
    command.push_back(0x08); //功能码
    uint8_t bcc = Bcc(command);
    command.push_back(bcc); //校验码
    command.push_back(0x7B); //帧尾
    F32C_SendData(command);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

/*! \brief  电机上电后转过角度清零
*/
void F32C::Zero_Position(uint8_t addr){
    std::vector<uint8_t> command;
    command.reserve(5);
    command.push_back(0x7A); //帧头
    command.push_back(addr); //F32C地址
    command.push_back(0x09); //功能码
    uint8_t bcc = Bcc(command);
    command.push_back(bcc); //校验码
    command.push_back(0x7B); //帧尾
    F32C_SendData(command);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

/*! \brief  单圈绝对角度设置零点。如电机到货时单圈绝对角度 0 度不合适， 可控制电机转动到合适的角度后发送此协议即可将目前该位置设置为单圈绝对角度 0 度，可掉电保存。
*/
void F32C::Single_Angle_Zero_Position(uint8_t addr){
    std::vector<uint8_t> command;
    command.reserve(5);
    command.push_back(0x7A); //帧头
    command.push_back(addr); //F32C地址
    command.push_back(0x0A); //功能码
    command.push_back(0x72); //校验码
    command.push_back(0x7B); //帧尾
    F32C_SendData(command);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

/*! \brief  恢复出厂设置
*/
void F32C::Factory_Reset(uint8_t addr){
    std::vector<uint8_t> command;
    command.reserve(5);
    command.push_back(0x7A); //帧头
    command.push_back(addr); //F32C地址
    command.push_back(0x0B); //功能码
    command.push_back(0x73); //校验码
    command.push_back(0x7B); //帧尾
    F32C_SendData(command);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

/*! \brief 地址设置
 *  \param addr F32C地址
 *  \param new_addr 新地址
 */
void F32C::Address_Set(uint8_t addr, uint8_t new_addr){
    std::vector<uint8_t> command;
    command.reserve(6);
    command.push_back(0x7A); //帧头
    command.push_back(addr); //F32C地址
    command.push_back(0x0D); //功能码
    command.push_back(new_addr); //新地址

    uint8_t bcc = Bcc(command);

    command.push_back(bcc); //校验码
    command.push_back(0x7B); //帧尾
    F32C_SendData(command);
    for(int i = 0; i < command.size(); ++i) {
        std::cout << command[i] << " ";
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

/*! \brief 数据反馈
    *  \param addr F32C地址
    *  \param data_type 数据类型
    *  -0x00: 速度反馈 
    *   -（如反馈数据为：7A 02 00 [00 00 00 64](数据位) 1C 7B 反馈数据为目前电机转速为 0X0064，也就是 100RPM）
    *  -0x01: 多圈(总角度)位置反馈 
    *   -（如反馈数据位：7A 02 01 [00 00 0E 10] 67 7B 反馈数据为目前电机已正转 0X0000E10，也就是 360 度）
    *  -0x02: 单圈(机械角度)位置反馈 
    *   -（如反馈数据为：7A 02 02 [00 00 05 DC] A3 7B 反馈数据为目前电机的机械角度为 0X05DC，也就是 150 度）
    *  -0x03: 电机加速度反馈 
    *   -（如反馈数据为：7A 02 03 [00 00 00 C8] B3 7B 反馈数据为目前电机的加速度为 0X00C8，也就是 100 转/s²）
    *  -0x04: 电机电压反馈 
    *   -（如反馈数据为：7A 02 04 [00 00 04 B0] C8 7B 反馈数据为目前电机的母线电压为 0X04B0，也就是 1200，也就是 12V）
*/
void F32C::Data_Feedback(uint8_t addr, uint8_t data_type){
    std::vector<uint8_t> command;
    command.reserve(6);
    command.push_back(0x7A); //帧头
    command.push_back(addr); //F32C地址
    command.push_back(0x0E); //功能码
    command.push_back(data_type); //数据类型
    uint8_t bcc = Bcc(command);
    command.push_back(bcc); //校验码
    command.push_back(0x7B); //帧尾
    F32C_SendData(command);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    std::vector<uint8_t> response =F32C_ReceiveData(0x7A, 0x7B, 32, 10);

    if(!response.empty()){
        uint32_t value = (uint32_t(response[3]) << 24) | (uint32_t(response[4]) << 16) | (uint32_t(response[5]) << 8) | uint32_t(response[6]);
        std::cout << "Data Feedback (Type: " << int(data_type) << "): " << std::dec << int(value) << std::endl;
    }
}