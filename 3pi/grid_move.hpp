#ifndef GRID_MOVE_HPP
#define GRID_MOVE_HPP

class grid_move_t
{
public:
	typedef uint8_t coord_t;
	typedef uint8_t dir_t;
	typedef uint8_t speed_t;
	typedef uint8_t count_t;
	grid_move_t(const coord_t& x = 0, const coord_t& y = c_grid_height - 1, const coord_t& dir = 1, const speed_t& speed = 4)
		:m_x(clamp_x(x)), m_y(clamp_y(y)), m_dir(avrlib::clamp(dir, 1, 4)), m_speed(avrlib::clamp(speed, 0, 9)),
		 m_motor_speed(0), m_horizontal_time(0), m_vertical_time(0), m_turn_time(0)
	{
		set_motor_speed(m_speed);
	}
	
	void clear()
	{
		m_x = 0;
		m_y = c_grid_height - 1;
		m_dir = 1;
		m_speed = 4;
		set_motor_speed(m_speed);
	}
	
	void set_speed(const speed_t& speed)
	{
		m_speed = avrlib::clamp(speed, 0, 9);
		set_motor_speed(m_speed);
	}
	
	speed_t get_speed(void) { return m_speed; }
	
	void go_forward(count_t n = 1)
	{
		uint32_t& t = (m_dir & 1) ? m_horizontal_time : m_vertical_time;
		setMotorPower(m_motor_speed, m_motor_speed);
		wait(t * n);
		setMotorPower(0, 0);
	}
	
	void go_backward(count_t n = 1)
	{
		uint32_t& t = (m_dir & 1) ? m_horizontal_time : m_vertical_time;
		setMotorPower(-m_motor_speed, -m_motor_speed);
		wait(t * n);
		setMotorPower(0, 0);
	}
	
	void turn_left(const count_t& n = 1)
	{
		const int16_t motor_speed = 25;
		setMotorPower(-motor_speed, motor_speed);
		wait(m_turn_time * n);
		setMotorPower(0, 0);
	}
	
	void turn_right(const count_t& n = 1)
	{
		const int16_t motor_speed = 25;
		setMotorPower(motor_speed, -motor_speed);
		wait(m_turn_time * n);
		setMotorPower(0, 0);
	}

	uint32_t get_horizonatal_time() const { return m_horizontal_time; }
	uint32_t get_vertical_time() const { return m_turn_time; }
	uint32_t get_turn_time() const { return m_turn_time; }
		
	void set_horizonatal_time(const uint32_t& t) { m_horizontal_time = t; }
	void set_vertical_time(const uint32_t& t) { m_vertical_time = t; }
	void set_turn_time(const uint32_t& t) { m_turn_time = t; }
	
	static const coord_t c_grid_width;
	static const coord_t c_grid_height;
	static const coord_t c_cell_width;
	static const coord_t c_cell_height;
	static const float c_p2v_k;
	static const float c_p2v_q;
	static const float c_wheel_distance;
	
private:
	coord_t clamp_x(const coord_t& x) { return avrlib::clamp(x, 0, c_grid_width-1); }
	coord_t clamp_y(const coord_t& y) { return avrlib::clamp(y, 0, c_grid_height-1); }
	void set_motor_speed(const speed_t& s)
	{
		m_motor_speed = 25 * (s + 1);
		const float v = (c_p2v_k * m_motor_speed + c_p2v_q) / 1000; // because of ms
		m_vertical_time = c_cell_height / v;
		m_horizontal_time = c_cell_width / v;
		m_turn_time = c_wheel_distance * M_PI_4 / ((c_p2v_k * 25 + c_p2v_q) / 1000);
		//format(uart, "speed % \n\ts: % \n\tv: % \n\th: % \n\tt: % \n") % s % m_motor_speed % m_vertical_time % m_horizontal_time % m_turn_time;
	}
	coord_t m_x;
	coord_t m_y;
	dir_t m_dir;
	speed_t m_speed;
	int16_t m_motor_speed;
	uint32_t m_horizontal_time;
	uint32_t m_vertical_time;
	uint32_t m_turn_time;
};

const grid_move_t::coord_t grid_move_t::c_grid_width = 15;
const grid_move_t::coord_t grid_move_t::c_grid_height = 10;
const grid_move_t::coord_t grid_move_t::c_cell_width = 200;
const grid_move_t::coord_t grid_move_t::c_cell_height = 150;
const float grid_move_t::c_p2v_k = 4.5807; // measured on 3pi
const float grid_move_t::c_p2v_q = -32.472; // measured on 3pi
const float grid_move_t::c_wheel_distance = 82; // in mm

#endif