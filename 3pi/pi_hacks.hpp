#ifndef PI_HACKS_HPP
#define PI_HACKS_HPP

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

#include "avrlib/portb.hpp"
#include "avrlib/portc.hpp"
#include "avrlib/portd.hpp"
#include "avrlib/pin.hpp"

#include "avrlib/make_byte.hpp"

#include "avrlib/math.hpp"

#include "avrlib/stopwatch.hpp"

namespace time_namespace {
#include "time.hpp"
}

void delayMicroseconds(uint16_t microseconds);
void delay(uint16_t ms);

#define usec(x) x
#define msec(x) (1000UL*x)
#define wait(x) if(x > 65535) delay(x / 1000); else delayMicroseconds(x);

using time_namespace::timeout;

#include "display.hpp"

#include "virtual_port.hpp"

typedef display_t <
	avrlib::pin<avrlib::portd, 4>,
	avrlib::pin<avrlib::portb, 0>,
	avrlib::pin<avrlib::portd, 2>,
	avrlib::dummy_pin<false>,
	virtual_half_port <
		avrlib::pin<avrlib::portb, 1>,
		avrlib::pin<avrlib::portb, 4>,
		avrlib::pin<avrlib::portb, 5>,
		avrlib::pin<avrlib::portd, 7> >,
	0, 8, 2> display_type;
display_type display;
void init_display() { display.init(); }
void clean_display() { display.clear(); }

#undef nop
#undef usec
#undef msec
#undef wait

using namespace time_namespace;
#define msec(value) stopwatch::time_type(value)

#define PI_LIB_DISPLAY

#define JUNIOR_RS232_BPS 115200

void run(void);

namespace pi {

#include "3piLibPack.h"

void run(void)
{
	::run();
}

} // namespace pi

using pi::setMotorPower;
using pi::rs232;

void delayMicroseconds(uint16_t microseconds)
{
	pi::delayMicroseconds(microseconds);
}

void delay(uint16_t ms)
{
	pi::delay(ms);
}

base_timer_t::time_type base_timer_t::value() const { return pi::getTicksCount(); }
base_timer_t::time_type base_timer_t::operator()() const { return pi::getTicksCount(); }

#include "avrlib/string.hpp"
#include "avrlib/format.hpp"
#include "avrlib/iostream.hpp"
#include "avrlib/sstream.hpp"

using avrlib::string;
using avrlib::send;
using avrlib::send_spgm;
using avrlib::format;
using avrlib::format_spgm;

#include "repro.hpp"

struct uart_t
{
	char read() { return rs232.get(); }
	void write(const char& c) { rs232.sendCharacter(c); }
	void flush() {}
};
uart_t uart;

int main()
{
	pi::main();
}

#endif