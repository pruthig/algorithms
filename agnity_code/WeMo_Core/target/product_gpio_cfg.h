/***************************************************************************
*
*
*
*
* Created by Belkin International, Software Engineering on XX/XX/XX.
* Copyright (c) 2012-2013 Belkin International, Inc. and/or its affiliates. All rights reserved.
*
* Belkin International, Inc. retains all right, title and interest (including all
* intellectual property rights) in and to this computer program, which is
* protected by applicable intellectual property laws.  Unless you have obtained
* a separate written license from Belkin International, Inc., you are not authorized
* to utilize all or a part of this computer program for any purpose (including
* reproduction, distribution, modification, and compilation into object code)
* and you must immediately destroy or return to Belkin International, Inc
* all copies of this computer program.  If you are licensed by Belkin International, Inc., your
* rights to utilize this computer program are limited by the terms of that license.
*
* To obtain a license, please contact Belkin International, Inc.
*
* This computer program contains trade secrets owned by Belkin International, Inc.
* and, unless unauthorized by Belkin International, Inc. in writing, you agree to
* maintain the confidentiality of this computer program and related information
* and to not disclose this computer program and related information to any
* other person or entity.
*
* THIS COMPUTER PROGRAM IS PROVIDED AS IS WITHOUT ANY WARRANTIES, AND BELKIN INTERNATIONAL, INC.
* EXPRESSLY DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED, INCLUDING THE WARRANTIES OF
* MERCHANTIBILITY, FITNESS FOR A PARTICULAR PURPOSE, TITLE, AND NON-INFRINGEMENT.
*
*
***************************************************************************/


#ifndef _PRODUCT_GPIO_CFG_H_
#define _PRODUCT_GPIO_CFG_H_
// These defines configure the features compiled into ralink_gpio.c

// Define POWER_DBL_CLICK to create a /proc/POWER_BUTTON_STATE
// to provide double click support for the power button
// 
// /proc/POWER_BUTTON_STATE:
// 0: button not pressed
// 1: button was clicked once
// 2: button was double clicked
// 3: button was held for a "long" time.

// #define POWER_DBL_CLICK	1

// Define MIN_OUT_DELAY to limit number of times the output relay can
// be turned on and off a second.  Measured in Jiffies.
// i.e. HZ/2 means the output can be turned on or off twice a second.

#define MIN_OUT_DELAY		(HZ/2)

// Define RELAY_ON_TIME to cause the relay to turn on for 
// RELAY_ON_TIME and then turn off.  
// When this mode is defined writes of a 1 to the relay control port
// turn on the relay and starts the timer, writes of a 0 are ignored.
// Writes of a 1 while the timer is running are ignored.
// The value of the define sets the default relay on time in jiffies.
// time can be changed by writing an new value to /proc/RELAY_ON_TIME
// i.e. to "# echo 800 >  /proc/RELAY_ON_TIME"

//#define RELAY_ON_TIME	10*HZ	// 10 seconds (time is measured in jiffies)

// Define PWM_SERIAL_PORT to use a serial port as a poor man's PWM to 
// allow the backlight LED to be dimmed.
// 
// LED is controlled using the following /proc files:
// 	/proc/BACKLIGHT_MODE
// 		0 - PWM disabled, serial port acts normally
// 		1 - PWM mode
// 		2 - PWM debug/demo mode, continuous ramping up and down
// 	/proc/BACKLIGHT_TARGET
// 		Desired LED intensity value 0 (off) to 10 (full on) in 10% steps
// 	/proc/BACKLIGHT_CURRENT
// 		Current LED intensity value 0 (off) to 10 (full on) in 10% steps
// 	/proc/FADE_TIME
// 		Number of system ticks per step when fading from one LED intensity
// 		to the next.  To disable fading set BACKLIGHT_MODE to 0 and
// 		set BACKLIGHT_CURRENT to the desired level.
// 
// Set PWM_SERIAL_PORT to the address of the LED's serial port.
// 
// ttyS0: 0xb0000500
// ttyS1: 0xb0000c00

//#define PWM_SERIAL_PORT	0xb0000c00

#endif	// _PRODUCT_GPIO_CFG_H_

