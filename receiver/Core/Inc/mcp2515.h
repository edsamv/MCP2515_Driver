#ifndef MCP2515_H
#define MCP2515_H

#include "main.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// SPI commands
#define MCP2515_RESET          0xC0
#define MCP2515_READ           0x03
#define MCP2515_WRITE          0x02
#define MCP2515_BIT_MODIFY     0x05
#define MCP2515_LOAD_TX0       0x40
#define MCP2515_RTS_TX0        0x81
#define MCP2515_READ_RX0       0x90
#define MCP2515_READ_STATUS    0xA0
#define MCP2515_RX_STATUS      0xB0

// Registers
#define MCP2515_CANSTAT        0x0E
#define MCP2515_CANCTRL        0x0F
#define MCP2515_CNF3           0x28
#define MCP2515_CNF2           0x29
#define MCP2515_CNF1           0x2A
#define MCP2515_CANINTE        0x2B
#define MCP2515_CANINTF        0x2C
#define MCP2515_TXB0CTRL       0x30
#define MCP2515_RXB0CTRL       0x60

// Interrupt flags
#define MCP2515_RX0IF          0x01
#define MCP2515_RX1IF          0x02
#define MCP2515_TX0IF          0x04

// Operating modes
#define MCP2515_MODE_NORMAL      0x00
#define MCP2515_MODE_SLEEP       0x20
#define MCP2515_MODE_LOOPBACK    0x40
#define MCP2515_MODE_LISTENONLY  0x60
#define MCP2515_MODE_CONFIG      0x80

// 8 MHz crystal, 500 kbps
#define MCP2515_8MHZ_500KBPS_CNF1    0x00
#define MCP2515_8MHZ_500KBPS_CNF2    0x90
#define MCP2515_8MHZ_500KBPS_CNF3    0x02

typedef struct
{
    uint32_t id;
    uint8_t dlc;
    uint8_t data[8];
} CAN_Frame;

typedef struct
{
    SPI_HandleTypeDef *hspi;
    GPIO_TypeDef *cs_port;
    uint16_t cs_pin;
} MCP2515_HandleTypeDef;

// SPI functions
void MCP2515_CS_Low(MCP2515_HandleTypeDef *dev);
void MCP2515_CS_High(MCP2515_HandleTypeDef *dev);
uint8_t SPI_TxRx(MCP2515_HandleTypeDef *dev, uint8_t data);

// Register functions
void MCP2515_Reset(MCP2515_HandleTypeDef *dev);

uint8_t MCP2515_ReadRegister(
    MCP2515_HandleTypeDef *dev,
    uint8_t reg
);

void MCP2515_WriteRegister(
    MCP2515_HandleTypeDef *dev,
    uint8_t reg,
    uint8_t value
);

void MCP2515_BitModify(
    MCP2515_HandleTypeDef *dev,
    uint8_t reg,
    uint8_t mask,
    uint8_t data
);

uint8_t MCP2515_ReadStatus(
    MCP2515_HandleTypeDef *dev
);

// CAN functions
uint8_t MCP2515_Init(
    MCP2515_HandleTypeDef *dev,
    uint8_t cnf1,
    uint8_t cnf2,
    uint8_t cnf3
);

void MCP2515_SetMode(
    MCP2515_HandleTypeDef *dev,
    uint8_t mode
);

// Transmission
void MCP2515_LoadTXBuffer(
    MCP2515_HandleTypeDef *dev,
    const CAN_Frame *frame
);

void MCP2515_RequestToSend(
    MCP2515_HandleTypeDef *dev
);

// Reception
void MCP2515_ReadRXBuffer(
    MCP2515_HandleTypeDef *dev,
    CAN_Frame *frame
);

uint8_t MCP2515_GetInterruptFlags(
    MCP2515_HandleTypeDef *dev
);

void MCP2515_ClearInterruptFlags(
    MCP2515_HandleTypeDef *dev,
    uint8_t mask
);

#ifdef __cplusplus
}
#endif

#endif
