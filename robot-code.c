#pragma config(I2C_Usage, I2C1, i2cSensors)
#pragma config(Sensor, in1,    ir_sensorCL,    sensorReflection)
#pragma config(Sensor, in2,    ir_sensorLO,    sensorReflection)
#pragma config(Sensor, dgtl1,  button2,        sensorTouch)
#pragma config(Sensor, dgtl2,  button1,        sensorTouch)
#pragma config(Sensor, dgtl3,  button3,        sensorTouch)
#pragma config(Sensor, dgtl4,  stop_button,    sensorTouch)
#pragma config(Sensor, dgtl10, led,            sensorNone)
#pragma config(Sensor, dgtl11, ping_sensor,    sensorSONAR_mm)
#pragma config(Sensor, I2C_1,  ,               sensorQuadEncoderOnI2CPort,    , AutoAssign )
#pragma config(Sensor, I2C_2,  ,               sensorQuadEncoderOnI2CPort,    , AutoAssign )
#pragma config(Motor,  port1,           lift_motor,    tmotorVex393_HBridge, openLoop)
#pragma config(Motor,  port2,           right_motor,   tmotorVex393_MC29, openLoop, reversed, encoderPort, I2C_1)
#pragma config(Motor,  port3,           left_motor,    tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port10,          state_motor,   tmotorVex393_HBridge, openLoop, encoderPort, I2C_2)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

// constants for move function
#define FORWARD 1
#define REVERSE 2
#define LEFT 3
#define RIGHT 4
#define WIDE_RIGHT 5
#define WIDE_LEFT 6
#define STOP 0

// constant definitions
#define WHEEL_CIRC 0.44 // diam = 0.103m (10.3cm) | 0.103*PI = 0.324m circ.
#define ROBOT_WIDTH 0.26 // 0.26m (26cm) wide
#define MOTOR_ENCODER_REV 627
#define PI 3.14159

#define LED_OFF 0
#define LED_ON 1

#define LED_F1 1
#define LED_F2 2
#define LED_F3 3

#define LONG_IR 0
#define SHORT_IR 1
#define IR_THRESHOLD 750

// function signature definitions
void move(int dir, int speed);
void slow(int cur_pulses, int tot_pulses, int final_speed, int start_speed);
void led_signal(int status);
void state_signal(int mode);

// milestone 1
int drop_the_ball();
void drive_one_m();
void quarter_rotate(int input_dir);

// milestone 2
void state_machine();
int scan_environment(int sensor);
int move_to_target(int sensor);
int line_the_shot();

void distance_test();

void ir_sensor_read(int sensor);
void narrow_search(int sensor, int speed);
void ping_sensor_test();
int detect_corner();

// helper functions
void run_test(int test);
int button_press();

// Global Variables
int state = 1;
int state_motor_pos = 0;

int ir_LO_min = 4095;
int ir_CL_min = 4095;
int motorEncoder_pos = 0;
int ping_min_val = 9999;
//int ping_ME_pos = 0;

// ---------------------------
// --------- Main ------------
// ---------------------------
task main()
{
	button_press();
	state_signal(0);
	wait1Msec(1000);

	while (state != 6)
	{
		state_machine();
	}
	move(STOP, 0);
}

// ---------------------------
// ----- State Functions -----
// ---------------------------

/****************************************************************************
Function: The initial state of the robot is to scan the environment looking
					for the target with the IR sensor. The robot will complete a full
					rotation reading inputs from the IR sensor. When a minimum IR reading
					is taken, both the value and right_motor encoder encoder position are
					recorded. The recorded motor encoder position is then used to orient
					the robot towards the target.
Inputs:		int sensor - The IR sensor used to read IR values.
Return:		int - The integer associated to the next state of the robot.
*/
int scan_environment(int sensor)
{
	float wheel_revs = (PI * ROBOT_WIDTH) / WHEEL_CIRC;
	int right_pulses = 1.3 * wheel_revs * MOTOR_ENCODER_REV;

	resetMotorEncoder(right_motor);

	move(RIGHT, 40);
	// Look for IR sensor reading
	while (getMotorEncoder(right_motor) < right_pulses){ir_sensor_read(sensor);}

	move(STOP, 0);
	wait1Msec(500);
	move(LEFT, 40);
	// Return to position of strongest signal position
	while (getMotorEncoder(right_motor) > motorEncoder_pos - 30){/*do nothing*/}

	move(STOP, 0);
	wait1Msec(100);

	ir_LO_min = 4095;
	ir_CL_min = 4095;

	//led_signal(LED_F1);
	return 2;
}

