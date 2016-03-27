#ifndef VIRTUAL_PORT_HPP
#define VIRTUAL_PORT_HPP

template <
typename PIN_0,
typename PIN_1,
typename PIN_2,
typename PIN_3 >
struct virtual_half_port
{
	
	static uint8_t port()
	{
		return avrlib::make_byte(
		PIN_0::get(),
		PIN_1::get(),
		PIN_2::get(),
		PIN_3::get());
	}
	static void port(uint8_t v)
	{
		PIN_0::set_value(v & (1<<0));
		PIN_1::set_value(v & (1<<1));
		PIN_2::set_value(v & (1<<2));
		PIN_3::set_value(v & (1<<3));
	}

	static uint8_t pin()
	{
		return avrlib::make_byte(
		PIN_0::read(),
		PIN_1::read(),
		PIN_2::read(),
		PIN_3::read());
	}

	static uint8_t dir()
	{
		return avrlib::make_byte(
		PIN_0::output(),
		PIN_1::output(),
		PIN_2::output(),
		PIN_3::output());
	}
	static void dir(uint8_t v)
	{
		PIN_0::output(v & (1<<0));
		PIN_1::output(v & (1<<1));
		PIN_2::output(v & (1<<2));
		PIN_3::output(v & (1<<3));
	}
};

#endif