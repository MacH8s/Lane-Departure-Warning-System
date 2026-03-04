#include "GPIOHandler.h"

const char* GPIOHandler::GPIO_CHIP_PATH = "/dev/gpiochip0";

// Default constructor
GPIOHandler::GPIOHandler()
{
	gpioReady = false;
	chip = NULL;
	inputRequest = NULL;
	outputRequest = NULL;
}

// Destructor
GPIOHandler::~GPIOHandler()
{
	cleanup();
}

// Initialize GPIO pins using libgpiod 2.0+ API
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

	// Create request config (shared by input and output requests)
	struct gpiod_request_config *reqCfg = gpiod_request_config_new();
	if (!reqCfg)
	{
		cerr << "Failed to create request config. GPIO features disabled." << endl;
		gpiod_chip_close(chip);
		chip = NULL;
		gpioReady = false;
		return;
	}
	gpiod_request_config_set_consumer(reqCfg, "LDWS");

	// --- Setup input lines (pins 22 and 23) ---
	struct gpiod_line_settings *inputSettings = gpiod_line_settings_new();
	if (!inputSettings)
	{
		cerr << "Failed to create input line settings. GPIO features disabled." << endl;
		gpiod_request_config_free(reqCfg);
		gpiod_chip_close(chip);
		chip = NULL;
		gpioReady = false;
		return;
	}
	gpiod_line_settings_set_direction(inputSettings, GPIOD_LINE_DIRECTION_INPUT);

	struct gpiod_line_config *inputLineCfg = gpiod_line_config_new();
	if (!inputLineCfg)
	{
		cerr << "Failed to create input line config. GPIO features disabled." << endl;
		gpiod_line_settings_free(inputSettings);
		gpiod_request_config_free(reqCfg);
		gpiod_chip_close(chip);
		chip = NULL;
		gpioReady = false;
		return;
	}

	unsigned int inputOffsets[] = {INPUT_PIN_LEFT, INPUT_PIN_RIGHT};
	if (gpiod_line_config_add_line_settings(inputLineCfg, inputOffsets, 2, inputSettings) < 0)
	{
		cerr << "Failed to add input line settings. GPIO features disabled." << endl;
		gpiod_line_config_free(inputLineCfg);
		gpiod_line_settings_free(inputSettings);
		gpiod_request_config_free(reqCfg);
		gpiod_chip_close(chip);
		chip = NULL;
		gpioReady = false;
		return;
	}

	inputRequest = gpiod_chip_request_lines(chip, reqCfg, inputLineCfg);
	gpiod_line_config_free(inputLineCfg);
	gpiod_line_settings_free(inputSettings);

	if (!inputRequest)
	{
		cerr << "Failed to request GPIO input lines. GPIO features disabled." << endl;
		gpiod_request_config_free(reqCfg);
		gpiod_chip_close(chip);
		chip = NULL;
		gpioReady = false;
		return;
	}

	// --- Setup output lines (pins 24 and 25) ---
	struct gpiod_line_settings *outputSettings = gpiod_line_settings_new();
	if (!outputSettings)
	{
		cerr << "Failed to create output line settings. GPIO features disabled." << endl;
		gpiod_line_request_release(inputRequest);
		inputRequest = NULL;
		gpiod_request_config_free(reqCfg);
		gpiod_chip_close(chip);
		chip = NULL;
		gpioReady = false;
		return;
	}
	gpiod_line_settings_set_direction(outputSettings, GPIOD_LINE_DIRECTION_OUTPUT);
	gpiod_line_settings_set_output_value(outputSettings, GPIOD_LINE_VALUE_INACTIVE);

	struct gpiod_line_config *outputLineCfg = gpiod_line_config_new();
	if (!outputLineCfg)
	{
		cerr << "Failed to create output line config. GPIO features disabled." << endl;
		gpiod_line_settings_free(outputSettings);
		gpiod_line_request_release(inputRequest);
		inputRequest = NULL;
		gpiod_request_config_free(reqCfg);
		gpiod_chip_close(chip);
		chip = NULL;
		gpioReady = false;
		return;
	}

	unsigned int outputOffsets[] = {OUTPUT_PIN_LEFT, OUTPUT_PIN_RIGHT};
	if (gpiod_line_config_add_line_settings(outputLineCfg, outputOffsets, 2, outputSettings) < 0)
	{
		cerr << "Failed to add output line settings. GPIO features disabled." << endl;
		gpiod_line_config_free(outputLineCfg);
		gpiod_line_settings_free(outputSettings);
		gpiod_line_request_release(inputRequest);
		inputRequest = NULL;
		gpiod_request_config_free(reqCfg);
		gpiod_chip_close(chip);
		chip = NULL;
		gpioReady = false;
		return;
	}

	outputRequest = gpiod_chip_request_lines(chip, reqCfg, outputLineCfg);
	gpiod_line_config_free(outputLineCfg);
	gpiod_line_settings_free(outputSettings);
	gpiod_request_config_free(reqCfg);

	if (!outputRequest)
	{
		cerr << "Failed to request GPIO output lines. GPIO features disabled." << endl;
		gpiod_line_request_release(inputRequest);
		inputRequest = NULL;
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
		gpiod_line_request_set_value(outputRequest, OUTPUT_PIN_LEFT, GPIOD_LINE_VALUE_INACTIVE);
		gpiod_line_request_set_value(outputRequest, OUTPUT_PIN_RIGHT, GPIOD_LINE_VALUE_INACTIVE);
		gpioReady = false;
	}

	if (outputRequest)
	{
		gpiod_line_request_release(outputRequest);
		outputRequest = NULL;
	}

	if (inputRequest)
	{
		gpiod_line_request_release(inputRequest);
		inputRequest = NULL;
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
	enum gpiod_line_value val = gpiod_line_request_get_value(inputRequest, INPUT_PIN_LEFT);
	if (val == GPIOD_LINE_VALUE_ERROR)
	{
		cerr << "Failed to read GPIO pin " << INPUT_PIN_LEFT << endl;
		return false;
	}
	return val == GPIOD_LINE_VALUE_ACTIVE;
}

// Read input pin 23 (right lane external signal)
bool GPIOHandler::readPin23()
{
	if (!gpioReady) return false;
	enum gpiod_line_value val = gpiod_line_request_get_value(inputRequest, INPUT_PIN_RIGHT);
	if (val == GPIOD_LINE_VALUE_ERROR)
	{
		cerr << "Failed to read GPIO pin " << INPUT_PIN_RIGHT << endl;
		return false;
	}
	return val == GPIOD_LINE_VALUE_ACTIVE;
}

// Write output pin 24 (left departure warning output)
void GPIOHandler::writePin24(bool high)
{
	if (!gpioReady) return;
	gpiod_line_request_set_value(outputRequest, OUTPUT_PIN_LEFT,
		high ? GPIOD_LINE_VALUE_ACTIVE : GPIOD_LINE_VALUE_INACTIVE);
}

// Write output pin 25 (right departure warning output)
void GPIOHandler::writePin25(bool high)
{
	if (!gpioReady) return;
	gpiod_line_request_set_value(outputRequest, OUTPUT_PIN_RIGHT,
		high ? GPIOD_LINE_VALUE_ACTIVE : GPIOD_LINE_VALUE_INACTIVE);
}

// Check if GPIO was successfully initialized
bool GPIOHandler::isInitialized()
{
	return gpioReady;
}
