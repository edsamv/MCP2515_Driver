#include "Motor.hpp"

Motor::Motor(uint16_t canId)
{
    m_canId = canId;
    m_enabled = false;
    m_targetCurrent = 0;
    m_actualRpm = 0;
    m_temperatureC = 25;
    m_errorCode = 0;
}

void Motor::enable()
{
    m_enabled = true;
}

void Motor::disable()
{
    m_enabled = false;
    m_targetCurrent = 0;
    m_actualRpm = 0;
    m_temperatureC = 25;
    m_errorCode = 0;
}

void Motor::setTargetCurrent(int16_t current)
{
    m_targetCurrent = current;
}

void Motor::setActualRpm(int16_t rpm)
{
    m_actualRpm = rpm;
}

void Motor::setTemperature(uint8_t tempC)
{
    m_temperatureC = tempC;
}

void Motor::setErrorCode(uint8_t errorCode)
{
    m_errorCode = errorCode;
}

uint16_t Motor::getCanId() const
{
    return m_canId;
}

bool Motor::isEnabled() const
{
    return m_enabled;
}

int16_t Motor::getTargetCurrent() const
{
    return m_targetCurrent;
}

int16_t Motor::getActualRpm() const
{
    return m_actualRpm;
}

uint8_t Motor::getTemperature() const
{
    return m_temperatureC;
}

uint8_t Motor::getErrorCode() const
{
    return m_errorCode;
}

// Pack motor data into CAN frame
// 0: Enable
// 1-2: Current
// 3-4: RPM
// 5: Temperature
// 6: Error
// 7: Reserved
void Motor::pack(uint8_t *buffer) const
{
    if (buffer == 0)
        return;

    buffer[0] = m_enabled ? 0x01 : 0x00;

    buffer[1] = (uint8_t)(m_targetCurrent >> 8);
    buffer[2] = (uint8_t)(m_targetCurrent & 0xFF);

    buffer[3] = (uint8_t)(m_actualRpm >> 8);
    buffer[4] = (uint8_t)(m_actualRpm & 0xFF);

    buffer[5] = m_temperatureC;
    buffer[6] = m_errorCode;
    buffer[7] = 0x00;
}

void Motor::unpack(const uint8_t *buffer)
{
    if (buffer == 0)
        return;

    m_enabled = (buffer[0] == 0x01);

    m_targetCurrent =
        (int16_t)(((uint16_t)buffer[1] << 8) | buffer[2]);

    m_actualRpm =
        (int16_t)(((uint16_t)buffer[3] << 8) | buffer[4]);

    m_temperatureC = buffer[5];
    m_errorCode = buffer[6];
}
