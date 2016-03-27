#ifndef EEPROM_HPP
#define EEPROM_HPP

// EEPROM addresses
//#define GPIB_ADDR_EEPROM_ADDRESS 0 //1B

#define STB_MASK_EEPROM_ADDRESS 1 //1B
#define ESR_MASK_EEPROM_ADDRESS 2 //1B

#define GLOBAL_FORMAT_EEPROM_ADDRESS 3 //1B

#define INPUT_READER_FORMAT_EEPROM_ADDRESS 4 //1B
#define INPUT_READER_INVERT_EEPROM_ADDRESS 5 //2B

#define OUTPUT_WRITER_FORMAT_EEPROM_ADDRESS 7 //1B
#define OUTPUT_WRITER_INVERT_EEPROM_ADDRESS 8 //2B

struct eeprom
{
	typedef uint16_t addr_t;
	
	static void write_byte(const addr_t& addr, const uint8_t& byte) { pi::store_eeprom(addr, byte); }
	
	static uint8_t read_byte(const addr_t& addr) { return avrlib::load_eeprom<uint8_t>(addr); }
	
	static void write_block(addr_t addr, addr_t length, uint8_t const * data)
	{
		for(length += addr; addr != length; ++addr)
			write_byte(addr, *(data++));
	}

	static void read_block(addr_t addr, uint16_t length, uint8_t *data)
	{
		for(length += addr; addr != length; ++addr)
			*(data++) = read_byte(addr);
	}

	template <typename T>
	static void write(const addr_t& addr, const T& data) { write_block(addr, sizeof data, (uint8_t const *)&data); }

	template <typename T>
	static T read(const addr_t& addr)
	{
		T res;
		read_block(addr, sizeof res, (uint8_t *)&res);
		return res;
	}
	
	static void flush(const uint16_t&) {}
};

#endif
