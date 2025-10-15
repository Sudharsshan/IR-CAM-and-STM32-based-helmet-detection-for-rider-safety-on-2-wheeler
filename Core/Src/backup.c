/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "dcmi.h"
#include "dma.h"
#include "i2c.h"
#include "rtc.h"
#include "spi.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "lcd.h"
#include "board.h"
#include "camera.h"
#include "ov2640.h" // <-- IMPORTANT: Make sure to include the official driver header

// IMPORTANT: Include the generated AI header files
#include "ai_platform.h"
#include "network.h"
#include "network_data.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
//extern ST7735_Object_t st7735_pObj; // We need to access the LCD object
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
#ifdef TFT96
// QQVGA (For 160x80 screen, we capture a 160x120 image)
#define FrameWidth 160
#define FrameHeight 120
#elif TFT18
// QQVGA2
#define FrameWidth 128
#define FrameHeight 160
#endif
// picture buffer
uint16_t pic[FrameHeight][FrameWidth] __attribute__((section(".RAM_D2")));
volatile uint32_t DCMI_FrameIsReady = 0;
uint32_t Camera_FPS=0;
uint8_t i = 0;

// --- AI Model Buffers & Handles ---
AI_ALIGNED(32)
static ai_u8 ai_activation_buffer[AI_NETWORK_DATA_ACTIVATIONS_SIZE];
AI_ALIGNED(32)
static ai_i8 ai_input_buffer[AI_NETWORK_IN_1_SIZE]; // Use ai_i8 for int8_t
AI_ALIGNED(32)
static ai_i8 ai_output_buffer[AI_NETWORK_OUT_1_SIZE]; // Use ai_i8 for int8_t
static ai_handle network = AI_HANDLE_NULL;

// --- Labels for YOUR custom model ---
const char *labels[] = { "With Helmet", "Without Helmet" };

static uint32_t helmet_on_timestamp = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
static void MPU_Config(void);
static void AI_Init(void);
static void AI_Run(const void *in_data, void *out_data);
static void Preprocess_Image(uint16_t* input_buf, int8_t* output_buf);
void Postprocess_And_Draw(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// THIS IS THE ONE AND ONLY, CORRECT MPU CONFIG
static void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};
  HAL_MPU_Disable();

  // Configure AXI SRAM as cacheable
  MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress      = 0x24000000;
  MPU_InitStruct.Size             = MPU_REGION_SIZE_512KB;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsBufferable     = MPU_ACCESS_BUFFERABLE;
  MPU_InitStruct.IsCacheable      = MPU_ACCESS_CACHEABLE;
  MPU_InitStruct.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.Number           = MPU_REGION_NUMBER0;
  MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL1;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE;
  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  // Configure D2 RAM (where 'pic' buffer is) as NON-CACHEABLE
  MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
  MPU_InitStruct.Number           = MPU_REGION_NUMBER1;
  MPU_InitStruct.BaseAddress      = 0x30000000;
  MPU_InitStruct.Size             = MPU_REGION_SIZE_256KB;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsBufferable     = MPU_ACCESS_BUFFERABLE;
  MPU_InitStruct.IsCacheable      = MPU_ACCESS_NOT_CACHEABLE; // The magic line
  MPU_InitStruct.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL0;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE;
  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

// This is our new function to prepare the camera image for the AI model
// This is our final, corrected function to prepare the camera image for the AI model
// FINAL VERSION: Pre-processes a center-cropped, RGB image for your model
void Preprocess_Image(uint16_t* input_buf_rgb565, int8_t* output_buf_rgb888)
{
    // Camera frame dimensions
    uint16_t const input_w = 160;
    uint16_t const input_h = 120;

    // Model input dimensions
    uint16_t const model_w = 96;
    uint16_t const model_h = 96;

    // --- Calculate the offsets for a center crop ---
    uint16_t const x_offset = (input_w - model_w) / 2; // (160 - 96) / 2 = 32
    uint16_t const y_offset = (input_h - model_h) / 2; // (120 - 96) / 2 = 12

    uint32_t out_idx = 0;

    // Loop through the dimensions of the model's input
    for (uint16_t y = 0; y < model_h; y++)
    {
        for (uint16_t x = 0; x < model_w; x++)
        {
            // Get the pixel from the *center* of the camera buffer
            uint16_t pixel = input_buf_rgb565[(y + y_offset) * input_w + (x + x_offset)];

            // Convert from 16-bit RGB565 to 8-bit R, G, B
            uint8_t r = (((pixel >> 11) & 0x1F) * 255) / 31;
            uint8_t g = (((pixel >> 5)  & 0x3F) * 255) / 63;
            uint8_t b = ((pixel & 0x1F) * 255) / 31;

            // Store the 3 color channels in the AI model's input buffer
            // and shift the range from 0-255 to the int8 range of -128 to +127
            output_buf_rgb888[out_idx++] = (int8_t)(r - 128);
            output_buf_rgb888[out_idx++] = (int8_t)(g - 128);
            output_buf_rgb888[out_idx++] = (int8_t)(b - 128);
        }
    }
}


