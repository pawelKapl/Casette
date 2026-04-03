#pragma once

#include "env.h"
#include <gpiod.h>

class GPIOButton
{
public:
    GPIOButton(gpiod_chip *chip, int pinNumber) : _pinNumber(pinNumber)
    {
        _gpioLine = gpiod_chip_get_line(chip, pinNumber);
        if (!_gpioLine) 
        {
            gpiod_chip_close(chip);
            log_error << "Failed to get GPIO line\n";
            throw std::runtime_error("Failed to get GPIO line");
        }

        std::string portName = "dpdt-button" + std::to_string(pinNumber);

        if (gpiod_line_request_input(_gpioLine, portName.c_str()) < 0) 
        {
            log_error << "Failed to request line " << pinNumber << " as input\n";
            gpiod_chip_close(chip);
            throw std::runtime_error("Failed to request line as input");
        }
    }

    void setShortPressCallback(std::function<void()> callback)
    {
        _shortPressCallback = callback;
    }

    void setLongPressCallback(std::function<void()> callback)
    {
        _longPressCallback = callback;
    }

    void debounce(bool longCallbackOnly = false)
    {
        int value = gpiod_line_get_value(_gpioLine);
        if (value == 0)
        {
            _debounceCounter++;
        }
        else
        {
            if (_debounceCounter > 4 && _debounceCounter < 220)
            {
                if (_shortPressCallback && !longCallbackOnly)
                    _shortPressCallback();
                log_info << "Pressed shorter button on line: " + std::to_string(_pinNumber);
            }
            _debounceCounter = 0;
        }

        if (_debounceCounter == 220)
        {
            if (_longPressCallback)
                _longPressCallback();
            log_info << "Pressed longer button on line: " + std::to_string(_pinNumber);
        }
    }

private:
    gpiod_line *_gpioLine;
    int _pinNumber;
    std::function<void()> _shortPressCallback;
    std::function<void()> _longPressCallback;

    int _debounceCounter = 0;


}; // GPIOButton