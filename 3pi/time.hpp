#ifndef TIME_HPP
#define TIME_HPP

struct base_timer_t
{
	typedef uint32_t time_type;
	time_type value() const;
	time_type operator()() const;
};
base_timer_t base_timer;

//typedef avrlib::counter<avrlib::timer1, uint32_t, uint32_t, false> base_timer_t;
//base_timer_t base_timer(avrlib::timer_no_clock);

struct stopwatch
	:avrlib::stopwatch<base_timer_t>
{
	stopwatch(bool run = true)
		:avrlib::stopwatch<base_timer_t>(base_timer)
	{
		if(run)
			return;
		stop();
		clear();
	}
};

struct timeout
	:avrlib::timeout<base_timer_t>
{
	timeout(avrlib::timeout<base_timer_t>::time_type timeout)
		:avrlib::timeout<base_timer_t>(base_timer, timeout)
	{
	}
};

void wait(base_timer_t::time_type time)
{
	avrlib::wait(base_timer, time);
}

template <typename Process>
void wait(base_timer_t::time_type time, Process process)
{
	avrlib::wait(base_timer, time, process);
}

template <typename Process>
void wait(base_timer_t::time_type time, Process process, int)
{
	avrlib::wait(base_timer, time, process, 0);
}

//#define  sec(value) stopwatch::time_type(1000UL*value)
//#define msec(value) stopwatch::time_type(value)
//#define usec(value) stopwatch::time_type(20UL*value)

#endif