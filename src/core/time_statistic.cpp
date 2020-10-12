/*
 * @Author: Lei Jiang
 * @Email: leijiang420@163.com
 * @Date: 2020-09-17 16:09:32
 * @Description: code description
 */
#include "time_statistic.h"
using namespace std;

TimeStatistic::TimeStatistic()
{
    time_start = chrono::high_resolution_clock::now();
}
long TimeStatistic::ShowTime()
{
    auto current_time = chrono::high_resolution_clock::now();
    auto cost_time = chrono::duration_cast<chrono::milliseconds>(current_time - time_start);
    long duration_time = static_cast<long>(cost_time.count());
    return duration_time;
}