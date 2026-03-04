#ifndef __GPIOHANDLER_H__
#define __GPIOHANDLER_H__

#include <gpiod.h>
#include <iostream>

using namespace std;

/*
 * Project:  Lane Departure Warning System (LDWS)
 *
 * File:     GPIOHandler.h
 *
 * Contents: The GPIOHandler class manages Raspberry Pi GPIO pins for
 *           external input/output signaling in the LDWS.
 *           Uses libgpiod for GPIO access on modern Raspberry Pi kernels.
 *           - Reads pins 22 (left) and 23 (right) as inputs from an external source.
 *           - Writes to pins 24 (left departure) and 25 (right departure) as outputs.
 *           HIGH = 3300mV, LOW = 0mV (standard Raspberry Pi GPIO).
 */

class GPIOHandler
{
	public:
		// Default constructor
		GPIOHandler();

		// Destructor - cleans up GPIO resources
		~GPIOHandler();

		// Initialize GPIO pins
		void initialize();

		// Cleanup GPIO resources
		void cleanup();

		// Read input pin 22 (left lane external signal)
		bool readPin22();

		// Read input pin 23 (right lane external signal)
		bool readPin23();

		// Write output pin 24 (left departure warning output)
		void writePin24(bool high);

		// Write output pin 25 (right departure warning output)
		void writePin25(bool high);

		// Check if GPIO was successfully initialized
		bool isInitialized();

	private:
		bool gpioReady;

		static const int INPUT_PIN_LEFT = 22;
		static const int INPUT_PIN_RIGHT = 23;
		static const int OUTPUT_PIN_LEFT = 24;
		static const int OUTPUT_PIN_RIGHT = 25;

		static const char* GPIO_CHIP_PATH;

		struct gpiod_chip *chip;
		struct gpiod_line *lineInputLeft;
		struct gpiod_line *lineInputRight;
		struct gpiod_line *lineOutputLeft;
		struct gpiod_line *lineOutputRight;
};

#endif