/************************************************************************
Function: After completion of finding and orienting to the target, the
					robot will then move to the target until it gets to a ping sensor
					reading of 400. It will then shift into the next state.
Inputs:		int sensor - The IR sensor used to read IR values.
Return:		int - The integer associated to the next state of the robot.
*/
int move_to_target(int sensor)
{
	int count = 0;
	move(FORWARD, 20);
	while (SensorValue(ping_sensor) > 400)
	{
		ir_sensor_read(LONG_IR);

		if (count%1000 == 999)
		{
			// if the robot is not pointing towards target, search.
			if (ir_LO_min > IR_THRESHOLD) {
				wait1Msec(1000);
				narrow_search(sensor, 40);
				move(FORWARD, 20);
			}

			ir_LO_min = 4095;
			ir_CL_min = 4095;
		}

		count++;
	}
	move(STOP, 0);
	wait1Msec(1000);
	//led_signal(LED_F2);
	return 3;
}

/*************************************************************************
Function: Once the robot is within range of the target it performs a narrow
					search to hone in its orientatin towards the target. It will then
					move slowly to the target until the stop switch is triggered.
Inputs:		NA
Return:		int - The integer associated to the next state of the robot.
*/
int line_the_shot()
{
	narrow_search(LONG_IR, 40);
	wait1Msec(500);

	move(FORWARD, 15);
	while(SensorValue(stop_button) != 1 || SensorValue(ping_sensor) > 65){/*do nothing*/} //SensorValue(ping_sensor) > 50 &&
	move(STOP, 0);

	return 4;
}

/*******************************************************************
Function: The visual signal that the robot has completed the current
					state and is moving into the next state.
Inputs:		NA
Return:		int - The integer associated to the next state of the robot.
*/
int drop_the_ball()
{
	wait1Msec(1000);
	motor[lift_motor] = 25;
	wait1Msec(1500);
	motor[lift_motor] = 0;
	wait1Msec(2000);
	motor[lift_motor] = -20;
	wait1Msec(1000);
	motor[lift_motor] = 0;
	wait1Msec(1000); // maybe remove

	return 5;
}

/*******************************************************************
											KAI TODO:
Function: Moves the robot to its final position near
Inputs:		NA
Return:		int - The integer associated to the next state of the robot.
*/
int end_positioning()
{
	// After droppin the object, Robot will do 360 and using sensor robbot can detect  all walls.
	move(REVERSE, 15);
	wait1Msec(1500);
	// Detect furthest wall to the robot.
	move(RIGHT, 40);//do a small rotation
	wait1Msec(800);

	detect_corner();

	move(STOP, 0);//stop the robot from rotating
	wait1Msec(500);

	move(RIGHT, 40);//rotate slightly to the left
	wait1Msec(500);

	// Drive to the furthest wall, implemet functionn that stops the robot from hitting the wall ( x-amount of distance from wall). 	After that robot can do another 360 and stop at poistion facing the target.
	move(FORWARD, 20);//move into the corner
	while (SensorValue(ping_sensor) > 250){/*do nothing*/}
	move(STOP, 0);


	return 6;
}


int detect_corner()
{
		int previous_reading = 999999;

		int current_reading = SensorValue(ping_sensor);

		while((current_reading - previous_reading) < 0)//make sure the derivative of the distance is negative
		{
			previous_reading = current_reading;
			current_reading = SensorValue(ping_sensor);
		}

		int largestDist = 0;

		while(true)//find the corner
		{
			if(current_reading >= largestDist) {largestDist = current_reading; }
			else{break;}
			current_reading = SensorValue(ping_sensor);
		}

		return largestDist;

}

