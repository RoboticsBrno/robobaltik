#ifndef LED_HPP
#define LED_HPP

template<class PIN>
class led_t
{
public:
	typedef timeout::time_type time_type;
	typedef uint8_t count_type;

	led_t()
		:m_timer(0), m_on_time(0), m_off_time(0), m_stop_counter(0), m_state(0)
	{
		PIN::make_low();
		m_timer.cancel();
	}
	void set() { m_state = 1; }
	void clear() { m_state = 2; }
	void toggle() { m_state = 3; }
	void set(const uint8_t& value)
	{
		if(value)
			set();
		else
			clear();
	}
	void blink(const time_type& period, const time_type& difference = 0, const count_type& count = 0)
	{
		m_on_time = (period>>1) + difference;
		m_off_time = (period>>1) - difference;
		m_stop_counter = count;
		m_timer.set_timeout(m_on_time);
		m_timer.restart();
		set();
	}
	void on()
	{
		m_timer.cancel();
		set();
	}
	void off()
	{
		m_timer.cancel();
		clear();
	}
	void operator() (const uint8_t& value) { if (value) on(); else off(); }
	uint8_t get() const { return PIN::read(); }
	uint8_t blinking() const { return m_timer.running(); }
	void process()
	{
		switch(m_state)
		{
			case 0: break;
			case 1: _set(); m_state = 0; break;
			case 2: _clear(); m_state = 0; break;
			case 3: _toggle(); m_state = 0; break;
		}
		if(m_timer.running() && m_timer)
		{
			if(m_off_time == 0) // last blink ends
			{
				_clear();
				m_timer.stop();
			}
			else
			{
				if(get()) // dark half-period starts
				{
					_clear();
					m_timer.ack();
					m_timer.set_timeout(m_off_time);
					if(m_stop_counter != 0 && --m_stop_counter == 0) // stop after next tiemout
						m_off_time = 0;
				}
				else // light half-period start
				{
					_set();
					m_timer.ack();
					m_timer.set_timeout(m_on_time);
				}
			}
		}
	}
private:
	void _set() { PIN::set_high(); }
	void _clear() { PIN::set_low(); }
	void _toggle() { PIN::toggle(); }
	
	timeout m_timer;
	time_type m_on_time;
	time_type m_off_time;
	count_type m_stop_counter;
	uint8_t m_state;
};

#endif
