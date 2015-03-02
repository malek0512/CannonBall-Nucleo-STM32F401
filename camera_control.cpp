//camera_control.cpp

#include "camera_control.h"

//define IO

Serial usb(SERIAL_TX, SERIAL_RX);
PwmOut servo(D9); //PWM on the D9 connector
InterruptIn detect1(D2);
InterruptIn detect2(D3);
InterruptIn detect3(D4); // set utrasonic connectors

Timer timer;
int time1,time2,time3;
uint8_t reach1,reach2,reach3;
uint16_t angle = 500; // set the servo to 0 T


Thread * Treatement_thread;
Thread * Camera_set_thread;

void trigger1() {
	time1 = timer.read_us();
	reach1 = true;
	detect1.disable_irq(); // disable to prevent multi rises
}
void trigger2() {
	time2 = timer.read_us();
	reach2 = true;
	detect2.disable_irq(); // disable to prevent multi rises
}
void trigger3() {
	time3 = timer.read_us();
	reach3 = true;
	detect3.disable_irq(); // disable to prevent multi rises
}

void ultrason_receive(void const *argument) { // using in a thread

		Treatement_thread = new Thread(treatement);
		Camera_set_thread = new Thread(set_camera);
	
		timer.start();
	
    while (true) {
			time1 = time2 = time3 = 0;
			reach1 = reach2 = reach3 = false;
			
			timer.reset(); //RAZ timer
			
			detect1.enable_irq(); // allows interrupt
			detect2.enable_irq();
			detect3.enable_irq();
			
			detect1.rise(&trigger1);
			detect2.rise(&trigger2);
			detect3.rise(&trigger3);
			
			usb.printf("\nWaiting for signal(s)..\n");
			
			while (!(reach1 | reach2 | reach3)) {} // Wait for a receive
			Thread::wait(5);
			
			Treatement_thread->signal_set(0X01);
			usb.printf("Signal(s) reached :\n");
			usb.printf("1)\treach = %d\ttime = %d\n",reach1,time1);
			usb.printf("2)\treach = %d\ttime = %d\n",reach2,time2);
			usb.printf("3)\treach = %d\ttime = %d\n",reach3,time3);
				
			detect1.disable_irq(); // disable interrupt
			detect2.disable_irq();
			detect3.disable_irq();
								
    }
}


void treatement(void const *argument) {
	/*
	              DU1>--------------------- angle = 0 + i*360 ; i entier
								 *
							 *   *
						 *       *
					 *  *  *  *  *
				DU2             DU3>------------- angle = 120 + i*360;
				  >------------------------------ angle = 240 + i*360;
	*/
	const float SuD = 340/0.2f;  // Speed divided by the distance between 2 points
	uint8_t combination;
	uint8_t round = 3; // initialy set the camera in the middle
	while(true) {
		Thread::signal_wait(0X01); // wait
		combination = (reach3 << 2) + (reach2 << 1) + reach1 ;  //the 3 lasts bits are all the reach's possibilities 
		switch(combination) {
			case 1 : // DU1 is reached
				angle = 0;
				break;
		
			case 2 : // DU2 is reached
				angle = 240;
				break;
			
			case 3 : // DU1 and DU2 are reached
				angle = 57.3 * asin(double((time2 - time1)* SuD)) + 300; //determined in the repport , 57.3 = 180 / pi
				break;
			
			case 4 : // DU3 is reached
				angle = 120;
				break;
			
			case 5 : // DU1 and DU3 are reached
				angle = 60 - 57.3 * asin(double((time1 - time3)* SuD)); //determined in the repport
				break;
			
			case 6 : // DU2 and DU3 are reached
				angle = 57.3 * asin(double((time3 - time2)* SuD)) + 180; //determined in the repport
				break;
		
			case 7 : // DU1 and DU2 and DU3 are reached
				if(time1 >= time2 & time1 >= time3) // DU1 is rejected, DU2 and DU3 are reached
					angle = 57.3 * asin(double((time3 - time2)* SuD)) + 180; // as case 6
				else if(time2 >= time1 & time2 >= time3) // DU2 is rejected, DU1 and DU3 are reached
					angle = 60 - 57.3 * asin(double((time1 - time3)* SuD)); // as case 5
				else // DU3 is rejected, DU1 and DU2 are reached
					angle = 57.3 * asin(double((time2 - time1)* SuD)) + 300; // as case 3
				break;
		 
			case 0 : // no reach	
			default :
				break;
		}
		angle = angle + 360 * round;
		Camera_set_thread->signal_set(0X01);
		usb.printf("angle is compute : %d\n",angle);
	}	
}
	

void set_camera(void const *argument) { // between 0 and 2160 (= 6*360°)
	while(true) {
		Thread::signal_wait(0X01);
		if( angle > 2160) exit(0);
			servo = float(angle) / 43200.0f + 0.05f; //initialy 0.05 for 0T, 0.10 for 6T
			usb.printf("Camera is set\n");
	}
	
}
