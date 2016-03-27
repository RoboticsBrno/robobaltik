#ifndef DISPLAY_HPP
#define DISPLAY_HPP

#ifndef nop
#define nop() __asm__ volatile ("nop;")
#endif

template <typename E, typename RW, typename RS, typename BL,
	typename PORT, uint8_t PIN_OFFSET, int WIDTH, int HEIGHT>
class display_t
{
public:
	typedef uint8_t coor_type;
	
	display_t()
		:m_x(0), m_y(0), m_enable_scrolling(true), m_line_fed(false)
	{}
		
	bool init()
	{
		E::make_low();
		RW::make_low();
		RS::make_low();
		PORT::dir(PORT::dir() | (0x0F<<PIN_OFFSET));
		wait(msec(16));
		_write_nibble(0x03);
		wait(msec(5));
		_write_nibble(0x03);
		wait(usec(200));
		_write_nibble(0x03);
		static const uint8_t init_values[] PROGMEM = { 0x32, 0x28, 0x08, 0x01, 0x02 };
		for(uint8_t i = 0; i != sizeof init_values; ++i)
		{
			uint8_t byte = pgm_read_byte(init_values + i);
			_write_nibble(byte>>4);
			wait(usec(40));
			_write_nibble(byte);
			wait(usec(40));
		}
		if(!_wait_for_ready())
			return false;
		_write_instr(0x14);
		_write_instr(0x0E);
		_write_instr(0x06);
		clear();
		return true;
	}

	void clear()
	{
		_write_instr(0x01);
		wait(usec(1500));
		m_x = 0;
		m_y = 0;
		_set_cursor_pos();
	}
	
	void cursor(const uint8_t& state) // 0 = off, 1 = on, 3 = blinking
	{
		_write_instr(0x0C | (state & 0x03));
	}
	
	void write(const char& c)
	{
		_write_data(c);
		_inc_cursor_pos();
	}
	
	void move_to(const coor_type& x, const coor_type& y)
	{
		m_x = avrlib::clamp(x, 0, c_width - 1);
		m_y = avrlib::clamp(y, 0, c_height - 1);
		_set_cursor_pos();
	}
	
	void move_right(const coor_type& n = 1)
	{
		m_x += avrlib::clamp(n, 0, c_width - m_x - 1);
		_set_cursor_pos();
	}
	
	void move_left(const coor_type& n = 1)
	{
		m_x -= avrlib::clamp(n, 0, m_x);
		_set_cursor_pos();
	}
	
	void move_down(const coor_type& n = 1)
	{
		coor_type dy = avrlib::clamp(n, 0, this->c_height - m_y - 1);
		if(dy == 0 && n != 0)
			_scroll_disp();
		else
			m_y += dy;
		_set_cursor_pos();
	}
	
	void move_up(const coor_type& n = 1)
	{
		m_y -= avrlib::clamp(n, 0, m_y);
		_set_cursor_pos();
	}
	
	void move_forward(coor_type n = 1)
	{
		n = avrlib::clamp(n, 0, (c_height - m_y) * c_width - m_x - 1);
		for(coor_type i = 0; i != n; ++i)
		{
			if(++m_x == c_width)
			{
				++m_y;
				m_x = 0;
			}
		}
		_set_cursor_pos();
	}
	
	void move_backward(coor_type n = 1)
	{
		n = avrlib::clamp(n, 0, m_y * c_width + m_x);
		for(coor_type i = 0; i != n; ++i)
		{
			if(m_x == 0)
			{
				--m_y;
				m_x = c_width;
			}
			--m_x;
		}
		_set_cursor_pos();
	}
	
	coor_type x() const { return m_x; }
		
	coor_type y() const { return m_y; }
		
	void x(const coor_type& X)
	{
		m_x = avrlib::clamp(X, 0, c_width - 1);
		_set_cursor_pos();
	}
	
	void y(const coor_type& Y)
	{
		m_y = avrlib::clamp(Y, 0, c_width - 1);
		_set_cursor_pos();
	}
	
