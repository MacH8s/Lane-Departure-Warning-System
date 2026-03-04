#include "GPIOHandler.h"

const char* GPIOHandler::GPIO_CHIP_PATH = "/dev/gpiochip0";

// Default constructor
GPIOHandler::GPIOHandler()
{
	gpioReady = false;
	chip = NULL;
	lineInputLeft = NULL;
	lineInputRight = NULL;
	lineOutputLeft = NULL;
	lineOutputRight = NULL;
}

// Destructor
GPIOHandler::~GPIOHandler()
{
	cleanup();
}

// Initialize GPIO pins
void GPIOHandler::initialize()
{
	chip = gpiod_chip_open(GPIO_CHIP_PATH);
	if (!chip)
	{
		cerr << "GPIO initialization failed. GPIO features disabled." << endl;
		cerr << "Could not open " << GPIO_CHIP_PATH << ". Ensure /dev/gpiochip0 exists." << endl;
		gpioReady = false;
		return;
	}

	// Get GPIO lines
	lineInputLeft = gpiod_chip_get_line(chip, INPUT_PIN_LEFT);
	lineInputRight = gpiod_chip_get_line(chip, INPUT_PIN_RIGHT);
	lineOutputLeft = gpiod_chip_get_line(chip, OUTPUT_PIN_LEFT);
	lineOutputRight = gpiod_chip_get_line(chip, OUTPUT_PIN_RIGHT);

	if (!lineInputLeft || !lineInputRight || !lineOutputLeft || !lineOutputRight)
	{
		cerr << "Failed to get GPIO lines. GPIO features disabled." << endl;
		gpiod_chip_close(chip);
		chip = NULL;
		gpioReady = false;
		return;
	}

	// Request input lines
	if (gpiod_line_request_input(lineInputLeft, "LDWS") < 0 ||
		gpiod_line_request_input(lineInputRight, "LDWS") < 0)
	{
		cerr << "Failed to request GPIO input lines. GPIO features disabled." << endl;
		gpiod_chip_close(chip);
		chip = NULL;
		gpioReady = false;
		return;
	}

	// Request output lines with initial value LOW
	if (gpiod_line_request_output(lineOutputLeft, "LDWS", 0) < 0 ||
		gpiod_line_request_output(lineOutputRight, "LDWS", 0) < 0)
	{
		cerr << "Failed to request GPIO output lines. GPIO features disabled." << endl;
		gpiod_chip_close(chip);
		chip = NULL;
		gpioReady = false;
		return;
	}

	gpioReady = true;
}

// Cleanup GPIO resources
void GPIOHandler::cleanup()
{
	if (gpioReady)
	{
		gpiod_line_set_value(lineOutputLeft, 0);
		gpiod_line_set_value(lineOutputRight, 0);
		gpioReady = false;
	}

	if (chip)
	{
		gpiod_chip_close(chip);
		chip = NULL;
	}
}

// Read input pin 22 (left lane external signal)
bool GPIOHandler::readPin22()
{
	if (!gpioReady) return false;
	return gpiod_line_get_value(lineInputLeft) == 1;
}

// Read input pin 23 (right lane external signal)
bool GPIOHandler::readPin23()
{
	if (!gpioReady) return false;
	return gpiod_line_get_value(lineInputRight) == 1;
}

// Write output pin 24 (left departure warning output)
void GPIOHandler::writePin24(bool high)
{
	if (!gpioReady) return;
	gpiod_line_set_value(lineOutputLeft, high ? 1 : 0);
}

// Write output pin 25 (right departure warning output)
void GPIOHandler::writePin25(bool high)
{
	if (!gpioReady) return;
	gpiod_line_set_value(lineOutputRight, high ? 1 : 0);
}

// Check if GPIO was successfully initialized
bool GPIOHandler::isInitialized()
{
	return gpioReady;
}
