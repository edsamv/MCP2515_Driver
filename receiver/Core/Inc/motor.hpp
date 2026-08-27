#ifndef MOTOR_HPP
#define MOTOR_HPP

#include <stdint.h>

class Motor
{
private:
    uint16_t m_canId;
    bool m_enabled;

    int16_t m_targetCurrent;
    int16_t m_actualRpm;

    uint8_t m_temperatureC;
    uint8_t m_errorCode;

public:
    Motor(uint16_t canId);

    // Motor control
    void enable();
    void disable();

    // Set motor values
    void setTargetCurrent(int16_t current);
    void setActualRpm(int16_t rpm);
    void setTemperature(uint8_t tempC);
    void setErrorCode(uint8_t errorCode);

    // Get motor values
    uint16_t getCanId() const;
    bool isEnabled() const;
    int16_t getTargetCurrent() const;
    int16_t getActualRpm() const;
    uint8_t getTemperature() const;
    uint8_t getErrorCode() const;

    // CAN data
    void pack(uint8_t *buffer) const;
    void unpack(const uint8_t *buffer);
};

#endif
