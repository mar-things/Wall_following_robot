#include "utils.h"
#include "math.h"
#include "nrf.h"
#include "motor.h"
#include "servo.h"

uint8_t escaping = 0;
int8_t auto_speed;
uint16_t max_laser_distance;
uint16_t min_laser_distance;
uint16_t target_wall_distance;
uint16_t target_circle_radius;
uint8_t laser_count;
float steering_sensitivity;

float select_target(const float x1, const float x2, const float y_intercept, const uint8_t followed_wall)
{
	float mapped;

	if (followed_wall == 0)
	{
		if (y_intercept > .0f)
		{
			if (x1 > x2)
				mapped = mapf(-(float)target_circle_radius, (float)target_circle_radius, -100.f, 100.f, x1);
			else
				mapped = mapf(-(float)target_circle_radius, (float)target_circle_radius, -100.f, 100.f, x2);
		}
		else
		{
			if (x1 > x2)
				mapped = mapf(-(float)target_circle_radius, (float)target_circle_radius, -100.f, 100.f, x2);
			else
				mapped = mapf(-(float)target_circle_radius, (float)target_circle_radius, -100.f, 100.f, x1);
		}
	}
	else
	{
		if (y_intercept > .0f)
		{
			if (x1 > x2)
				mapped = mapf(-(float)target_circle_radius, (float)target_circle_radius, -100.f, 100.f, x2);
			else
				mapped = mapf(-(float)target_circle_radius, (float)target_circle_radius, -100.f, 100.f, x1);
		}
		else
		{
			if (x1 > x2)
				mapped = mapf(-(float)target_circle_radius, (float)target_circle_radius, -100.f, 100.f, x1);
			else
				mapped = mapf(-(float)target_circle_radius, (float)target_circle_radius, -100.f, 100.f, x2);
		}
	}

	return mapped;
}

uint8_t any_walls_detected(const uint16_t* lasers, const uint8_t followed_wall)
{
	if (followed_wall == 0)
	{
		for (uint8_t i = 0; i < laser_count; ++i)
			if (lasers[i] <= max_laser_distance)
				return 1;
	}
	else
	{
		for (uint8_t i = 7 - laser_count; i < 7; ++i)
			if (lasers[i] <= max_laser_distance)
				return 1;
	}

	return 0;
}

uint8_t need_escaping(const uint16_t* lasers, float scaling)
{
	for (uint8_t i = 2; i <= 4; ++i)
		if (lasers[i] <= (uint16_t)((float)min_laser_distance * scaling))
			return 1;

	return 0;
}

void widest_laser_pair(const uint16_t* lasers, uint8_t* left, uint8_t* right, const uint8_t followed_wall)
{
	if (followed_wall == 0)
	{
		*left = 0;
		*right = laser_count - 1;
	}
	else
	{
		*left = 7 - laser_count;
		*right = 6;
	}

	while (lasers[*left] > max_laser_distance)
		(*left)++;

	while (lasers[*right] > max_laser_distance)
		(*right)--;
}

void on_single_detection(uint8_t index)
{
	//Turn wheels towards detected wall
	const float mapped = mapf(0.f, 6.f, -100.f, 100.f, (float)index);
	servo_set((int8_t)mapped);
}

void on_multiple_detections(const uint16_t* lasers,
							const uint8_t left_index,
							const uint8_t right_index,
							const uint8_t followed_wall)
{
	//Angle in radians between horizontal axis and left laser (clockwise)
	const float angle_left = deg2rad((float)(left_index) * 30.f);

	//Angle in radians between horizontal axis and right laser (counter clockwise)
	const float angle_right = deg2rad((float)(6 - right_index) * 30.f);

	//Left laser intersection with wall coordinates
	const float x_left = (float)lasers[left_index] * -cosf(angle_left);
	const float y_left = (float)lasers[left_index] * sinf(angle_left);

	//Right laser intersection with wall coordinates
	const float x_right = (float)lasers[right_index] * cosf(angle_right);
	const float y_right = (float)lasers[right_index] * sinf(angle_right);

	//Slope of the wall
	const float slope = (y_right - y_left) / (x_right - x_left);

	//Convert slope to angle (between horizontal axis and wall)
	const float angle = atanf(slope);

	//Calculate points on the imaginary wall which has been offset by TARGET_WALL_DISTANCE
	float x1, y1;

	const float y_intercept_wall = y_left - slope * x_left;

	if (y_intercept_wall > 0.f) //If slope is positive add positive offset to x
	{
		x1 = x_left + (float)target_wall_distance * sinf(angle);
		y1 = y_left - (float)target_wall_distance * cosf(angle);
	}
	else //If slope is negative add negative offset to x
	{
		x1 = x_left - (float)target_wall_distance * sinf(angle);
		y1 = y_left + (float)target_wall_distance * cosf(angle);
	}

	//Calculate the y intercept of imaginary wall
	const float y_intercept_target = y1 - x1 * slope;

	//Calculate intersection between robot's circle and imaginary wall
	const float f = 1 + slope * slope;
	const float g = 2 * slope * y_intercept_target;
	const float h = y_intercept_target * y_intercept_target - (float)target_circle_radius * (float)target_circle_radius;

	float D = g * g - 4 * f * h;

	if (D < 0.f)
		D = 0.f;
	else
		D = sqrtf(D);

	const float target_x1 = (-g + D) / 2.f / f;
	const float target_x2 = (-g - D) / 2.f / f;

	//Pick which point is best to follow
	const float mapped = select_target(target_x1, target_x2, y_left - x_left * slope, followed_wall);

	//Scale mapped value by steering_sensitivity to increase sensitivity and clamp to [-100;100] range
	servo_set((int8_t)clampf(mapped * steering_sensitivity, -100.f, 100.f));
}

void on_escape(const uint8_t followed_wall, const int8_t boost)
{
	if (followed_wall == 0)
	{
		servo_set(-100);
	}
	else
	{
		servo_set(100);
	}

	motor_set_speed(-auto_speed, boost);
}

void auto_controls(const uint16_t* lasers,
		const uint8_t followed_wall,
		const int8_t boost)
{
	if (need_escaping(lasers, 1.f))
	{
		escaping = 1;
		on_escape(followed_wall, boost);
		return;
	}

	if (need_escaping(lasers, 1.5f) && escaping == 1)
	{
		on_escape(followed_wall, boost);
		return;
	}
	else
	{
		escaping = 0;
	}

	motor_set_speed(auto_speed, boost);

	//If no walls detected go in followed wall direction
	if (any_walls_detected(lasers, followed_wall) == 0)
	{
		if (followed_wall == 0)
		{
			servo_set(-75);
		}
		else
		{
			servo_set(75);
		}


		return;
	}

	//Find widest pair
	uint8_t left_index;
	uint8_t right_index;
	widest_laser_pair(lasers, &left_index, &right_index, followed_wall);

	if (left_index == right_index) // Only one laser detected a wall
	{
		on_single_detection(right_index);
	}
	else //Multiple lasers detected a wall
	{
		on_multiple_detections(lasers, left_index, right_index, followed_wall);
	}
}

void auto_controls_init(
		const int8_t speed,
		const uint16_t max_laser,
		const uint16_t min_laser,
		const uint16_t target,
		const uint16_t radius,
		const uint8_t laser_amount,
		const float servo_sensitivity
)
{
	laser_count = laser_amount;
	max_laser_distance = max_laser;
	min_laser_distance = min_laser;
	target_wall_distance = target;
	target_circle_radius = radius;
	auto_speed = speed;
	steering_sensitivity = servo_sensitivity;
}
