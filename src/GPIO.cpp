#include "GPIO.hpp"
#include <gpiod.h>
#include <map>
#include <string>
#include <cstring>

struct GPIO::Impl{
    gpiod_chip* chip = nullptr;
    std::string chipname;

    struct LineInfo{
        gpiod_line_request* request = nullptr;
        Direction dir;
    };

    std::map<int, LineInfo> lines;

    std::string last_error;

    ~Impl(){
        release_all();
        if(chip){
            gpiod_chip_close(chip);
            chip = nullptr;
        }
    }

    void release_all(){
        for(auto& kv:lines){
            if (kv.second.request){
                gpiod_line_request_release(kv.second.request);
                kv.second.request = nullptr;
            }
        }
        lines.clear();
    }
};

static gpiod_chip* find_header_chip(std::string& name_out){
    for (int i = 0; i <21; ++i){
        std::string name = "gpiochip" + std::to_string(i);
        std::string path = "/dev/" + name;
        gpiod_chip* chip = gpiod_chip_open(path.c_str());
        if(!chip) continue;

        gpiod_line_info* info2 = gpiod_chip_get_line_info(chip, 2);
        gpiod_line_info* info27 = gpiod_chip_get_line_info(chip, 27);

        bool match = false;
        if(info2 && info27){
            const char* n2 = gpiod_line_info_get_name(info2);
            const char* n27 = gpiod_line_info_get_name(info27);
            if(n2 && n27 && std::strcmp(n2, "GPIO2") == 0 && std::strcmp(n27, "GPIO27") == 0){
                match = true;
            }
        }

        if(info2) gpiod_line_info_free(info2);
        if(info27) gpiod_line_info_free(info27);

        if(match){
            name_out = name;
            return chip;
        }
        gpiod_chip_close(chip);
    }
    return nullptr;
}

GPIO::GPIO(const std::string& chipname)
: impl_(std::make_unique<Impl>())
{
    if(!chipname.empty()){
        std::string path = "/dev/" + chipname;
        impl_->chip = gpiod_chip_open(path.c_str());
        impl_->chipname = chipname;

        if(!impl_ ->chip){
            impl_->last_error = "无法打开芯片: " + chipname;
        }
    }
    else{
        impl_->chip = find_header_chip(impl_->chipname);
        if(!impl_->chip){
            impl_->last_error = "无法找到头文件芯片";
        }
    }
}

GPIO::~GPIO(){
    // impl_ 会自动释放资源
}

bool GPIO::setup(int bcm_pin ,Direction dir){
    if(!impl_->chip){
        impl_->last_error = "chip打开失败";
        return false;
    }

    if(bcm_pin < 0 || bcm_pin > 27){
        impl_->last_error = "无效的BCM引脚号: " + std::to_string(bcm_pin);
        return false;
    }

    auto it = impl_->lines.find(bcm_pin);
    if(it != impl_->lines.end()){
        if(it -> second.request){
            gpiod_line_request_release(it->second.request);
            it->second.request = nullptr;
        }
        impl_->lines.erase(it);
    }

    // 1) line settings：设置方向
    gpiod_line_settings* settings = gpiod_line_settings_new();
    if(!settings){
        impl_->last_error = "无法创建line设置";
        return false;
    }

    gpiod_line_settings_set_direction(
        settings,
        dir == Direction::Out ? GPIOD_LINE_DIRECTION_OUTPUT : GPIOD_LINE_DIRECTION_INPUT);
    
     // 2) line config：指定要请求哪条线
    gpiod_line_config* lcfg = gpiod_line_config_new();
    if(!lcfg){
        gpiod_line_settings_free(settings);
        impl_->last_error = "无法创建line config";
        return false;
    }

    unsigned int offset = static_cast<unsigned int>(bcm_pin);
    if(gpiod_line_config_add_line_settings(lcfg, &offset, 1, settings) < 0){
        gpiod_line_settings_free(settings);
        gpiod_line_config_free(lcfg);
        impl_->last_error = "无法添加line设置";
        return false;
    };

     // 3) request config：consumer 名字
    gpiod_request_config* rcfg = gpiod_request_config_new();
    if (!rcfg) {
        gpiod_line_settings_free(settings);
        gpiod_line_config_free(lcfg);
        impl_->last_error = "创建 request config 失败";
        return false;
    }
    gpiod_request_config_set_consumer(rcfg, "gpio_cpp");

    // 4) 发起请求
    gpiod_line_request* req = gpiod_chip_request_lines(impl_->chip, rcfg, lcfg);

    // 5) 释放中间对象
    gpiod_line_settings_free(settings);
    gpiod_line_config_free(lcfg);
    gpiod_request_config_free(rcfg);

    if (!req) {
        impl_->last_error = "请求 line " + std::to_string(bcm_pin) + " 失败";
        return false;
    }

    Impl::LineInfo info;
    info.request = req;
    info.dir = dir;
    impl_->lines[bcm_pin] = info;

    return true;
}

bool GPIO::output(int bcm_pin, Level level){
    if (!impl_) return false;
    auto it = impl_->lines.find(bcm_pin);
    if(it == impl_->lines.end()){
        impl_->last_error =  "引脚 "+std::to_string(bcm_pin) + " 未初始化";
        return false;
    }
    if(it->second.dir != Direction::Out){
        impl_->last_error = "引脚 "+std::to_string(bcm_pin) + " 不是输出方向";
        return false;
    }

    int v = (level == Level::High) ? 1 : 0;
    int ret = gpiod_line_request_set_value(it->second.request, static_cast<unsigned int>(bcm_pin), static_cast<gpiod_line_value>(v));

    if(ret < 0){
        impl_->last_error = "设置引脚 "+std::to_string(bcm_pin) + " 输出值失败";
        return false;
    }

    return true;
}

int GPIO::input(int bcm_pin){
    if(!impl_) return -1;
    auto it = impl_->lines.find(bcm_pin);
    if(it == impl_->lines.end()){
        impl_->last_error = "引脚 "+std::to_string(bcm_pin) + " 未初始化";
        return -1;
    }
    if(it->second.dir != Direction::In){
        impl_->last_error = "引脚 "+std::to_string(bcm_pin) + " 不是输入方向";
        return -1;
    }

    int v = gpiod_line_request_get_value(it->second.request, static_cast<unsigned int>(bcm_pin));
    if(v < 0){
        impl_->last_error = "读取引脚 "+std::to_string(bcm_pin) + " 输入值失败";
        return -1;
    }
    return v;
}

void GPIO::cleanup(){
    if(!impl_) return;
    impl_->release_all();
}

std::string GPIO::get_last_error() const{
    if(!impl_) return "对象无效";
    return impl_->last_error;
}