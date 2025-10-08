#ifndef ARDUINO_INPUT_PROVIDER_H
#define ARDUINO_INPUT_PROVIDER_H

#include "Input.h"
#include <Arduino.h>

class ArduinoInputProvider final : public IInputProvider
{
    // Analog pins for X and Y axes, digital pin for button
    const uint8_t pinX;
    const uint8_t pinY;
    const uint8_t pinBtn;
    bool initialized = false;

public:
    ArduinoInputProvider(uint8_t x, uint8_t y, uint8_t b, const InputCalibration &c = {})
        : IInputProvider(c), pinX(x), pinY(y), pinBtn(b) {}

    bool init()
    {
        if (!initialized)
        {
            pinMode(pinX, INPUT);
            pinMode(pinY, INPUT);
            pinMode(pinBtn, INPUT_PULLUP); // Button active low
            initialized = true;
        }
        return initialized;
    }

    void sample(InputState &state) override
    {
        state.x_adc = static_cast<AnalogInput_t>(analogRead(pinX));
        state.y_adc = static_cast<AnalogInput_t>(analogRead(pinY));
        state.pressed = (digitalRead(pinBtn) == LOW); // Active low
    }
};

#endif // ARDUINO_INPUT_PROVIDER_H