/*******************************************************************
											KAI TODO:
Function: Action that signals the change of state of the robot.
Inputs:		NA
Return:		NA
*/
void state_signal(int mode)
{
	switch (mode)
	{
		case 0:
			// rotate until button press
			motor[state_motor] = 10;
			wait1Msec(50);
			while (SensorValue(button2) == 0) {}
			motor[state_motor] = 0;
			resetMotorEncoder(state_motor);
			break;

		case 1:
			motor[state_motor] = 13;

			state_motor_pos += 125;

			while((-1)*GetMotorEncoder(state_motor) < state_motor_pos) {}

			motor[state_motor] = 0;

			// rotates same amount for all state changes
			break;

		case 2:
			//big finnish excitement spinning!!
			motor[state_motor] = 25;
			wait1Msec(2500);
		  motor[state_motor] = 0;
			break;

		default:
			break;
	}
}

/**************************************************************************************
Function: The robot performs a full rotation to the right. While rotating, the robot is
					checking for IR reading values below the threshold set. If a reading is found
					to be less than the threshold, both the reading and the right_motor enocder
					position is recorded. The robot looks for and records the minimum IR value
					and position. The position is then used to orient the robot towards the target.
Inputs:		int sensor - The IR sensor being used to read the sensor values.
Return:		NA
*/
void ir_sensor_read(int sensor)
{
	int ir_val = 0;

	if (SensorValue(ping_sensor) < 150)
	{
		move(STOP, 0);
		wait1Msec(100);

		move(REVERSE, 20);
		wait1Msec(1000);
		move(STOP, 0);
		resetMotorEncoder(right_motor);
		ir_LO_min = 4095;
		ir_CL_min = 4095;
		move(RIGHT, 40);
	}

	switch (sensor)
	{
		// ---------- use LONG range IR detection (100k ohm) -----------
		case LONG_IR:
			for (int i=0; i<2 ;i++) {ir_val += SensorValue(ir_sensorLO);}
			ir_val /= 2;

			if ( ir_val < ir_LO_min && ir_val < IR_THRESHOLD)
			{
				ir_LO_min = ir_val;
				motorEncoder_pos = getMotorEncoder(right_motor);
			}
			break;

		// ---------- use SHORT range IR detection (1k ohm) ------------
		case SHORT_IR:
			for (int i=0; i<2 ;i++) {ir_val += SensorValue(ir_sensorCL);}
			ir_val /= 2;

			if ( ir_val < ir_CL_min && ir_val < IR_THRESHOLD)
			{
				ir_CL_min = ir_val;
				motorEncoder_pos = getMotorEncoder(right_motor);
				ping_min_val = SensorValue(ping_sensor);
			}
			break;
	}
}

/**********************************************************************
Function: The robot does a wide sweep left and right reading inputs from
					the IR sensor. While sweeping, the robot looks for the minimum
					IR sensor value and captures the value and the current right_motor
					encoder position with which it will use to return its orientation
					towards the target.
Inputs:		int sensor - The IR sensor being used to detect the target.
					int speed - the speed with which the motors are set for the sweep.
Return:		NA
*/
void narrow_search(int sensor, int speed)
{
	ir_LO_min = 4095;
	ir_CL_min = 4095;
	ping_min_val = 9999;

	move(STOP, 0);
	wait1Msec(100);
	resetMotorEncoder(right_motor);

	move(WIDE_LEFT, speed);
	// narrow search left
	while (getMotorEncoder(right_motor) > -250){ir_sensor_read(sensor);}

	move(STOP, 0);
	wait1Msec(500);

	move(WIDE_RIGHT, speed);
	// narrow search right
	while (getMotorEncoder(right_motor) < 125){ir_sensor_read(sensor);}

	move(STOP, 0);
	wait1Msec(500);

	move(WIDE_LEFT, speed);
	while(SensorValue(ir_sensorLO) > ir_LO_min + 5 || getMotorEncoder(right_motor) > motorEncoder_pos - 10){/*do nothing*/}

	move(STOP, 0);
}

