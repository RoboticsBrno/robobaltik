#ifndef CMD_PARSER_HPP
#define CMD_PARSER_HPP
#include <string.h>

#define SENSORS_CALIBRATION_DEFAULT_EEPROM_ADDRESS 1010
#define PARSING_ESCAPE_SEQUENCES_EEPROM_ADDRESS 1009

class cmd_parser_t
{
public:
	cmd_parser_t()
		:m_esc(false), m_cmd(), m_rptr(0), m_parse_escape_sequence(pi::load_eeprom<uint8_t>(PARSING_ESCAPE_SEQUENCES_EEPROM_ADDRESS))
	{}
	
	void push(const char& ch)
	{
		switch(ch)
		{
			case '\n':
				//uart.write('\n');
				//disp.write('\n');
				parse();
				clear();
				break;
			case '\b':
			case 0x7f:
				if(!m_cmd.empty())
				{
					m_cmd.erase(m_cmd.end() - 1);
					//uart.write('\b');
					//disp.write('\b');
				}
				else
				{
					//uart.write('\a');
					disp.write('\a');
				}
				break;
			default:
				if(m_cmd.size() < m_cmd.max_size())
				{
					m_cmd += ch;
					//uart.write(ch);
					//disp.write(ch);
				}
				else
				{
					//uart.write('\a');
					disp.write('\a');
				}
				break;
		}
	}
private:
	void clear()
	{
		m_cmd.clear();
		m_rptr = 0;
	}
	
