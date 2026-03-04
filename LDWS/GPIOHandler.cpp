#include "GPIOHandler.h"

// Default constructor
GPIOHandler::GPIOHandler()
{
	gpioReady = false;
}

// Destructor
GPIOHandler::~GPIOHandler()
{
	cleanup();
}

// Initialize GPIO pins
void GPIOHandler::initialize()
{
	if (gpioInitialise() < 0)
	{
		cerr << "GPIO initialization failed. GPIO features disabled." << endl;
		cerr << "Ensure pigpio daemon is running (sudo pigpiod) or run with sudo." << endl;
		gpioReady = false;
		return;
	}

	// Set pin modes
	gpioSetMode(INPUT_PIN_LEFT, PI_INPUT);
	gpioSetMode(INPUT_PIN_RIGHT, PI_INPUT);
	gpioSetMode(OUTPUT_PIN_LEFT, PI_OUTPUT);
	gpioSetMode(OUTPUT_PIN_RIGHT, PI_OUTPUT);

	// Initialize outputs to LOW
	gpioWrite(OUTPUT_PIN_LEFT, 0);
	gpioWrite(OUTPUT_PIN_RIGHT, 0);

	gpioReady = true;
}

// Cleanup GPIO resources
void GPIOHandler::cleanup()
{
	if (gpioReady)
	{
		gpioWrite(OUTPUT_PIN_LEFT, 0);
		gpioWrite(OUTPUT_PIN_RIGHT, 0);
		gpioTerminate();
		gpioReady = false;
	}
}

// Read input pin 22 (left lane external signal)
bool GPIOHandler::readPin22()
{
	if (!gpioReady) return false;
	return gpioRead(INPUT_PIN_LEFT) == 1;
}

// Read input pin 23 (right lane external signal)
bool GPIOHandler::readPin23()
{
	if (!gpioReady) return false;
	return gpioRead(INPUT_PIN_RIGHT) == 1;
}

// Write output pin 24 (left departure warning output)
void GPIOHandler::writePin24(bool high)
{
	if (!gpioReady) return;
	gpioWrite(OUTPUT_PIN_LEFT, high ? 1 : 0);
}

// Write output pin 25 (right departure warning output)
void GPIOHandler::writePin25(bool high)
{
	if (!gpioReady) return;
	gpioWrite(OUTPUT_PIN_RIGHT, high ? 1 : 0);
}

// Check if GPIO was successfully initialized
bool GPIOHandler::isInitialized()
{
	return gpioReady;
}
