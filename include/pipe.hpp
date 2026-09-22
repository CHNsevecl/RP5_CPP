#ifndef __PIPE_HPP__
#define __PIPE_HPP__

#include <iostream>
#include <thread>
#include <chrono>

class Pipe{
public:
    double speed = 0.0;
    double target = 0.0;
    double other = 0.0;

    Pipe();
    void start();
    void process();
    void stop();

    bool running;
};

#endif