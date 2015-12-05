===================================
|       N E O  D E C T R A        |
|  Audectra Compatible Firmware   |
|      @shadoxx on Github         |
===================================

:INTRODUCTION
	Neodectra is a micro-controller firmware intended for use with Audectra, a
	Windows based audio analyzation suite. It uses the serial data provided by
	Audectra to drive an arbitrary (within reason) amount of LEDs using the
	FastLED library written for the Teensy USB Development Board.
  
	A full list of all supported chipsets can be found at the following URL:
	https://github.com/FastLED/FastLED/wiki/Chipset-reference

:HARDWARE
	+ Teensy 3.0 or later
	store: https://www.pjrc.com/store/teensy32.html
	notes: I have only tested this firmware with the Teensy 3.0 so YMMV.
	
	+ RGB123 16x16 WS2812 LED Panel (Optional), or supported LEDs
	store: http://rgb-123.com/product/1616-16-x-16-rgb-led-matrix/
	notes: I have tested this firmware with 2 of the 16x16 panels.
	
	+ Powersupply
	notes: Running two 16x16 WS2818 LED panels at full brightness requires at
	       least 8A @ 5V. I hacked an old computer PSU to drive my panels, but
		   I would recommend going with something more conventional. RGB123
		   sells a great 5V PSU.
		   
	+ Computer running Windows 7 or later (tested on Windows 7)
  
: SOFTWARE

	
		  
:CONFIGURATION
I will document all of the configuration options...eventually. For now, this is
the bare minimum needed to get you up and running.

	1. Install the FastLED library and configure your IDE if necessary.
	   https://github.com/FastLED/FastLED/releases/tag/v3.1.0

	2. Once you have the LEDs connected to your Teensy and you've verified
	   that you're able to run one of the FastLED demo sketches, open up this
	   project in your IDE.
	   
	3. 'config.h' contains four important constants you need to configure before
	    Neodectra will properly work:
		
		DATAPIN			The PIN the Teensy uses to communicate with your LEDs.
						You should know what this is if you completed step 2.
						Clocked chipsets are unsupported at the moment.		
		
		CHIPSET			Supported chipsets can be found at the following link:
						https://github.com/FastLED/FastLED/wiki/Chipset-reference
		
		PIXEL_ORDER		'GRB' for the WS2812s.
				
		STRIP_LENGTH	The number of LEDs you intend to drive with Neodectra.
		
		
	