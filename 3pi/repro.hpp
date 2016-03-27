#ifndef REPRO_HPP
#define REPRO_HPP

using pi::buzzer;

struct repro_t
{
	typedef uint16_t freq_type;
	typedef timeout::time_type time_type;
	typedef uint8_t count_type;
	
	static void init()
	{
		m_timer.stop();
		buzzer.set(0, 0, false);
	}
		
	static void beep(const bool& en = true)
	{
		if(en)
		{
			enable();
		}
		else
		{
			disable();
			m_timer.stop();
		}
	}
	static void beep(const freq_type& freq, const time_type& length = 0)
	{
		set_frequency(freq);
		m_stop_counter = 1;
		m_off_time = 0;
		m_timer.set_timeout(length);
		m_timer.restart();
		enable();
	}
	static void beep(const freq_type& freq, const time_type& period, const time_type& difference, const count_type& count = 0)
	{
		set_frequency(freq);
		m_on_time = (period>>1) + difference;
		m_off_time = (period>>1) - difference;
		m_stop_counter = count;
		m_timer.set_timeout(m_on_time);
		m_timer.restart();
		enable();
	}
	
	static bool beeping() { return buzzer.isStarted() || m_timer.running(); }
	
	static void process()
	{
		if(!m_timer.running())
			return;
		if(m_timer)
		{
			if(m_off_time == 0) // last beep ends
			{
				disable();
				m_timer.stop();
			}
			else
			{
				if(buzzer.isStarted()) // silent half-period starts
				{
					disable();
					m_timer.ack();
					m_timer.set_timeout(m_off_time);
					if(m_stop_counter != 0 && --m_stop_counter == 0) // stop after beep ends
						m_off_time = 0;
				}
				else // beep half-period start
				{
					enable();
					m_timer.ack();
					m_timer.set_timeout(m_on_time);
				}
			}
		}
	}
	
private:
	static void set_frequency(const freq_type& freq) {}
	
	static void enable() { buzzer.start(); }
	static void disable() { buzzer.stop(); }

	static uint8_t m_prescaller;
	static timeout m_timer;
	static time_type m_on_time;
	static time_type m_off_time;
	static count_type m_stop_counter;
	
};

uint8_t repro_t::m_prescaller = 5;
timeout repro_t::m_timer(0);
repro_t::time_type repro_t::m_on_time = 0;
repro_t::time_type repro_t::m_off_time = 0;
repro_t::count_type repro_t::m_stop_counter = 0;

typedef repro_t repro;

#endif