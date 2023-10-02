#ifndef INC_WALL_FOLLOW_H_
#define INC_WALL_FOLLOW_H_

//Set followed_wall to 0 to follow left wall
//Set followed_wall to 1 to follow right wall
void auto_controls(const uint16_t* lasers, const uint8_t followed_wall, const int8_t boost);

void auto_controls_init(
		const int8_t speed,
		const uint16_t max_laser,
		const uint16_t min_laser,
		const uint16_t target,
		const uint16_t radius,
		const uint8_t laser_amount,
		const float servo_sensitivity
);

#endif
