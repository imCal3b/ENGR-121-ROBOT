#pragma config(I2C_Usage, I2C1, i2cSensors)
#pragma config(Sensor, I2C_1,  ,               sensorQuadEncoderOnI2CPort,    , AutoAssign )
#pragma config(Sensor, I2C_2,  ,               sensorQuadEncoderOnI2CPort,    , AutoAssign )
#pragma config(Motor,  port1,           lift_motor,    tmotorVex393_HBridge, openLoop)
#pragma config(Motor,  port2,           right_motor,   tmotorVex393_MC29, openLoop, encoderPort, I2C_1)
#pragma config(Motor,  port3,           left_motor,    tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port4,           ping_motor,    tmotorVex393_MC29, openLoop, encoderPort, I2C_2)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

// constants for move function
#define FORWARD 1
#define REVERSE 2
#define LEFT_TURN 3
#define RIGHT_TURN 4
#define STOP 0

// function signature definitions
void IR_scan();
void move(int dir, int speed);
void drop_the_ball();

// MAIN
// -------------------------------
task main()
{
	testing();
}
// ---------------------------------

/*
Function:	Use for the purpose of testing the implementation
			of new functions.
Inputs:		NA
Return:		NA
*/
void testing()
{
	move(FORWARD, 40);
	wait1Msec(5000); // 5 sec
	move(LEFT_TURN, 35);
	wait1Msec(3000);
	move(STOP, 0);
	wait1Msec(1500);
	move(RIGHT_TURN, 35);
	wait1Msec(3000);
	move(FORWARD, 40);
	wait1Msec(5000);
	move(STOP, 0);

	drop_the_ball();
	wait1Msec(1500);
	move(REVERSE, 30);
	wait1Msec(2000);
	move(STOP, 0);
}

/*
Function:	Scans the arena looking for an IR signal. Once
			a signal is detected, point the robot in the direction
			of the strongest signal reading.
Inputs:		NA
Return:		NA
*/
void IR_scan()
{
	//code
}

/*
Function:	Sets the motors to move the car in the specified direction
			and speed.
			- Forward
			- Backward
			- Left
			- Right
			- Stop (default)
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

		default:
			// stop motors
		motor[right_motor] = 0;
		motor[left_motor] = 0;
	}
}

/*
Function:	Raise the release mechanism to drop the ball.
Inputs:		NA
Return:		NA
*/
void drop_the_ball()
{
	motor[lift_motor] = 20;
	wait1Msec(1200);
	motor[lift_motor] = 0;
	wait1Msec(2000);
	motor[lift_motor] = -20;
	wait1Msec(1000);
	motor[lift_motor] = 0;
}
