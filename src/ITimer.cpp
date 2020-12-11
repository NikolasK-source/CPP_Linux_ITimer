/*
 * \file ITimer.cpp
 * \brief Source file de::Koesling::Threading::ITimer
 *
 * required compiler options:
 *          -std=c++11 (or higher)
 *
 * recommended compiler options:
 *          -O2
 *
 * Copyright (c) 2020 Nikolas Koesling
 *
 */

#include "ITimer.hpp"
#include "sysexcept.hpp"
#include "destructor_exception.hpp"
#include <cmath>
#include <limits>
#include <iostream>
#include <sysexits.h>

//! timeval to stop timer
static constexpr itimerval STOP_TIMER = {{0, 0}, {0, 0}};

static constexpr auto _inf = std::numeric_limits<double>::infinity();
static constexpr auto _nan = std::numeric_limits<double>::quiet_NaN();

#define USEC_PER_SEC 1000000

namespace de {
namespace Koesling {
namespace ITimer {

std::ostream* ITimer::error_stream = &std::cerr;

bool ITimer_Real::instance_exists = false;
bool ITimer_Virtual::instance_exists = false;
bool ITimer_Prof::instance_exists = false;

void ITimer::adjust_speed(double new_factor)
{
    // not running? --> no time adjustment possible
    if(!running) throw std::runtime_error(std::string(__PRETTY_FUNCTION__) + ": timer not running!");

    // read current timer value
    itimerval val;
    sysexcept(setitimer(type, &STOP_TIMER, &val) < 0, "setitimer", errno);

    // set timer interval
    val.it_interval = timer_interval / new_factor;

    // scale timer value
    val.it_value *= speed_factor / new_factor;

    // set new timer value
    sysexcept(setitimer(type, &val, nullptr), "setitimer", errno);
}

ITimer::ITimer(int type, const timeval &interval) noexcept :
        timer_value(interval), timer_interval(interval), type(type),
        speed_factor(1.0),  // normal speed
        running(false)      // not running
{
}

ITimer::ITimer(int type, const timeval &interval,
        const timeval &value) noexcept :
        timer_value(value), timer_interval(interval), type(type),
        speed_factor(1.0),  // normal speed
        running(false)      // not running
{
}

ITimer::~ITimer( )
{
    if(running) // stop timer if running
    {
        try
        {
            stop();
        }
        catch (const std::system_error& e)
        {
            destructor_exception_terminate(e, *error_stream, EX_OSERR);
        }
    }
}

void ITimer::start( )
{
    if(running) throw std::logic_error(std::string(__PRETTY_FUNCTION__) + ": timer already started");

    // create scaled timer value
    itimerval timer_val;
    timer_val.it_interval = timer_interval / speed_factor;
    timer_val.it_value = timer_value / speed_factor;

    if(timer_val.it_interval.tv_sec == 0 && timer_val.it_interval.tv_usec == 0)
        throw std::runtime_error(std::string(__PRETTY_FUNCTION__) + 
                ": invalid timer values due to to a to small speed factor");

    //start timer;
    sysexcept(setitimer(type, &timer_val, nullptr) < 0, "setitimer", errno);

    running = true;
}

void ITimer::stop( )
{
    if(!running) throw std::runtime_error(std::string(__PRETTY_FUNCTION__) + ": timer already stopped");

    // stop timer and save value
    itimerval timer_val;
    int temp = setitimer(type, &STOP_TIMER, &timer_val);
    sysexcept(temp < 0, "setitimer", errno);

    // normalize value
    timer_value = timer_val.it_value * speed_factor;

    running = false;
}

// check for nan and inf --> disable direct float equal check warning
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
void ITimer::set_speed_factor(const double speed_factor)
{
    // check speed_factor
    if(speed_factor <= 0.0) 
    throw std::invalid_argument(std::string(__PRETTY_FUNCTION__) + ": Negative values not allowed!");

    if(speed_factor == _inf || speed_factor == _nan)
        throw std::invalid_argument(std::string(__PRETTY_FUNCTION__) + ": invalid double value!");

    bool running = this->running;

    if(running) stop();

    // save speed factor
    this->speed_factor = speed_factor;

    if(running) start();
}
// re-enable warnings
#pragma GCC diagnostic pop

void ITimer::set_speed_to_normal( )
{
    // adjust speed if running
    if(running)
        adjust_speed(1.0);

    // save speed factor
    speed_factor = 1.0;
}

ITimer_Real::ITimer_Real(const timeval &interval) :
        ITimer(ITIMER_REAL, interval)
{
    // prevent multiple instances
    if(instance_exists)
        throw std::logic_error(std::string(__PRETTY_FUNCTION__) + 
                ": only one interval timer of each type per process possible");

    instance_exists = true;
}

ITimer_Real::ITimer_Real(const timeval &interval,
        const timeval &value) :
        ITimer(ITIMER_REAL, interval, value)
{
    // prevent multiple instances
    if(instance_exists)
        throw std::logic_error(std::string(__PRETTY_FUNCTION__) + 
                ": only one interval timer of each type per process possible");

    instance_exists = true;
}

ITimer_Real::~ITimer_Real( )
{
    // allow new instance
    instance_exists = false;
}

ITimer_Virtual::ITimer_Virtual(const timeval &interval) :
        ITimer(ITIMER_VIRTUAL, interval)
{
    // prevent multiple instances
    if(instance_exists)
        throw std::logic_error(std::string(__PRETTY_FUNCTION__) + 
                ": only one interval timer of each type per process possible");

    instance_exists = true;
}

ITimer_Virtual::ITimer_Virtual(const timeval &interval,
        const timeval &value) :
        ITimer(ITIMER_VIRTUAL, interval, value)
{
    // prevent multiple instances
    if(instance_exists)
        throw std::logic_error(std::string(__PRETTY_FUNCTION__) + 
                ": only one interval timer of each type per process possible");

    instance_exists = true;
}

ITimer_Virtual::~ITimer_Virtual( )
{
    // allow new instance
    instance_exists = false;
}

ITimer_Prof::ITimer_Prof(const timeval &interval) :
        ITimer(ITIMER_PROF, interval)
{
    // prevent multiple instances
    if(instance_exists)
        throw std::logic_error(std::string(__PRETTY_FUNCTION__) + 
                ": only one interval timer of each type per process possible");

    instance_exists = true;
}

ITimer_Prof::ITimer_Prof(const timeval &interval,
        const timeval &value) :
        ITimer(ITIMER_PROF, interval, value)
{
    // prevent multiple instances
    if(instance_exists)
        throw std::logic_error(std::string(__PRETTY_FUNCTION__) + 
                ": only one interval timer of each type per process possible");

    instance_exists = true;
}

ITimer_Prof::~ITimer_Prof( )
{
    // allow new instance
    instance_exists = false;
}

timeval& operator *=(timeval &left, double right) noexcept
{
    double timer_value = timeval_to_double(left) * right;
    left = double_to_timeval(timer_value);
    return left;
}

itimerval& operator *=(itimerval &left, double right) noexcept
{
    left.it_interval *= right;
    left.it_value *= right;
    return left;
}

timeval operator *(const timeval &left, double right) noexcept
{
    auto ret_val = left;
    ret_val *= right;
    return ret_val;
}

itimerval operator *(const itimerval &left, double right) noexcept
{
    auto ret_val = left;
    ret_val *= right;
    return ret_val;
}

timeval& operator /=(timeval &left, double right) noexcept
{
    double timer_value = timeval_to_double(left) / right;
    left = double_to_timeval(timer_value);
    return left;
}

itimerval& operator /=(itimerval &left, double right) noexcept
{
    left.it_interval /= right;
    left.it_value /= right;
    return left;
}

timeval operator /(const timeval &left, double right) noexcept
{
    auto ret_val = left;
    ret_val /= right;
    return ret_val;
}

itimerval operator /(const itimerval &left, double right) noexcept
{
    auto ret_val = left;
    ret_val /= right;
    return ret_val;
}

double timeval_to_double(const timeval &time) noexcept
{
    double ret_val = static_cast<double>(time.tv_sec)+
            static_cast<double>(time.tv_usec) / static_cast<double>(USEC_PER_SEC);
    return ret_val;
}

timeval double_to_timeval(const double time) noexcept
{
    timeval ret_val;
    ret_val.tv_sec = static_cast<time_t>(time);
    ret_val.tv_usec = static_cast<suseconds_t>(fmod(time, 1.0) * static_cast<double>(USEC_PER_SEC));
    return ret_val;
}

void ITimer::to_fstream(std::ofstream &fstream) const
{
    itimerval val;
    if(running)
    {
        sysexcept(getitimer(type, &val) < 0, "getitimer", errno);
        val.it_value *= speed_factor;
    }
    else
    {
        val.it_value = timer_value;
    }

    val.it_interval = timer_interval;

    fstream.write(reinterpret_cast<char*>(&val), sizeof(val));
}

void ITimer::from_fstream(std::ifstream &fstream)
{
    if(running) throw std::logic_error(std::string(__PRETTY_FUNCTION__) + ": timer must be stopped!");

    itimerval val;
    fstream.read(reinterpret_cast<char*>(&val), sizeof(val));
    timer_interval = val.it_interval;
    timer_value = val.it_value;
}

timeval ITimer::get_timer_value() const
{
	if(running)
	{
        itimerval temp;
        sysexcept(getitimer(type, &temp), "getitimer", errno);
        return temp.it_value;
	}
	else
		return timer_value;
}

unsigned long ITimer::getSourceVersion() noexcept
{
    return KOESLINGNI_ITIMER_VERSION;
}

} /* namespace ITimer */
} /* namespace Koesling */
} /* namespace de */

