#ifndef __GPIO__
#define __GPIO__

#pragma once
#include <string>
#include <memory>

class GPIO{
public:
    enum class Direction{ In , Out};
    enum class Level{ Low , High};

    explicit GPIO(const std::string& chipname = "");    //构造函数（如何创建）
    ~GPIO();                                            //析构函数（如何销毁）

    GPIO(const GPIO&) = delete;                         //禁用拷贝构造
    GPIO& operator= (const GPIO&) = delete;             //禁用赋值操作符
    GPIO(GPIO&&) noexcept;                              //移动构造函数 
    GPIO& operator=(GPIO&&) noexcept;                   //移动赋值操作符

    bool setup(int bcm_pin, Direction dir);

    bool output(int bcm_pin, Level level);
    int input(int bcm_pin);

    void cleanup();

    std::string get_last_error() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    
};

#endif