/*
 * \file ITimer.hpp
 * \brief Header file de::Koesling::Threading::ITimer
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
#pragma once

#include <sys/time.h>
#include <fstream>

#define KOESLINGNI_ITIMER_VERSION 001000000ul    //!< Library version

namespace de {
namespace Koesling {
namespace ITimer {

    /*! Abstract class ITimer
     *
     * General linux interval timer
     */
    class ITimer
    {
        private:
            //! timer value (speed factor 1.0)
            timeval timer_value;

            //! timer interval (speed factor 1.0)
            timeval timer_interval;

            //! timer type (REAL/VIRTUAL/PROF see man getitimer)
            int type;

            /*! \brief speed adjustment factor
             *
             * - ]0;1[   -->  slower
             * - ]1;inf[ -->  faster
             * - 1       -->  normal speed
             */
            double speed_factor;

            //! timer running indicator
            bool running;

            //! internal use only!
            virtual void adjust_speed(double new_factor);

            //! error message stream for "non-throwable" errors
            static std::ostream* error_stream;

        protected:
            //! internal use only!
            ITimer(int type, const timeval &interval) noexcept;
            //! internal use only!
            ITimer(int type, const timeval &interval,
                    const timeval &value) noexcept;

            //! copying is not possible
            ITimer(const ITimer &other) = delete;
            //! moving is not possible
            ITimer(ITimer &&other) = delete;
            //! copying is not possible
            ITimer& operator=(const ITimer &other) = delete;
            //! moving is not possible
            ITimer& operator=(ITimer &&other) = delete;

        public:
            /*! \brief Destroy the timer instance
             *
             * Timer is stopped if running, however, the timer should be
             * stopped beforehand if possible.
             * If the timer is still running and the call of stop() fails, the
             * process will be terminated.
             */
            virtual ~ITimer( );

            /*! \brief start timer
             *
             * possible throws:
             *      std::runtime_error  timer is already started
             *      std::system_error   a system call failed
             */
            void start();

            /*! \brief stop timer
             *
             * possible throws:
             *      std::runtime_error  timer is already stopped
             *      std::system_error   a system call failed
             */
            void stop();

            /*! \brief set speed factor
             *
             * is applied directly, even if the timer is running
             *
             * possible throws:
             *      std::invalid_argument   speed_factor is out of range
             *      std::system_error       a system call failed
             */
            void set_speed_factor(const double speed_factor);

            /*! \brief set speed to normal
             *
             * is applied directly, even if the timer is running
             *
             * possible throws:
             *      std::invalid_argument   speed_factor is out of range
             *      std::system_error       a system call failed
             */
            void set_speed_to_normal();

            /*! \brief write to binary file stream
             *
             * write interval and value to file stream.
             * type and speed factor is not stored!
             *
             */
            void to_fstream(std::ofstream& fstream) const;

            /*! \brief read from binary filestream
             *
             * read interval and value to file stream.
             * Timer must be stopped
             *
             * possible throws:
             *      std::logic_error   timer is not stopped
             */
            void from_fstream(std::ifstream& fstream);

            /*! \brief get timer value (non const objects)
             *
             * stored timer value or actual timer value (if running)
             */
            timeval get_timer_value() const;

            // get current timer state
            inline bool is_running() const noexcept;

            /*! \brief get the version of the header file
             *
             * only interesting if used as library.
             *
             * possible throws:
             *      std::system_error       a system call failed
             */
            static inline unsigned long getHeaderVersion() noexcept;

            /*! \brief get the version of the source file
             *
             * only interesting if used as library.
             */
            static unsigned long getSourceVersion() noexcept;

            //! Set stream for error output for "non-throwable" errors
            inline static void set_error_stream(std::ostream& stream) noexcept;
    };

    /*! \brief class ITimer_Real
     *
     * "This  timer counts down in real (i.e., wall clock) time.
     * At each expiration, a SIGALRM signal is generated." (man getitimer)
     */
    class ITimer_Real : public ITimer
    {
        private:
            //! only one instance per process allowed
            static bool instance_exists;

        public:
            /*! \brief create real time interval timer
             *
             * attributes:
             *      interval: Interval at which the timer is triggered
             *
             * possible throws:
             *      std::logic_error    an instance already exists
             */
            ITimer_Real(const timeval &interval);

            /*! \brief create real time interval timer
             *
             * attributes:
             *      interval: Interval at which the timer is triggered
             *      value   : Time period after which the timer expires for the
             *                first time
             *
             * possible throws:
             *      std::logic_error    an instance already exists
             */
            ITimer_Real(const timeval &interval, const timeval &value);

            //! destroy instance (see ITimer::~ITimer())
            virtual ~ITimer_Real( );

            //! copying is not possible
            ITimer_Real(const ITimer_Real &other) = delete;
            //! moving is not possible
            ITimer_Real(ITimer_Real &&other) = delete;
            //! copying is not possible
            ITimer_Real& operator=(const ITimer_Real &other) = delete;
            //! moving is not possible
            ITimer_Real& operator=(ITimer_Real &&other) = delete;
    };

    /*! \brief class ITimer_Virtual
     *
     * "This timer counts down against the  user-mode  CPU  time
     * consumed  by the process.  (The measurement includes CPU
     * time consumed by all threads in the process.)   At  each
     * expiration, a SIGVTALRM signal is generated." (man getitimer)
     */
    class ITimer_Virtual : public ITimer
    {
        private:
            //! only one instance per process allowed
            static bool instance_exists;

        public:
            /*! \brief create user cpu time interval timer
             *
             * attributes:
             *      interval: Interval at which the timer is triggered
             *
             * possible throws:
             *      std::logic_error    an instance already exists
             */
            ITimer_Virtual(const timeval &interval);

            /*! \brief create user cpu time interval timer
             *
             * attributes:
             *      interval: Interval at which the timer is triggered
             *      value   : Time period after which the timer expires for the
             *                first time
             *
             * possible throws:
             *      std::logic_error    an instance already exists
             */
            ITimer_Virtual(const timeval &interval, const timeval &value);

            //! destroy instance (see ITimer::~ITimer())
            virtual ~ITimer_Virtual( );

            //! copying is not possible
            ITimer_Virtual(const ITimer_Virtual &other) = delete;
            //! moving is not possible
            ITimer_Virtual(ITimer_Virtual &&other) = delete;
            //! copying is not possible
            ITimer_Virtual& operator=(const ITimer_Virtual &other) = delete;
            //! moving is not possible
            ITimer_Virtual& operator=(ITimer_Virtual &&other) = delete;
    };

    /*! \brief class ITimer_Prof
     *
     * "This  timer  counts  down  against the total (i.e., both
     * user and system) CPU time consumed by the process.  (The
     * measurement includes CPU time consumed by all threads in
     * the process.)  At each expiration, a SIGPROF  signal  is
     * generated.

     * In  conjunction  with  ITIMER_VIRTUAL, this timer can be
     * used to profile user and system CPU time consumed by the
     * process." (man getitimer)
     */
    class ITimer_Prof : public ITimer
    {
        private:
            //! only one instance per process allowed
            static bool instance_exists;

        public:
            /*! \brief create cpu time interval timer
             *
             * attributes:
             *      interval: Interval at which the timer is triggered
             *
             * possible throws:
             *      std::logic_error    an instance already exists
             */
            ITimer_Prof(const timeval &interval);

            /*! \brief create cpu time interval timer
             *
             * attributes:
             *      interval: Interval at which the timer is triggered
             *      value   : Time period after which the timer expires for the
             *                first time
             *
             * possible throws:
             *      std::logic_error    an instance already exists
             */
            ITimer_Prof(const timeval &interval, const timeval &value);

            //! destroy instance (see ITimer::~ITimer())
            virtual ~ITimer_Prof( );

            //! copying is not possible
            ITimer_Prof(const ITimer_Prof &other) = delete;
            //! moving is not possible
            ITimer_Prof(ITimer_Prof &&other) = delete;
            //! copying is not possible
            ITimer_Prof& operator=(const ITimer_Prof &other) = delete;
            //! moving is not possible
            ITimer_Prof& operator=(ITimer_Prof &&other) = delete;
    };

    //! multiply timeval with double factor
    timeval& operator *= (timeval& left, double right) noexcept;

    //! multiply each timeval of itimerval with double factor
    itimerval& operator *= (itimerval& left, double right) noexcept;

    //! multiply timeval with double factor
    timeval operator * (const timeval& left, double right) noexcept;

    //! multiply each timeval of itimerval with double factor
    itimerval operator * (const itimerval& left, double right) noexcept;

    //! divide timeval by double factor
    timeval& operator /= (timeval& left, double right) noexcept;

    //! divide each timeval of itimerval by double factor
    itimerval& operator /= (itimerval& left, double right) noexcept;

    //! divide timeval by double factor
    timeval operator / (const timeval& left, double right) noexcept;

    //! divide each timeval of itimerval by double factor
    itimerval operator / (const itimerval& left, double right) noexcept;

    //! convert timeval to double (seconds)
    double timeval_to_double(const timeval& time) noexcept;

    //! convert double (seconds) to timeval
    timeval double_to_timeval(const double time) noexcept;

    inline unsigned long ITimer::getHeaderVersion() noexcept
    {
        return KOESLINGNI_ITIMER_VERSION;
    }

    inline bool ITimer::is_running() const noexcept
    {
    	return running;
    }

    inline void ITimer::set_error_stream(std::ostream& stream) noexcept
    {
        error_stream = &stream;
    }

} /* namespace ITimer */
} /* namespace Koesling */
} /* namespace de */