static void AI_Init(void) {
    ai_error err;

    // Create an instance of the network
    err = ai_network_create(&network, AI_NETWORK_DATA_CONFIG);
    if (err.type != AI_ERROR_NONE) {
        // Handle error
        while(1);
    }

    // CORRECTED: Use the suggested function name and the dedicated activation buffer
        const ai_network_params params = AI_NETWORK_PARAMS_INIT(
            AI_NETWORK_DATA_WEIGHTS(ai_network_data_weights_get()),
            AI_NETWORK_DATA_ACTIVATIONS(ai_activation_buffer)
        );

    if (!ai_network_init(network, &params)) {

        // Handle error
        while(1);
    }
}

static void AI_Run(const void *in_data, void *out_data) {
	ai_buffer* ai_input = ai_network_inputs_get(network, NULL);
    ai_buffer* ai_output = ai_network_outputs_get(network, NULL);
    ai_input[0].data = AI_HANDLE_PTR(in_data);
    ai_output[0].data = AI_HANDLE_PTR(out_data);
    ai_network_run(network, ai_input, ai_output);
}

// --- FINAL POST-PROCESSING AND DRAW FUNCTION for your FOMO model ---
// --- FINAL, CORRECTED POST-PROCESSING AND DRAW FUNCTION ---
void Postprocess_And_Draw(void)
{
    bool helmet_found = false;

    // Your FOMO model properties from network.h
    const int grid_w = AI_NETWORK_OUT_1_WIDTH;    // Should be 12
    const int grid_h = AI_NETWORK_OUT_1_HEIGHT;   // Should be 12
    const int num_classes = AI_NETWORK_OUT_1_CHANNEL; // Should be 3

    // We'll work directly with integer scores. A score above 50 is a confident detection.
    const int8_t confidence_threshold = 50;

    int8_t* out_ptr = (int8_t*)ai_output_buffer;

    // First, display the raw camera feed as the background
	ST7735_FillRGBRect(&st7735_pObj, 0, 0, (uint8_t *)&pic[20][0], 160, 80);

    // --- Loop through each cell in the output grid ---
    for (int y = 0; y < grid_h; y++)
    {
        for (int x = 0; x < grid_w; x++)
        {
            // Find the winning class for this cell by finding the highest integer score
            int8_t max_score_int = -128;
            int winning_class_index = -1;

            for (int c = 0; c < num_classes; c++) {
                if (out_ptr[c] > max_score_int) {
                    max_score_int = out_ptr[c];
                    winning_class_index = c;
                }
            }

            // Check if we found a class that isn't background and meets our confidence threshold
            // The label order is often alphabetical: "With Helmet" is 0, "Without Helmet" is 1.
            // Cube.AI might add "background" as the last class (index 2). Let's check our targets.
            if (max_score_int > confidence_threshold)
            {
                int box_size = 160 / grid_w;
                int x1 = x * box_size;
                int y1 = y * box_size;

                // If we found "With Helmet" (index 0)
                if (winning_class_index == 0) {
                    helmet_found = true;
                    ST7735_LCD_Driver.FillRect(&st7735_pObj, x1, y1, box_size, box_size, GREEN);
                }
                // If we found "Without Helmet" (index 1)
                else if (winning_class_index == 1) {
                	ST7735_LCD_Driver.FillRect(&st7735_pObj, x1, y1, box_size, box_size, RED);
                }
                // We ignore the background class (index 2)
            }

            // Move the pointer to the next cell's data
            out_ptr += num_classes;
        }
    }

    // --- Control the kill switch based on our findings ---
//    if (helmet_found) {
//        HAL_GPIO_WritePin(BIKE_KILL_SWITCH_GPIO_Port, BIKE_KILL_SWITCH_Pin, GPIO_PIN_SET);
//    } else {
//        HAL_GPIO_WritePin(BIKE_KILL_SWITCH_GPIO_Port, BIKE_KILL_SWITCH_Pin, GPIO_PIN_RESET);
//    }
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  MPU_Config();
  /* USER CODE END 1 */

  /* Enable I-Cache---------------------------------------------------------*/
  SCB_EnableICache();

  /* Enable D-Cache---------------------------------------------------------*/
  SCB_EnableDCache();

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
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_DCMI_Init();
  MX_I2C1_Init();
  MX_RTC_Init();
  MX_SPI4_Init();
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */

  board_button_init();
  board_led_init();

  LCD_Test();
  LCD_SetBrightness(100);

  // Initialize the AI Model
  AI_Init();

  uint8_t text[30];

  #ifdef TFT96
  Camera_Init_Device(&hi2c1, FRAMESIZE_QQVGA);
  #elif TFT18
  Camera_Init_Device(&hi2c1, FRAMESIZE_QQVGA2);
  #endif
  //clean Ypos 58
  // Clear the area for the FPS counter BEFORE writing the new one
  ST7735_LCD_Driver.FillRect(&st7735_pObj, 0, 0, ST7735Ctx.Width, ST7735Ctx.Height, BLACK);


  uint8_t text_buffer[50];
  if (ov2640_init(FRAMESIZE_QQVGA) == Camera_OK)
  {
   sprintf((char *)text_buffer, "System Online..");
   LCD_ShowString(10, 30, 200, 16, 16, text_buffer);
   HAL_Delay(1000);
   // We will now start the capture inside the loop
  }
  else
  {
   // Handle camera init failure
   sprintf((char *)text_buffer, "No camera found.");
   LCD_ShowString(10, 30, 200, 16, 16, text_buffer);
   while(1);
  }

//  while (HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin) == GPIO_PIN_RESET)
//  {
//
//    sprintf((char *)&text, "Camera id:0x%x        ", hcamera.device_id);
//    LCD_ShowString(4, 58, ST7735Ctx.Width, 16, 12, text);
//
//    HAL_Delay(500);
//
//    sprintf((char *)&text, "LongPress K1 to Run");
//    LCD_ShowString(4, 58, ST7735Ctx.Width, 16, 12, text);
//
//  	HAL_Delay(500);
//  }
  HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_CONTINUOUS, (uint32_t)&pic, FrameWidth * FrameHeight * 2 / 4);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  if (DCMI_FrameIsReady)
	      {
	          DCMI_FrameIsReady = 0;
	          // --- THE MAGIC LINE ---
	          // Manually invalidate the D-Cache for the pic buffer.
	          // This forces the CPU to fetch the fresh data written by the DMA from RAM.
	          SCB_InvalidateDCache_by_Addr((uint32_t*)pic, sizeof(pic));

	          #ifdef TFT96
	          ST7735_FillRGBRect(&st7735_pObj,0,0,(uint8_t *)&pic[20][0], ST7735Ctx.Width, 80);
	          #elif TFT18
	          ST7735_FillRGBRect(&st7735_pObj,0,0,(uint8_t *)&pic[0][0], ST7735Ctx.Width, ST7735Ctx.Height);
  	          #endif

	          // --- 1. PRE-PROCESS ---
	          Preprocess_Image((uint16_t*)pic, (int8_t*)ai_input_buffer);

	          // --- 2. INFERENCE ---
	          AI_Run(ai_input_buffer, ai_output_buffer);

	          // --- 3. POST-PROCESSING AND DRAW RESULTS ---
//	          Postprocess_And_Draw();
	          // --- FOMO Post-Processing Logic ---
	              // The output is a grid. These values come directly from your network.h!
	          bool helmet_found = false;
	          const int8_t confidence_threshold = 50;
	          const int grid_w = AI_NETWORK_OUT_1_WIDTH;    // Should be 12
	          const int grid_h = AI_NETWORK_OUT_1_HEIGHT;   // Should be 12
	          const int num_classes = AI_NETWORK_OUT_1_CHANNEL; // Should be 3

	          int8_t* out_ptr = (int8_t*)ai_output_buffer;

	          // Display the raw camera feed as the background
	          //ST7735_FillRGBRect(&st7735_pObj, 0, 0, (uint8_t *)&pic[20][0], 160, 80);

	          for (int y = 0; y < grid_h; y++) {
	        	  for (int x = 0; x < grid_w; x++) {
	        		  // Find the winning class for this cell
	                  int8_t max_score_int = -128;
	                  int winning_class_index = -1;

	                  for (int c = 0; c < num_classes; c++) {
	                	  if (out_ptr[c] > max_score_int) {
	                		  max_score_int = out_ptr[c];
	                          winning_class_index = c;
	                      }
	                  }

	                  // NOTE: The label order is alphabetical.
	                  // In Edge Impulse: "With Helmet" is 0, "Without Helmet" is 1.
	                  // Cube.AI adds a "background" class, often at the end.
	                  // Let's assume for now 0="With Helmet", 1="Without Helmet", 2="background"
	                  if (winning_class_index == 1 && max_score_int > confidence_threshold) // Threshold on int8 score
	                  {
	                       helmet_found = true;
	                       int box_size = 160 / grid_w;
	                       ST7735_LCD_Driver.FillRect(&st7735_pObj, x * box_size, y * box_size, box_size, box_size, GREEN);
	                  }

	                  out_ptr += num_classes;
	              }
	          }



	          // 4. Control the kill switch GPIO
	          // --- NON-BLOCKING LOGIC ---
	          uint8_t text_buffer[50];
	          		if (helmet_found)
	          		{
	          			// A helmet was found! Set the pin HIGH and start/reset the 10-second timer.
	          			HAL_GPIO_WritePin(BIKE_KILL_SWITCH_GPIO_Port, BIKE_KILL_SWITCH_Pin, GPIO_PIN_SET);
	          			helmet_on_timestamp = HAL_GetTick();

	                      // Display the "Helmet Detected" message
	          			sprintf((char *)text_buffer,"Helmet Detected");
	          			LCD_ShowString(70, 65, 90, 12, 12, text_buffer); // Bottom right
	          		}
	          		else if (helmet_on_timestamp > 0)
	          		{
	          			// No helmet was found, but the timer is active. Check if 10s have passed.
	          			if (HAL_GetTick() - helmet_on_timestamp > 5000) // Every 5 seconds
	          			{
	          				// 10 seconds are up! Turn the pin LOW and stop the timer.
	          				HAL_GPIO_WritePin(BIKE_KILL_SWITCH_GPIO_Port, BIKE_KILL_SWITCH_Pin, GPIO_PIN_RESET);
	          				helmet_on_timestamp = 0;

	                          // Also clear the message from the screen
	                          ST7735_LCD_Driver.FillRect(&st7735_pObj, 70, 65, 90, 12, BLACK);
	          			}
	          			else
	          			{
	          				// Timer is still running, so keep the message on screen
	          				sprintf((char *)text_buffer,"Helmet Detected");
	          				LCD_ShowString(70, 65, 90, 12, 12, text_buffer);
	          			}
	          		}
	          		// If no helmet is found and the timer is off, the pin is already LOW and the screen is clear.

	          // --- Update UI Elements like FPS counter ---
	          ST7735_LCD_Driver.FillRect(&st7735_pObj, 5, 5, 60, 12, BLACK);
	          sprintf((char *)&text,"%luFPS",Camera_FPS);
	          LCD_ShowString(5,5,60,16,12,text);

	          board_led_toggle();
	      }
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

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48|RCC_OSCILLATORTYPE_HSE
                              |RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 2;
  RCC_OscInitStruct.PLL.PLLN = 44;
  RCC_OscInitStruct.PLL.PLLP = 1;
  RCC_OscInitStruct.PLL.PLLQ = 46;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_HSI48, RCC_MCODIV_4);
}

/* USER CODE BEGIN 4 */
void HAL_DCMI_FrameEventCallback(DCMI_HandleTypeDef *hdcmi)
{
	static uint32_t count = 0,tick = 0;

	if(HAL_GetTick() - tick >= 1000)
	{
		tick = HAL_GetTick();
		Camera_FPS = count;
		count = 0;
	}
	count ++;

  DCMI_FrameIsReady ++;
}
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
