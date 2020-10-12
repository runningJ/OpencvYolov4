/*
 * @Author: Lei Jiang
 * @Email: leijiang420@163.com
 * @Date: 2020-09-17 16:02:31
 * @Description: code description
 * 
 */
#ifndef VIDEOSYSTEM_SRC_CORE_TIME_STATISTIC_H
#define VIDEOSYSTEM_SRC_CORE_TIME_STATISTIC_H
#include <chrono>
class TimeStatistic
{
public:
    TimeStatistic();
    long ShowTime();

private:
    std::chrono::high_resolution_clock::time_point time_start;
};
#endif