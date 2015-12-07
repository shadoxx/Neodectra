/******************************************************************************
* Neodectra - Unofficial compatible firmware for use with Audectra [http://www.audectra.com/]
*
* Firmware written by Brandon Wooldridge [brandon+neodectra@hive13.org]
* Audectra written by A1i3n37 [support@audectra.com]
*
* This program is provided as is. People with epilepsy should exercise extreme
*   caution when using this firmware. All rights to Audectra are retained by
*   A1i3n37 and his team. Neodectra is an independent project, and does not
*   represent or claim to represent Audectra in any way, shape, or form.
*
* FILE: config.h
* DESC: All (or most) of our configuration options packed in a neat little file
*
******************************************************************************/

// ** CONFIG: MAIN ************************************************************
#define CURRENT_EFFECT          4
#define AUDECTRA_VERSION        1
#define POTPIN                  A0
#define POT_READ                0
#define BRIGHTNESS              42

// ** CONFIG: MICROCONTROLLER *************************************************
#define	DATAPIN			6               // pin the LED data-in line is connected to on the Arduino
#define	BAUDRATE		9600            // baud rate for serial/COM communication, usually 9600

// ** CONFIG: FASTSPI *********************************************************
#define	CHIPSET			WS2812	        // chipset we're planning on controlling (read FastSPI documentation)
#define	PIXEL_ORDER		GRB		// led color pixel order, discovered by running FastSPI calibration sketch
#define	STRIP_LENGTH		512		// the number of LEDs we'll be controlling
#define STRIP_CENTER	        STRIP_LENGTH / 2

// ** CONFIG: NETWORK *********************************************************
//byte macAddr[] = { 0xA0, 0xAD, 0x78, 0x00, 0x00, 0x01 };  // MAC address, doesn't matter as long as it's unique
//byte  ipAddr[] = { 172, 16, 2, 148 };  // ip address

// ** CONFIG: FIRMWARE ID *****************************************************
//const char* ID = "DIYAudectra";
//const char* FirmwareVersion = "1.1.0";

// ** CONFIG: EFFECTS *********************************************************

// FADE
#define	FADE_DELAY		20
#define	FADE_PERCENT	        10

// LFO (Audectra Settings: Bass[90,2] - Mid[70,18] - High[34,32])
#define LFO_RATE		10
#define	SAMPLERATE		8	    // how often we poll for data, in milliseconds

// VU METER (Audectra Settings: Bass[85,8] - Mid[65,16] - High[35,32])
#define	VU_DELAY			20  // in milliseconds
#define	MASTER_GAIN			15
#define	HI_GAIN				5
#define MID_GAIN			5
#define LOW_GAIN			5
#define MAX_VOLUME_RANGE	(((255 * HI_GAIN) + (255 * MID_GAIN) + (255 * LOW_GAIN)) /3) * MASTER_GAIN
