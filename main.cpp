//main.cpp

/********************************************
Projet CONNONBALL using NUCLEO STM32F401RE
								Year 2015 
					Created by MAJiiiC Team
					
			@mail jerome_laurent16@yahoo.fr
*********************************************/

//Include Library
#include "mbed.h"
//#include "rtos.h"
#include "SerialDriver.h"


//IO definition
SerialDriver usb(USBTX, USBRX,2,2);

InterruptIn detect1(D2);
InterruptIn detect2(D3);
InterruptIn detect3(D4); // set utrasonic connectors
PwmOut servo(D9); //PWM on the D9 connector

PwmOut STEERING_SERVO_PIN(D5); // set servo-motor connector for steering
PwmOut THROTTLE_SERVO_PIN(D6); // set servo-motor connector for throttle
DigitalOut led(LED1);

// PWM car control
uint8_t steeringTarget;
uint8_t throttleTarget;

//Timers
Timer timer1;
Timer timer2;

//Variables
unsigned int time_data_check;


//Main function
int main() {
	led = false;
	
	usb.baud(115200);
	STEERING_SERVO_PIN.write(90/255); //initialisation
	THROTTLE_SERVO_PIN.write(91/255);
	timer1.start();
	time_data_check = timer1.read_us();
	
	while(true)
		{
			//usb.printf("Ready to get data\n");
			time_data_check=timer1.read_ms();
			
			steeringTarget = usb.getc();
			throttleTarget = usb.getc();
			
			usb.putc(steeringTarget);
			usb.putc(throttleTarget);
	
			STEERING_SERVO_PIN.write(steeringTarget/255);
			THROTTLE_SERVO_PIN.write(throttleTarget/255);		
			
			if ((timer1.read_ms() - time_data_check > 5000)) //the delay is to hight
			{ 
					//usb.printf("Emergency as occured\n");
					led = true;
					if (throttleTarget < 89) //high speed
							STEERING_SERVO_PIN.write(130/255); //brake
					else 
							THROTTLE_SERVO_PIN.write(91/255); //only slow down, not to go reverse
	
					led = false;
					time_data_check = timer1.read_ms();
			}

		}

}
