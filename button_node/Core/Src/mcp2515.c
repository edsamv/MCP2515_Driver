#include "mcp2515.h"

void MCP2515_CS_Low(MCP2515_HandleTypeDef *dev)
{
    HAL_GPIO_WritePin(dev->cs_port, dev->cs_pin, GPIO_PIN_RESET);
}

void MCP2515_CS_High(MCP2515_HandleTypeDef *dev)
{
    HAL_GPIO_WritePin(dev->cs_port, dev->cs_pin, GPIO_PIN_SET);
}

uint8_t SPI_TxRx(MCP2515_HandleTypeDef *dev, uint8_t data)
{
    uint8_t rx_data = 0;

    HAL_SPI_TransmitReceive(
        dev->hspi,
        &data,
        &rx_data,
        1,
        HAL_MAX_DELAY
    );

    return rx_data;
}

void MCP2515_Reset(MCP2515_HandleTypeDef *dev)
{
    MCP2515_CS_Low(dev);
    SPI_TxRx(dev, MCP2515_RESET);
    MCP2515_CS_High(dev);

    HAL_Delay(10);
}

uint8_t MCP2515_ReadRegister(MCP2515_HandleTypeDef *dev, uint8_t reg)
{
    uint8_t value;

    MCP2515_CS_Low(dev);

    SPI_TxRx(dev, MCP2515_READ);
    SPI_TxRx(dev, reg);

    value = SPI_TxRx(dev, 0x00);

    MCP2515_CS_High(dev);

    return value;
}

void MCP2515_WriteRegister(
    MCP2515_HandleTypeDef *dev,
    uint8_t reg,
    uint8_t value)
{
    MCP2515_CS_Low(dev);

    SPI_TxRx(dev, MCP2515_WRITE);
    SPI_TxRx(dev, reg);
    SPI_TxRx(dev, value);

    MCP2515_CS_High(dev);
}

void MCP2515_BitModify(
    MCP2515_HandleTypeDef *dev,
    uint8_t reg,
    uint8_t mask,
    uint8_t data)
{
    MCP2515_CS_Low(dev);

    SPI_TxRx(dev, MCP2515_BIT_MODIFY);
    SPI_TxRx(dev, reg);
    SPI_TxRx(dev, mask);
    SPI_TxRx(dev, data);

    MCP2515_CS_High(dev);
}

uint8_t MCP2515_ReadStatus(MCP2515_HandleTypeDef *dev)
{
    uint8_t status;

    MCP2515_CS_Low(dev);

    SPI_TxRx(dev, MCP2515_READ_STATUS);
    status = SPI_TxRx(dev, 0x00);

    MCP2515_CS_High(dev);

    return status;
}

void MCP2515_SetMode(MCP2515_HandleTypeDef *dev, uint8_t mode)
{
    MCP2515_BitModify(
        dev,
        MCP2515_CANCTRL,
        0xE0,
        mode
    );

    HAL_Delay(10);
}

uint8_t MCP2515_Init(
    MCP2515_HandleTypeDef *dev,
    uint8_t cnf1,
    uint8_t cnf2,
    uint8_t cnf3)
{
    // Reset MCP2515
    MCP2515_CS_High(dev);
    MCP2515_Reset(dev);

    // Check config mode
    if ((MCP2515_ReadRegister(dev, MCP2515_CANSTAT) & 0xE0)
        != MCP2515_MODE_CONFIG)
    {
        return 0;
    }

    // Set CAN speed
    MCP2515_WriteRegister(dev, MCP2515_CNF1, cnf1);
    MCP2515_WriteRegister(dev, MCP2515_CNF2, cnf2);
    MCP2515_WriteRegister(dev, MCP2515_CNF3, cnf3);

    // Accept all messages
    MCP2515_WriteRegister(dev, MCP2515_RXB0CTRL, 0x60);

    // Clear interrupt flags
    MCP2515_WriteRegister(dev, MCP2515_CANINTF, 0x00);

    // Normal mode
    MCP2515_SetMode(dev, MCP2515_MODE_NORMAL);

    // Check normal mode
    if ((MCP2515_ReadRegister(dev, MCP2515_CANSTAT) & 0xE0)
        != MCP2515_MODE_NORMAL)
    {
        return 0;
    }

    return 1;
}

void MCP2515_LoadTXBuffer(
    MCP2515_HandleTypeDef *dev,
    const CAN_Frame *frame)
{
    uint8_t sidh;
    uint8_t sidl;

    // Convert 11-bit CAN ID
    sidh = (uint8_t)(frame->id >> 3);
    sidl = (uint8_t)((frame->id & 0x07) << 5);

    MCP2515_CS_Low(dev);

    // Load TX buffer 0
    SPI_TxRx(dev, MCP2515_LOAD_TX0);

    // CAN ID
    SPI_TxRx(dev, sidh);
    SPI_TxRx(dev, sidl);

    // Extended ID
    SPI_TxRx(dev, 0x00);
    SPI_TxRx(dev, 0x00);

    // DLC
    SPI_TxRx(dev, frame->dlc & 0x0F);

    // Data
    for (uint8_t i = 0; i < frame->dlc; i++)
    {
        SPI_TxRx(dev, frame->data[i]);
    }

    MCP2515_CS_High(dev);
}

void MCP2515_RequestToSend(MCP2515_HandleTypeDef *dev)
{
    MCP2515_CS_Low(dev);

    SPI_TxRx(dev, MCP2515_RTS_TX0);

    MCP2515_CS_High(dev);
}

void MCP2515_ReadRXBuffer(
    MCP2515_HandleTypeDef *dev,
    CAN_Frame *frame)
{
    uint8_t sidh;
    uint8_t sidl;

    MCP2515_CS_Low(dev);

    // Read RX buffer 0
    SPI_TxRx(dev, MCP2515_READ_RX0);

    // Read CAN ID
    sidh = SPI_TxRx(dev, 0x00);
    sidl = SPI_TxRx(dev, 0x00);

    // Skip extended ID
    SPI_TxRx(dev, 0x00);
    SPI_TxRx(dev, 0x00);

    // Read DLC
    frame->dlc = SPI_TxRx(dev, 0x00) & 0x0F;

    // Convert back to 11-bit CAN ID
    frame->id = ((uint32_t)sidh << 3) |
                ((uint32_t)sidl >> 5);

    // Read data
    for (uint8_t i = 0; i < frame->dlc && i < 8; i++)
    {
        frame->data[i] = SPI_TxRx(dev, 0x00);
    }

    MCP2515_CS_High(dev);
}

uint8_t MCP2515_GetInterruptFlags(MCP2515_HandleTypeDef *dev)
{
    return MCP2515_ReadRegister(dev, MCP2515_CANINTF);
}

void MCP2515_ClearInterruptFlags(
    MCP2515_HandleTypeDef *dev,
    uint8_t mask)
{
    MCP2515_BitModify(
        dev,
        MCP2515_CANINTF,
        mask,
        0x00
    );
}