/*****************************************************************
Function: The state machine runs all of the states of the robot
					in order. The state globally starts at 1, calling the first
					state function. That function then returns an incrimented
					state value triggering the next state function. This repeats
					until all states are completed.
Inputs:		NA
Return:		NA
*/
void state_machine()
{
		switch (state)
	{
			case 1:
				state = scan_environment(LONG_IR);
				state_signal(1);
				break;

			case 2:
				state = move_to_target(LONG_IR);
				state_signal(1);
				break;

			case 3:
				state = line_the_shot();
				state_signal(1);
				break;

			case 4:
				state = drop_the_ball();
				state_signal(1);
				break;

			case 5:
				state = end_positioning();
				state_signal(2);
				break;

			default:
				break;
	}
}

/*******************************************************************
Function:	Sets the motors to move the car in the specified direction
					and speed.
					- FORWARD
					- REVERSE
					- LEFT
					- RIGHT
					- WIDE_RIGHT
					- WIDE_LEFT
					- STOP (default)
Inputs:		int dir - Specified direction of movement
					int speed - The speed of rotation of the motors
Return:		NA
*/
void move(int dir, int speed)
{
	int neg_speed = 0 - speed;

	switch (dir)
	{
		case 1:
			// move forward
			motor[right_motor] = speed;
			motor[left_motor] = speed;
			break;

		case 2:
			// move backward
			motor[right_motor] = neg_speed;
			motor[left_motor] = neg_speed;
			break;

		case 3:
			// rotate left
			motor[right_motor] = speed;
			motor[left_motor] = neg_speed;
			break;

		case 4:
			// rotate right
			motor[right_motor] = neg_speed;
			motor[left_motor] = speed;
			break;

		case 5:
			// scan wide right
			motor[right_motor] = neg_speed;
			motor[left_motor] = 25;
			break;

		case 6:
			// scan wide left
			motor[right_motor] = speed;
			motor[left_motor] = -25;
			break;

		default:
			// stop motors
		motor[right_motor] = 0;
		motor[left_motor] = 0;
		break;
	}
}

// ---------------------------
// -- Milestone 2 Functions --
// ---------------------------

/*
Function: Drive the robot forward. When the ping sensor reads a value
					less than 250 stop the robot.
Inputs:		NA
Return:		NA
*/
void ping_sensor_test()
{
	move(FORWARD, 20);
	while (SensorValue(ping_sensor) > 250){/*do nothing*/}
	move(STOP, 0);

	state_signal(1);
}

/*
Function: Checks whether or not the robot is within the right
					distance from the target to drop the payload.
					Lift_motor spins clockwise when too far, counter-clockwise
					when too close, and is stopped when in the correct position.
Inputs:		NA
Return:		NA
*/
void distance_test()
{
	wait1Msec(1000);
	while (SensorValue(button2) == 0)
	{
		if (SensorValue(ping_sensor) > 60) {
			motor[lift_motor] = 20;
		} else if (SensorValue(ping_sensor) < 40) {
			motor[lift_motor] = -20;
		} else {
			motor[lift_motor] = 0;
		}
	}
}

// ---------------------------
// -- Milestone 1 Functions --
// ---------------------------

/*
Function:	Raise the release mechanism to drop the ball.
Inputs:		NA
Return:		NA
*/
/*void drop_the_ball()
{
	motor[lift_motor] = 20;
	wait1Msec(1500);
	motor[lift_motor] = 0;
	wait1Msec(2500);
	motor[lift_motor] = -20;
	wait1Msec(1000);
	motor[lift_motor] = 0;
}*/

/*
Function:	Move forward a distance of 1 meter.
Inputs:		NA
Return:		NA
*/
void drive_one_m()
{
	// 1 meter / circumference of wheel
	float wheel_revs = 1.0/WHEEL_CIRC;
	int pulses = wheel_revs * MOTOR_ENCODER_REV * -1;

	resetMotorEncoder(right_motor);

	move(FORWARD, 25);
	while (getMotorEncoder(right_motor) > pulses)
	{
		if (getMotorEncoder(right_motor) < (pulses + 627))
		{
			slow(getMotorEncoder(right_motor), pulses, 20, 25);
		}
	}

	move(STOP, 0);
}

