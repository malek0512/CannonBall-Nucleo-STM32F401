#include "mbed.h"
#include "rtos.h"

Serial usb(SERIAL_TX, SERIAL_RX);

DigitalOut myled(LED1);
DigitalIn mybutton(USER_BUTTON);
PwmOut STEERING_SERVO_PIN(D5); // set servo-motor connector for steering
PwmOut THROTTLE_SERVO_PIN(D6); // set servo-motor connector for throttle
DigitalOut led(D13);

InterruptIn detect1(D2);
InterruptIn detect2(D3);
InterruptIn detect3(D4); // set utrasonic connectors
InterruptIn detectTX(SERIAL_TX);	
	
int steeringTarget = 90;
int throttleTarget = 91;

unsigned long time_rc_handler = 0;
unsigned long time_data_check = 0;
unsigned long last_time_data_check = 0;
unsigned long last_time_rc_handler = 0;
unsigned long period = 0;
bool emergency = false;

Timer timer;
int time1,time2,time3;
bool reach1,reach2,reach3;





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

void ultrason_receive(void const *argument) {
	
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
			
			usb.printf("\nAttente..\n");
			
			while (!(reach1 | reach2 | reach3)) {}
			wait_ms(5);
			myled= !myled;
				
			usb.printf("Signal recu :\n");
			usb.printf("1)\treach = %d\ttime = %d\n",reach1,time1);
			usb.printf("2)\treach = %d\ttime = %d\n",reach2,time2);
			usb.printf("3)\treach = %d\ttime = %d\n",reach3,time3);
				
			reach1 = reach2 = reach3 = false;
    }
}
		
		

/*
void usb_communication_thread(void const *argument) {
	Serial usb(SERIAL_TX, SERIAL_RX);
	usb.baud(9600);
	usb.printf("send somthing");
	usb.scanf("receive something");	
}
		
void led1(void const *argument) {		
    while (true) {
        if(mybutton == false)
						myled= true;
				else
						myled = false;
    }
}*/

void USB_recieve(void const *argument)
{
	while(true)
		{
		time_data_check=timer.read_us();
		usb.scanf("%d",&steeringTarget);
		usb.scanf("%d",&throttleTarget);
		last_time_data_check = time_data_check;
			
		 if (emergency || (time_data_check - last_time_data_check > 1000))
			 {
						led=true;
        if (throttleTarget < 89) { //high speed
            for (;;)
                STEERING_SERVO_PIN.write(130); //brake
        }
        else {
            for (;;)
                THROTTLE_SERVO_PIN.write(91); //only slow down, not to go reverse
        }
		
			}
		}
}
// go back to this function to know if we use it or no.
void rc_handler()
{
	    time_rc_handler = timer.read_us(); 
			period = time_rc_handler - last_time_rc_handler;
    if (period < 1000) {
        //We've got an emergency, stop everything
        emergency = true;
    }
    last_time_rc_handler = time_rc_handler;
}



int main() {
    //Thread th1(led1);	
		timer.start();
		detectTX.enable_irq();	
		detectTX.rise(&rc_handler);
		detectTX.fall(&rc_handler);
	
		Thread th2(ultrason_receive);
		Thread th4(USB_recieve);
    while(1) {}
}