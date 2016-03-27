/*
 * 3pi.cpp
 *
 * Created: 17.3.2016 13:43:12
 * Author : kubas
 */

#define AVRLIB_DEFAULT_STRING_SIZE 64

#include "pi_hacks.hpp"

#include "version_info.hpp"

#include "led.hpp"
led_t< avrlib::pin<avrlib::portd, 7> > led;

#include "VT100_parser.hpp"
VT100_parser_t<uart_t, display_type, repro> disp(uart, display);

#include "grid_move.hpp"
grid_move_t grid_move;

const char device_name[] PROGMEM = "3pi remote control";
//const char hw_version[] PROGMEM = "v1.0";
const char vendor[] PROGMEM = "kubas@Robotarna";

void sw_rst()
{
	pi::setMotorPower(0, 0);
	grid_move.clear();
	repro::beep(false);
	led.off();
	disp.clear(true);
}

template <class Stream>
void show_info(Stream& stream)
{
	send_spgm(stream, device_name);
	stream.write(' ');
	send_spgm(stream, build_info);
	stream.write('\n');
}

#include "cmd_parser.hpp"
cmd_parser_t cmd_parser;

void run(void)
{
	DDRD |= (1<<7);
	repro::init();
	send_spgm(uart, PSTR("\n\n"));
	show_info(uart);
	send_spgm(disp, PSTR("\f\aAhoj\n"));
	display.cursor(0);
	char ch = 0;
	for(;;)
	{
		if(rs232.peek(ch))
		{
			cmd_parser.push(ch);
		}
		led.process();
		repro::process();
	}
}