	void backspace()
	{
		move_backward();
		write(' ');
		move_backward();
	}
	
	coor_type width() const { return c_width; }
		
	coor_type height() const { return c_height; }
		
	bool enable_scrolling(const bool& en)
	{
		bool old = m_enable_scrolling;
		m_enable_scrolling = en;
		return old;
	}
		
	bool is_scrolling_enabled() const { return m_enable_scrolling; }
	
	bool line_fed()
	{
		bool tmp = m_line_fed;
		m_line_fed = false;
		return tmp;
	}
	
	void set_led(const uint8_t& index, const bool& value)
	{
		switch(index)
		{
			case 0:
				BL::set_value(value);
				break;
		}
	}
	
private:
	void _set_cursor_pos()
	{
		static const coor_type line_tab[c_height] = { 0, 64 };//, c_width, c_width + 64 };
		_write_instr(0x80 | (m_x + line_tab[m_y]));
	}

	void _inc_cursor_pos()
	{
		if(++m_x == c_width)
		{
			m_x = 0;
			m_line_fed = true;
			if(++m_y == c_height)
			{
				m_y = c_height - 1;
				_scroll_disp();
			}
			_set_cursor_pos();
		}
		else
			m_line_fed = false;
	}
	
	void _scroll_disp()
	{
		if(!m_enable_scrolling)
			return;
		coor_type x = m_x;
		coor_type y = m_y;
		for(m_y = 1; m_y != c_height; m_y += 2)
		{
			m_x = 0;
			_set_cursor_pos();
			wait(usec(39));
			for(; m_x != c_width; ++m_x)
			{
				m_buff[m_x] = _read_byte(false);
				wait(usec(37));
			}
			m_x = 0;
			--m_y;
			_set_cursor_pos();
			for(; m_x != c_width; ++m_x)
				_write_data(m_buff[m_x]);
		}
		m_x = 0;
		m_y = c_height - 1;
		_set_cursor_pos();
		for(; m_x != c_width; ++m_x)
			_write_data(' ');
		m_x = x;
		m_y = y;
		_set_cursor_pos();
	}

	bool _wait_for_ready()
	{
		timeout t(msec(10));
		while(!t)
		{
			if((_read_byte() & 0x80) == 0)
			return true;
		}
		return false;
	}
	
	void _write_nibble(uint8_t nibble)
	{
		PORT::port((PORT::port() & ~(0x0F<<PIN_OFFSET)) | ((nibble & 0x0F)<<PIN_OFFSET));
		E::set_high();
		nop(); nop();
		E::set_low();
		nop(); nop();
	}
	
	void _write_data(const uint8_t& byte)
	{
		RW::set_low();
		_write_nibble(byte>>4);
		_write_nibble(byte);
		wait(usec(43));
	}
	
	bool _write_instr(const uint8_t& byte)
	{
		if(!_wait_for_ready())
			return false;
		RS::set_low();
		_write_data(byte);
		RS::set_high();
		return true;
	}
	
	uint8_t _read_byte(const bool& instruction = true)
	{
		PORT::port(PORT::port() | (0x0F<<PIN_OFFSET));
		PORT::dir(PORT::dir() & ~(0x0F<<PIN_OFFSET));
		RW::set_high();
		RS::set_value(!instruction);
		E::set_high();
		nop(); nop(); nop(); nop();
		uint8_t value = (PORT::pin()>>PIN_OFFSET)&0x0F;
		E::set_low();
		value <<= 4;
		nop(); nop(); nop(); nop();
		E::set_high();
		nop(); nop(); nop(); nop();
		value |= (PORT::pin()>>PIN_OFFSET)&0x0F;
		E::set_low();
		nop(); nop();
		PORT::dir(PORT::dir() | (0x0F<<PIN_OFFSET));
		RS::set_high();
		return value;
	}
	
	static const coor_type c_width = WIDTH;
	static const coor_type c_height = HEIGHT;
	
	coor_type m_x;
	coor_type m_y;
	
	bool m_enable_scrolling;
	
	bool m_line_fed;
	
	char m_buff[c_width];
};

#endif