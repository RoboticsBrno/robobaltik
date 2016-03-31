#ifndef CMD_PARSER_HPP
#define CMD_PARSER_HPP

class cmd_parser_t
{
public:
	cmd_parser_t()
		:m_esc(false), m_cmd(), m_rptr(0)
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
			send(disp, m_cmd.substr(m_rptr));
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
		else if(cmd.compare_spgm(PSTR("resetCalibration")) == 0)
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
			pi::store_sensor_cal(get_num_opt<uint16_t>(0));
		}
		else if(cmd.compare_spgm(PSTR("load_sensor_cal")) == 0)
		{
			pi::load_sensor_cal(get_num_opt<uint16_t>(0));
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
// 		else if(cmd.compare_spgm(PSTR("time")) == 0)
// 		{
// 			grid_move_t::speed_t s = 0;
// 			if(get_num(s))
// 				grid_move.set_time(s);
// 		}
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
};

#endif