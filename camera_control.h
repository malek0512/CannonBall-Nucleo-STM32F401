//camera_control.h

/*
Projet cannonball using NUCLEO STM32F401RE
Year 2015 created by MAJ-3i team 
email 
*/

#include "mbed.h"
#include "rtos.h"

#ifndef CAMERA_CONTROL_H
#define CAMERA_CONTROL_H

// Definition of triggers for the interupts
void trigger1();
void trigger2();
void trigger3();

//definition of the function used in threads
void ultrason_receive(void const *argument);

void treatement(void const *argument);
	
void set_camera(void const *argument);


#endif
