#include "GPIOHandler.h"

#include <linux/gpio.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

const char* GPIOHandler::GPIO_CHIP_PATH = "/dev/gpiochip0";

// Default constructor
GPIOHandler::GPIOHandler()
{
	gpioReady = false;
	chipFd = -1;
	inputFd = -1;
	outputFd = -1;
}

// Destructor
GPIOHandler::~GPIOHandler()
{
	cleanup();
}

// Initialize GPIO pins using Linux kernel GPIO character device ioctl interface
void GPIOHandler::initialize()
{
	chipFd = open(GPIO_CHIP_PATH, O_RDONLY);
	if (chipFd < 0)
	{
		cerr << "GPIO initialization failed. GPIO features disabled." << endl;
		cerr << "Could not open " << GPIO_CHIP_PATH << ". Ensure /dev/gpiochip0 exists." << endl;
		gpioReady = false;
		return;
	}

	// --- Setup input lines (pins 22 and 23) ---
	struct gpiohandle_request inputReq;
	memset(&inputReq, 0, sizeof(inputReq));
	inputReq.lineoffsets[0] = INPUT_PIN_LEFT;
	inputReq.lineoffsets[1] = INPUT_PIN_RIGHT;
	inputReq.flags = GPIOHANDLE_REQUEST_INPUT;
	inputReq.lines = 2;
	strncpy(inputReq.consumer_label, "LDWS", sizeof(inputReq.consumer_label) - 1);

	if (ioctl(chipFd, GPIO_GET_LINEHANDLE_IOCTL, &inputReq) < 0)
	{
		cerr << "Failed to request GPIO input lines. GPIO features disabled." << endl;
		close(chipFd);
		chipFd = -1;
		gpioReady = false;
		return;
	}
	inputFd = inputReq.fd;

	// --- Setup output lines (pins 24 and 25) ---
	struct gpiohandle_request outputReq;
	memset(&outputReq, 0, sizeof(outputReq));
	outputReq.lineoffsets[0] = OUTPUT_PIN_LEFT;
	outputReq.lineoffsets[1] = OUTPUT_PIN_RIGHT;
	outputReq.flags = GPIOHANDLE_REQUEST_OUTPUT;
	outputReq.default_values[0] = 0;
	outputReq.default_values[1] = 0;
	outputReq.lines = 2;
	strncpy(outputReq.consumer_label, "LDWS", sizeof(outputReq.consumer_label) - 1);

	if (ioctl(chipFd, GPIO_GET_LINEHANDLE_IOCTL, &outputReq) < 0)
	{
		cerr << "Failed to request GPIO output lines. GPIO features disabled." << endl;
		close(inputFd);
		inputFd = -1;
		close(chipFd);
		chipFd = -1;
		gpioReady = false;
		return;
	}
	outputFd = outputReq.fd;

	gpioReady = true;
}

// Cleanup GPIO resources
void GPIOHandler::cleanup()
{
	if (gpioReady)
	{
		// Set outputs LOW before releasing
		struct gpiohandle_data data;
		memset(&data, 0, sizeof(data));
		data.values[0] = 0;
		data.values[1] = 0;
		ioctl(outputFd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data);
		gpioReady = false;
	}

	if (outputFd >= 0)
	{
		close(outputFd);
		outputFd = -1;
	}

	if (inputFd >= 0)
	{
		close(inputFd);
		inputFd = -1;
	}

	if (chipFd >= 0)
	{
		close(chipFd);
		chipFd = -1;
	}
}

// Read input pin 22 (left lane external signal)
bool GPIOHandler::readPin22()
{
	if (!gpioReady) return false;
	struct gpiohandle_data data;
	memset(&data, 0, sizeof(data));
	if (ioctl(inputFd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &data) < 0)
	{
		cerr << "Failed to read GPIO pin " << INPUT_PIN_LEFT << endl;
		return false;
	}
	return data.values[0] != 0;
}

// Read input pin 23 (right lane external signal)
bool GPIOHandler::readPin23()
{
	if (!gpioReady) return false;
	struct gpiohandle_data data;
	memset(&data, 0, sizeof(data));
	if (ioctl(inputFd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &data) < 0)
	{
		cerr << "Failed to read GPIO pin " << INPUT_PIN_RIGHT << endl;
		return false;
	}
	return data.values[1] != 0;
}

// Write output pin 24 (left departure warning output)
void GPIOHandler::writePin24(bool high)
{
	if (!gpioReady) return;
	struct gpiohandle_data data;
	memset(&data, 0, sizeof(data));
	// Read current output state to preserve pin 25's value
	if (ioctl(outputFd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &data) < 0)
	{
		cerr << "Failed to read GPIO output state for pin " << OUTPUT_PIN_LEFT << endl;
		return;
	}
	data.values[0] = high ? 1 : 0;
	if (ioctl(outputFd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data) < 0)
	{
		cerr << "Failed to write GPIO pin " << OUTPUT_PIN_LEFT << endl;
	}
}

// Write output pin 25 (right departure warning output)
void GPIOHandler::writePin25(bool high)
{
	if (!gpioReady) return;
	struct gpiohandle_data data;
	memset(&data, 0, sizeof(data));
	// Read current output state to preserve pin 24's value
	if (ioctl(outputFd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &data) < 0)
	{
		cerr << "Failed to read GPIO output state for pin " << OUTPUT_PIN_RIGHT << endl;
		return;
	}
	data.values[1] = high ? 1 : 0;
	if (ioctl(outputFd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data) < 0)
	{
		cerr << "Failed to write GPIO pin " << OUTPUT_PIN_RIGHT << endl;
	}
}

// Check if GPIO was successfully initialized
bool GPIOHandler::isInitialized()
{
	return gpioReady;
}
