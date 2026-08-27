extern "C" {
#include "main.h"
#include "mcp2515.h"
}

#include "Motor.hpp"

SPI_HandleTypeDef hspi1;
MCP2515_HandleTypeDef hcan1;

Motor motorFL(0x201);

uint8_t last_button_state = GPIO_PIN_RESET;
bool motor_toggle = false;

extern "C" void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    MX_GPIO_Init();
    MX_SPI1_Init();

    HAL_Delay(100);

    // MCP2515 setup
    hcan1.hspi = &hspi1;
    hcan1.cs_port = GPIOB;
    hcan1.cs_pin = GPIO_PIN_0;

    // Start CAN at 500 kbps
    if (!MCP2515_Init(&hcan1,
                     MCP2515_8MHZ_500KBPS_CNF1,
                     MCP2515_8MHZ_500KBPS_CNF2,
                     MCP2515_8MHZ_500KBPS_CNF3))
    {
        while (1)
        {
            HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_1);
            HAL_Delay(50);
        }
    }

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);

    last_button_state = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1);

    while (1)
    {
        uint8_t button = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1);

        // Button pressed
        if (button == GPIO_PIN_SET &&
            last_button_state == GPIO_PIN_RESET)
        {
            HAL_Delay(50);

            if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) == GPIO_PIN_SET)
            {
                motor_toggle = !motor_toggle;

                if (motor_toggle)
                {
                    motorFL.enable();
                    motorFL.setTargetCurrent(4500);
                    motorFL.setActualRpm(3180);
                    motorFL.setTemperature(42);
                    motorFL.setErrorCode(0x00);
                }
                else
                {
                    motorFL.disable();
                }

                // Create CAN frame
                CAN_Frame tx_frame;
                tx_frame.id = motorFL.getCanId();
                tx_frame.dlc = 8;

                motorFL.pack(tx_frame.data);

                // Send CAN frame
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);

                MCP2515_WriteRegister(&hcan1, 0x30, 0x00);
                MCP2515_ClearInterruptFlags(&hcan1, 0x04);
                MCP2515_LoadTXBuffer(&hcan1, &tx_frame);
                MCP2515_RequestToSend(&hcan1);

                HAL_Delay(100);

                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
            }
        }

        last_button_state = button;
        HAL_Delay(10);
    }
}

static void MX_SPI1_Init(void)
{
    hspi1.Instance = SPI1;
    hspi1.Init.Mode = SPI_MODE_MASTER;
    hspi1.Init.Direction = SPI_DIRECTION_2LINES;
    hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi1.Init.NSS = SPI_NSS_SOFT;
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi1.Init.CRCPolynomial = 10;

    if (HAL_SPI_Init(&hspi1) != HAL_OK)
        Error_Handler();
}

static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    // LED
    GPIO_InitStruct.Pin = GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // Button
    GPIO_InitStruct.Pin = GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // MCP2515 CS
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
}

extern "C" void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 25;
    RCC_OscInitStruct.PLL.PLLN = 168;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 4;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
        Error_Handler();

    RCC_ClkInitStruct.ClockType =
        RCC_CLOCKTYPE_HCLK |
        RCC_CLOCKTYPE_SYSCLK |
        RCC_CLOCKTYPE_PCLK1 |
        RCC_CLOCKTYPE_PCLK2;

    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
        Error_Handler();
}

extern "C" void Error_Handler(void)
{
    __disable_irq();

    while (1)
    {
    }
}