	void parse()
	{
		string cmd = pop_arg();
		if(cmd.compare_spgm(PSTR("disp")) == 0)
		{
			string str = m_cmd.substr(m_rptr);
			string::size_t length = str.length();
			bool end = false;
			bool hex_ch = false;
			
			if (m_parse_escape_sequence)
			{
				bool back_slash = false;
				for (string::size_t i = 0; i < length; ++i)
				{
					if (str[i] == '\\' && !back_slash)
					{
						if (length <= (i + 1))
						{
							format_spgm(uart, PSTR("behind \"% \" is not char\n")) % str[i];
							end = true;
							break;
						}
						back_slash = true;
						continue;
					}
					else if (back_slash)
					{
						switch(str[i])
						{
							case '\\': str[i] = '\\'; break;
							case 'n': str[i] = '\n'; break;
							case 'b': str[i] = '\b'; break;
							case 'a': str[i] = '\a'; break;
							case 'r': str[i] = '\r'; break;
							case 'v': str[i] = '\v'; break;
							case 'f': str[i] = '\f'; break;
							case 't': str[i] = '\t'; break;
							case 'x':
								if (length <= (i + 2))
								{
									format_spgm(uart, PSTR("behind \"% \" is not char for make hex\n")) % str[i];
									end = true;
									break;
								}
								str[i] = 0;
								hex_ch = true;
								for (string::size_t j = (i + 1); j <= (i + 2); ++j)
								{
									switch(str[j])
									{
										case '1' ... '9': str[i] = (str[i]<<4) | (str[j] - '0'); break; //bitovy posun o 4 a logicky soucet
										case 'A' ... 'F': str[i] = (str[i]<<4) | ((str[j] -'A') + 10); break;
										default: end = true; format_spgm(uart, PSTR("\"% \" is not hex\n")) % str[j];						
									}
								}
								break; 
							default: end = true; format_spgm(uart, PSTR("\"% \" is not valid escape sequence\n")) % str[i];
						}
						back_slash = false;
					}
					if(!end)
					{
						disp.write(str[i]);
					}
					if (hex_ch)
					{
						hex_ch = false;
						i += 2;
					}
				}
			}
			else
			{
				send(disp, m_cmd.substr(m_rptr));
			}
		}
		else if(cmd.compare_spgm(PSTR("cursor")) == 0)
		{
			display.cursor(get_num_opt<uint8_t>(1));
		}
		else if(cmd.compare_spgm(PSTR("w")) == 0)
		{
			grid_move.go_forward(get_num_opt<grid_move_t::count_t>(1));
		}
		else if(cmd.compare_spgm(PSTR("s")) == 0)
		{
			grid_move.go_backward(get_num_opt<grid_move_t::count_t>(1));
		}
		else if(cmd.compare_spgm(PSTR("a")) == 0)
		{
			grid_move.turn_left(get_num_opt<grid_move_t::count_t>(1));
		}
		else if(cmd.compare_spgm(PSTR("d")) == 0)
		{
			grid_move.turn_right(get_num_opt<grid_move_t::count_t>(1));
		}
		else if(cmd.compare_spgm(PSTR("b")) == 0)
		{
			auto n = get_num_opt<repro::count_type>(1);
			auto length = get_num_opt<repro::time_type>(200);
			auto diff = get_num_opt<repro::time_type>(length / 4);
			//format(uart, "repro: n = % , l = % , d = % \n") % n % length % diff;
			repro::beep(440, length, diff, n);
		}
		else if(cmd.compare_spgm(PSTR("speed")) == 0)
		{
			grid_move_t::speed_t s = 0;
			if(get_num(s))
				grid_move.set_speed(s);
		}
		else if(cmd.compare_spgm(PSTR("power")) == 0)
		{
			int16_t l, r;
			if(get_num(l) && get_num(r))
			{
				stopwatch::time_type t = get_num_opt<stopwatch::time_type>(0);
				setMotorPower(l, r);
				if(t)
				{
					wait(t);
					setMotorPower(0, 0);
				}
			}
		}
		else if(cmd.compare_spgm(PSTR("led")) == 0)
		{
			if(m_rptr != m_cmd.length())
			{
				switch(m_cmd[m_rptr])
				{
				case 's':
					led.set();
					break;
				
				case 'c':
					led.clear();
					break;
					
				case 't':
					led.toggle();
					break;
					
				default:
					{
						auto n = get_num_opt<uint8_t>(1);
						auto length = get_num_opt<stopwatch::time_type>(500);
						auto diff = get_num_opt<stopwatch::time_type>(length / 4);
						led.blink(length, diff, n);
					}
					break;
				}
			}
		}
		else if(cmd.compare_spgm(PSTR("buttons")) == 0)
		{
			if(pi::isPressed(BUTTON_A))
				uart.write('A');
			if(pi::isPressed(BUTTON_B))
				uart.write('B');
			if(pi::isPressed(BUTTON_C))
				uart.write('C');
			uart.write('\n');
		}
		else if(cmd.compare_spgm(PSTR("sensors")) == 0)
		{
			bool raw = get_num_opt<bool>(false);
			for(uint8_t i = 0; i != PI_TOTAL_SENSORS; ++i)
			{
				avrlib::send_int(uart, pi::getSensorValue(i, !raw));
				if(i != (PI_TOTAL_SENSORS - 1))
					uart.write(' ');
			}
			uart.write('\n');
		}
		else if(cmd.compare_spgm(PSTR("line_position")) == 0)
		{
			format_spgm(uart, PSTR("% \n")) % pi::getLinePos();
		}
		else if(cmd.compare_spgm(PSTR("reset_calibration")) == 0)
		{
			pi::resetCalibration();
		}
		else if(cmd.compare_spgm(PSTR("calibration_sensors")) == 0)
		{
			pi::calibrate_sensors();
		}
		else if(cmd.compare_spgm(PSTR("cal_round")) == 0)
		{
			pi::cal_round();
		}
		else if(cmd.compare_spgm(PSTR("store_sensor_cal")) == 0)
		{
			pi::store_sensor_cal(get_num_opt<uint16_t>(SENSORS_CALIBRATION_DEFAULT_EEPROM_ADDRESS));
		}
		else if(cmd.compare_spgm(PSTR("load_sensor_cal")) == 0)
		{
			pi::load_sensor_cal(get_num_opt<uint16_t>(SENSORS_CALIBRATION_DEFAULT_EEPROM_ADDRESS));
		}
		else if(cmd.compare_spgm(PSTR("parse_escape_seq")) == 0)
		{
			if(m_rptr != m_cmd.length()){
				switch(m_cmd[m_rptr]){
					case '1':
						m_parse_escape_sequence = true;
						break;
					
					case '0':
						m_parse_escape_sequence = false;
						break;
					default:
						//if(!m_cmd.substr(m_rptr).empty())
						format_spgm(uart, PSTR("wrong arg \"% \"")) % m_cmd.substr(m_rptr);
						format_spgm(uart, PSTR(" for command \"% \"")) % cmd;
						uart.write('\n');
				}
				pi::store_eeprom(PARSING_ESCAPE_SEQUENCES_EEPROM_ADDRESS, m_parse_escape_sequence);
			}
		}
		else if(cmd.compare_spgm(PSTR("store_eeprom")) == 0)
		{
			uint16_t addr = 0;
			if(get_num(addr))
			{
				for(; m_rptr != m_cmd.length(); ++addr)
				{
					pi::store_eeprom(addr, get_num_opt<uint8_t>(0));
				}
			}
		}
		else if(cmd.compare_spgm(PSTR("load_eeprom")) == 0)
		{
			uint16_t addr = 0;
			if(get_num(addr))
			{
				const uint16_t len = get_num_opt<uint16_t>(1);
				for(uint16_t i = 0; i != len; ++i)
				{
					avrlib::send_int(uart, pi::load_eeprom<uint8_t>(addr));
					++addr;
					if(i != (len - 1))
						uart.write(' ');
				}
				uart.write('\n');
			}
		}
		else if(cmd.compare_spgm(PSTR("show_info")) == 0)
		{
			show_info(uart);
		}
		else if(cmd.compare_spgm(PSTR("reset")) == 0)
		{
			sw_rst();
		}
		else if(cmd.compare_spgm(PSTR("time_horizontal")) == 0)
		{
			uint32_t t = get_num_opt(0);
			if(t)
				grid_move.set_horizonatal_time(t);
			else
				format_spgm(uart, PSTR("% \n")) % grid_move.get_horizonatal_time();
		}
		else if(cmd.compare_spgm(PSTR("time_vertical")) == 0)
		{
			uint32_t t = get_num_opt(0);
			if(t)
				grid_move.set_vertical_time(t);
			else
				format_spgm(uart, PSTR("% \n")) % grid_move.get_vertical_time();
		}
		else if(cmd.compare_spgm(PSTR("time_turn")) == 0)
		{
			uint32_t t = get_num_opt(0);
			if(t)
				grid_move.set_turn_time(t);
			else
				format_spgm(uart, PSTR("% \n")) % grid_move.get_turn_time();
		}

		else
		{
			format_spgm(uart, PSTR("Unknown command \"% \"")) % cmd;
			if(!m_cmd.substr(m_rptr).empty())
				format_spgm(uart, PSTR(" with args\"% \"")) % m_cmd.substr(m_rptr);
			uart.write('\n');
		}
	}
	
	string pop_arg()
	{
		string::size_t stop = m_cmd.find_first_of(' ', m_rptr);
		string::size_t start = m_rptr;
		m_rptr = stop == string::npos ? m_cmd.length() : (stop + 1);
		return m_cmd.substr(start, stop - start);
	}
	
	string top_arg()
	{
		string::size_t stop = m_cmd.find_first_of(' ', m_rptr);
		string::size_t start = m_rptr;
		return m_cmd.substr(start, stop - start);
	}
	
	template <class Integer>
	bool get_num(Integer& n)
	{
		string s = pop_arg();
		if(!avrlib::string2number(s, n))
		{
			format_spgm(uart, PSTR("Not a number: input[% :% ] = \"% \"\n")) % (m_rptr - s.length()) % m_rptr % s;
			return false;
		}
		return true;
	}
	
	template <class Integer>
	Integer get_num_opt(const Integer& Default)
	{
		Integer n = 0;
		if(avrlib::string2number(pop_arg(), n))
			return n;
		return Default;
	}

	bool m_esc;
	string m_cmd;
	string::size_t m_rptr;
	bool m_parse_escape_sequence;
};

#endif