/*
Function:	Rotate the robot 90 degrees.
Inputs:		NA
Return:		NA
*/
void quarter_rotate(int input_dir)
{
	// dist along arc for 90 / circumference of wheel
	float wheel_revs = ((PI * ROBOT_WIDTH) / 4) / WHEEL_CIRC;
	int right_pulses = 1.2 * wheel_revs * MOTOR_ENCODER_REV;
	int left_pulses = right_pulses * -1;

	resetMotorEncoder(right_motor);

	move(input_dir, 40);

	// taking into account the direction of rotation of the encoder (+/-)
	if (input_dir != RIGHT) {
		while (getMotorEncoder(right_motor) > left_pulses){/*do nothing*/}
	} else {
		while (getMotorEncoder(right_motor) < right_pulses){/*do nothing*/}
	}

	move(STOP, 0);
}

// ---------------------------
//   -- Helper Functions --
// ---------------------------

/*
Function:	Slows the speed of the robot when it gets within
					a certain range of an object.
Inputs:		int cur_pulses - the current state of the getMotorEncoder().
					int tot_pulses - the exit condition when the car should stop.
					int final_speed - the specified final speed for the robot to slow to.
					int start_speed - the initial speed before starting to slow.
Return:		NA
*/
void slow(int cur_pulses, int tot_pulses, int final_speed, int start_speed)
{
	int diff_pulses = 0 - tot_pulses + cur_pulses;
	int diff_speed = start_speed - final_speed;

	int new_speed = start_speed - (diff_speed - (diff_pulses / 627)*diff_speed);

	if (new_speed < final_speed) new_speed = final_speed;

	move(FORWARD, new_speed);
}

/*
Function:	Use for the purpose of testing the implementation
					of new functions.
Inputs:		int test - Identifies the test we want to run.
Return:		NA
*/
void run_test(int test)
{
	wait1Msec(1500);
	switch (test)
	{
		case 0:
			// tests locate and move to position
			scan_environment(LONG_IR);
			state_signal(1);
			break;

		case 1:
			distance_test();
			//state = 2;
			//while (state != 4) {state_machine();}

			break;

		case 2:
			ping_sensor_test();
			break;

		default:
			// do nothing
			break;
	}
}

/*
Function: Reads one of the three buttons on the robot and
					returns a unique integer identifier.
Inputs:		NA
Return:		int - The identifying integer relative to the button pressed.
*/
int button_press()
{
	while (true)
	{
		if (SensorValue(button1) == 1) return 0;
		if (SensorValue(button2) == 1) return 1;
		if (SensorValue(button3) == 1) return 2;
	}
}

/*
Function:
Inputs:		NA
Return:		NA
*/
void led_signal(int status) // Fix or change------------------------------
{
	switch (status)
	{
		case LED_OFF:
			SensorValue(led) = LED_OFF;
			break;

		case LED_F1:
			for (int i=0;i<4;i++)
			{
				SensorValue(led) = LED_ON;
				wait1Msec(125);
				SensorValue(led) = LED_OFF;
				wait1Msec(500);
			}
			break;

		case LED_F2:
			for (int i=0;i<4;i++)
			{
				for (int j=0;j<2;j++)
				{
					SensorValue(led) = LED_ON;
					wait1Msec(1000);
					SensorValue(led) = LED_OFF;
					wait1Msec(250);
				}
				wait1Msec(250);
			}
			break;

		case LED_F3:
			for (int i=0;i<4;i++)
			{
				for (int j=0;j<3;j++)
				{
					SensorValue(led) = LED_ON;
					wait1Msec(1000);
					SensorValue(led) = LED_OFF;
					wait1Msec(250);
				}
				wait1Msec(250);
			}
			break;
	}
}
