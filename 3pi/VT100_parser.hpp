#ifndef VT100_PARSER_HPP
#define VT100_PARSER_HPP

#ifndef VT100_ARG_BUFF
#define VT100_ARG_BUFF 8
#endif

template <typename Usart, typename Display, typename Repro>
class VT100_parser_t
{
public:
	typedef uint8_t int_t;
	typedef typename Repro::time_type time_type;
	
	static const uint8_t c_rec_num_buff_size = VT100_ARG_BUFF;

	VT100_parser_t(Usart& usart, Display& disp)
		:m_usart(usart), m_disp(disp), m_state(0), m_cursor_x(0), m_cursor_y(0),
		 m_rec_num_ptr(0)
	{}
		
	void clear(const bool& with_display = false)
	{
		m_state = 0;
		m_cursor_x = 0;
		m_cursor_y = 0;
		m_rec_num_ptr = 0;
		if(with_display)
			m_disp.clear();
	}
	
	Display& display() { return m_disp; }
		
	void write(const char& ch)
	{
// 		display_t::coor_type cursor_x = 0;
// 		display_t::coor_type cursor_y = 0;
// 		bool scrolling = false;
		switch(ch)
		{
			// abort current control sequence
			case 0x18: // CAN
			case 0x1A: // SUB
				m_state = 0;
				return;
				
			// abort current control sequence, but immediately starts new one
			case 0x1B: // ESC
				m_state = 0;
				break;
				
			// these control characters are processed immediately
			case '\a': // BEL
				Repro::beep(440, time_type(200));
				return;
						
			case '\b': // BS
			case 0x7F: // DEL
				m_disp.backspace();
				return;
						
			case '\t': // HT
				m_disp.x((1+(m_disp.x()>>2))<<2);
				return;
						
			case '\n': // CR
				if(m_disp.line_fed())
					return;
				m_disp.x(0);
			case '\v': // VT
				m_disp.move_down();
				return;
						
			case '\r': // CR
				m_disp.x(0);
				return;
						
			case '\f': // FF
				m_disp.clear();
				return;
				
			// not implemented
			/*case 0x00: // NUL
			case 0x05: // ENQ
			case 0x0E: // SO
			case 0x0F: // SI
			case 0x11: // XON
			case 0x13: // XOFF
				return;*/
				
			// ignored
			default:
				if(ch <= 0x1F)
					return;
				break;
		}
		switch(m_state)
		{
			case 0: // check ESC
				if(ch == 0x1B) // ESC
					m_state = 1;
				else
					m_disp.write(ch);
				break;
				
			case 1: // first char after ESC
				switch(ch)
				{
					case '[':
						m_state = 2;
						if(m_rec_num_ptr < c_rec_num_buff_size)
							++m_rec_num_ptr;
						else
							m_rec_num_ptr = c_rec_num_buff_size;
						for(uint8_t i = 0; i != m_rec_num_ptr; ++i)
							m_rec_num[i] = 0;
						m_rec_num_ptr = 0;
						break;
						
					case '7': // FIX-ME: save attributes
						m_cursor_x = m_disp.x();
						m_cursor_y = m_disp.y();
						break;
						
					case '8': // FIX-ME: load attributes
						m_disp.move_to(m_cursor_x, m_cursor_y);
						break;
						
					case 'D':
						m_disp.move_down();
						break;
						
					case 'E':
						m_disp.move_down();
						m_disp.x(0);
						break;
						
					case 'M':
						m_disp.move_up();
						break;
				}
				if(m_state == 1)
					m_state = 0;
				break;
				
			case 2: // '['
				switch(ch)
				{
					case '0' ... '9':
						m_rec_num[m_rec_num_ptr] *= 10;
						m_rec_num[m_rec_num_ptr] += ch - '0';
						return;
						
					case ';':
						if(m_rec_num_ptr < c_rec_num_buff_size - 1)
							++m_rec_num_ptr;
						return;
					
					case 'A': // CUU - cursor up
						m_disp.move_up(min_one(0));
						break;
						
					case 'B': // CUD - cursor down
						m_disp.move_down(min_one(0));
						break;
						
					case 'C': // CUF - cursor forward
						// cursor should stop on left margin, therefore move_right, instead of move_forward
						m_disp.move_right(min_one(0));
						break;
					
					case 'D': // CUB - cursor backward
						// cursor should stop on left margin, therefore move_left, instead of move_backward
						m_disp.move_left(min_one(0));
						break;
						
					case 'H': // CUP - cursor position
					case 'f': // HVP – Horizontal and Vertical Position
						m_disp.move_to(min_one(1)-1, min_one(0)-1);
						break;
						
					case 'q': // DECLL - load LEDs
						for(uint8_t i = 0; i <= m_rec_num_ptr; ++i)
						{
							if(m_rec_num[i] == 0)
							{
								for(uint8_t i = 0; i != 4; ++i)
									m_disp.set_led(i, false);
							}
							else if(m_rec_num[i] < 5)
							{
								m_disp.set_led(m_rec_num[i]-1, true);
							}
						}
						break;
						
					case 'n': // DSR - device status report
						switch(m_rec_num[0])
						{
							case 5: // DSR - device status report
								send_spgm(m_usart, PSTR("\x1b[0n"));
								break;
								
							case 6: // CPR – Cursor Position Report
								format_spgm(m_usart, PSTR("\x1b[% ;% R")) % (m_disp.y() + 1) % (m_disp.x() + 1);
								break;
						}
						break;
						
					/*case 'j': // ED – Erase In Display
						cursor_x = m_disp.x();
						cursor_y = m_disp.y();
						scrolling = m_disp.enable_scrolling(false);
						switch(m_rec_num[0])
						{
							case 0: // from the active position to the end of the screen, inclusive
								do {
									m_disp.write(' ');
								} while(m_disp.x() != m_disp.width() - 1 && m_disp.y() != m_disp.height() - 1);
								break;
								
							case 1: // from the start of the screen to the active position, inclusive
								m_disp.move_to(0, 0);
								do {
									m_disp.write(' ');
								} while(m_disp.x() != cursor_x && m_disp.y() != cursor_y);
								break;
								
							case 2: // entire display
								m_disp.clear();
								break;
						}
						m_disp.enable_scrolling(scrolling);
						m_disp.move_to(cursor_x, cursor_y);
						break;
						
					case 'k': // EL – Erase In Line
						cursor_x = m_disp.x();
						cursor_y = m_disp.y();
						scrolling = m_disp.enable_scrolling(false);
						switch(m_rec_num[0])
						{
							case 0: // from the active position to the end of the line, inclusive
								do {
									m_disp.write(' ');
								} while(m_disp.x() != m_disp.width() - 1);
								break;
								
							case 1: // from the start of the line to the active position, inclusive
								m_disp.x(0);
								do {
									m_disp.write(' ');
								} while(m_disp.x() != cursor_x);
								break;
								
							case 2: // entire line
								m_disp.x(0);
								do {
									m_disp.write(' ');
								} while(m_disp.x() != m_disp.width() - 1);
								break;
						}
						m_disp.enable_scrolling(scrolling);
						m_disp.move_to(cursor_x, cursor_y);
						break;*/
						break;
				}
				if(m_state == 2)
					m_state = 0;
				break;
		}
	}
		
private:
	int_t min_one(const uint8_t& num_index)
	{
		return m_rec_num[num_index] == 0 ? 1 : m_rec_num[num_index];
	}

	Usart& m_usart;
	Display& m_disp;
	
	uint8_t m_state;
	typename Display::coor_type m_cursor_x;
	typename Display::coor_type m_cursor_y;
	uint8_t m_rec_num_ptr;
	int_t m_rec_num[c_rec_num_buff_size];
};

#endif
