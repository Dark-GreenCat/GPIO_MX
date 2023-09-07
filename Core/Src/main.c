/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
/* USER CODE BEGIN PFP */
#define BIT(x) ((uint32_t) 1U << (x))
#define REG(x) ((volatile uint32_t *) (x))

inline void clear_bit(volatile uint32_t *REG, uint8_t bit_index) {
  *REG &= ~BIT(bit_index);
}

inline void set_bit(volatile uint32_t *REG, uint8_t bit_index) {
  *REG |= BIT(bit_index);
} 

typedef struct {
  uint32_t CRL;
  uint32_t CRH;
  uint32_t IDR;
  uint32_t ODR;
  uint32_t BSRR;
  uint32_t BRR;
  uint32_t LCKR;
} gpio_config_t;

#define USER_GPIOA_BASE 0x40010800
#define USER_LED_GPIO_PORT (gpio_config_t *) USER_GPIOA_BASE
#define USER_LED_GPIO_PIN 6
#define USER_BUTTON_GPIO_PORT (gpio_config_t *) USER_GPIOA_BASE
#define USER_BUTTON_GPIO_PIN 1
typedef enum { LOW, HIGH } gpio_state_t;

inline void gpio_write(gpio_config_t *GPIOx, uint16_t GPIO_PIN, gpio_state_t PIN_STATE) {
  clear_bit(&GPIOx->BSRR, GPIO_PIN);

  if (PIN_STATE == HIGH) set_bit(&GPIOx->BSRR, GPIO_PIN);
  else set_bit(&GPIOx->BRR, GPIO_PIN);
}

inline uint8_t gpio_read(gpio_config_t *GPIOx, uint16_t GPIO_PIN) {
  return (GPIOx->IDR & BIT(GPIO_PIN)) ? HIGH : LOW;
}

/* io_mode */
enum { MODE_INPUT, MODE_OUTPUT_LOW_SPEED, MODE_OUTPUT_MEDIUM_SPEED, MODE_OUTPUT_HIGH_SPEED };
/* io_config */
enum { CONFIG_INPUT_ANALOG, CONFIG_INPUT_FLOATING, CONFIG_INPUT_PULLUP, CONFIG_INPUT_PULLDOWN };
enum { CONFIG_OUTPUT_GP_PUSHPULL, CONFIG_OUTPUT_GP_OPENDRAIN, CONFIG_OUTPUT_AF_PUSHPULL, CONFIG_OUTPUT_AF_OPENDRAIN };

typedef struct {
  gpio_config_t *gpio_port;
  uint16_t gpio_pin;
  uint32_t io_mode;
  uint32_t io_config;
} gpio_init_t;

void gpio_init_pin(gpio_init_t *GPIO_INIT) {
  #define CLEAR_CNF_MODE(REG, x) { clear_bit((REG), (x)); clear_bit((REG), (x) + 1); }
  volatile uint32_t *config_reg = (GPIO_INIT->gpio_pin <= 7)? &GPIO_INIT->gpio_port->CRL : &GPIO_INIT->gpio_port->CRH;

  /* Config io_mode */
  CLEAR_CNF_MODE(config_reg, 4 * (GPIO_INIT->gpio_pin % 8))
  *config_reg |= ((uint32_t) GPIO_INIT->io_mode) << (4 * (GPIO_INIT->gpio_pin % 8));

  /* Config io_config */
  if(GPIO_INIT->io_mode != MODE_INPUT) {
    CLEAR_CNF_MODE(config_reg, 4 * (GPIO_INIT->gpio_pin % 8) + 2)
    *config_reg |= ((uint32_t) GPIO_INIT->io_config) << (4 * (GPIO_INIT->gpio_pin % 8) + 2);
  }
  else {
    if(GPIO_INIT->io_config != CONFIG_INPUT_PULLDOWN && GPIO_INIT->io_config != CONFIG_INPUT_PULLUP) {
      CLEAR_CNF_MODE(config_reg, 4 * (GPIO_INIT->gpio_pin % 8) + 2)
      *config_reg |= ((uint32_t) GPIO_INIT->io_config) << (4 * (GPIO_INIT->gpio_pin % 8) + 2);
    }
    else {
      CLEAR_CNF_MODE(config_reg, 4 * (GPIO_INIT->gpio_pin % 8) + 2)
      *config_reg |= ((uint32_t) 0b10) << (4 * (GPIO_INIT->gpio_pin % 8) + 2);
      if(GPIO_INIT->io_config == CONFIG_INPUT_PULLDOWN) {
        set_bit(&GPIO_INIT->gpio_port->BRR, GPIO_INIT->gpio_pin);
      }
      else {
        set_bit(&GPIO_INIT->gpio_port->BSRR, GPIO_INIT->gpio_pin);
      }
  }
  }
}

#define USER_RCC_BASE 0x40021000
#define USER_RCC_APB2ENR_OFFSET 0x18

inline void enable_RCC_GPIOA_clock() {
  set_bit(REG(USER_RCC_BASE + USER_RCC_APB2ENR_OFFSET), 2);
}

void gpio_init() {
  /* GPIO Ports Clock Enable */
  enable_RCC_GPIOA_clock();

  gpio_write(USER_LED_GPIO_PORT, USER_LED_GPIO_PIN, LOW);

  gpio_init_t GPIO_INIT;

  /* Initialize USER_LED */
  GPIO_INIT.gpio_port = USER_LED_GPIO_PORT;
  GPIO_INIT.gpio_pin = USER_LED_GPIO_PIN;
  GPIO_INIT.io_mode = MODE_OUTPUT_HIGH_SPEED;
  GPIO_INIT.io_config = CONFIG_OUTPUT_GP_PUSHPULL;
  gpio_init_pin(&GPIO_INIT);

  /* Initialize USER_BUTTON */
  GPIO_INIT.gpio_port = USER_BUTTON_GPIO_PORT;
  GPIO_INIT.gpio_pin = USER_BUTTON_GPIO_PIN;
  GPIO_INIT.io_mode = MODE_INPUT;
  GPIO_INIT.io_config = CONFIG_INPUT_PULLUP;
  gpio_init_pin(&GPIO_INIT);
}

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  //MX_GPIO_Init();
   gpio_init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    uint8_t button_state = gpio_read(USER_BUTTON_GPIO_PORT, USER_BUTTON_GPIO_PIN);
    gpio_write(USER_LED_GPIO_PORT, USER_LED_GPIO_PIN, !button_state);

    // gpio_write(USER_LED_GPIO_PORT, USER_LED_GPIO_PIN, HIGH);
    // HAL_Delay(500);
    // gpio_write(USER_LED_GPIO_PORT, USER_LED_GPIO_PIN, LOW);
    // HAL_Delay(500);
    }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_MX_GPIO_Port, LED_MX_Pin, GPIO_PIN_RESET);

  gpio_init();
  // /*Configure GPIO pin : BUTTON_MX_Pin */
  // GPIO_InitStruct.Pin = BUTTON_MX_Pin;
  // GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  // GPIO_InitStruct.Pull = GPIO_PULLUP;
  // HAL_GPIO_Init(BUTTON_MX_GPIO_Port, &GPIO_InitStruct);

  // /*Configure GPIO pin : LED_MX_Pin */
  // GPIO_InitStruct.Pin = LED_MX_Pin;
  // GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  // GPIO_InitStruct.Pull = GPIO_NOPULL;
  // GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  // HAL_GPIO_Init(LED_MX_GPIO_Port, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
