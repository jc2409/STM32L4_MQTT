/**
  ******************************************************************************
  * @file    Wifi/MQTT_Client/src/main.c
  * @brief   Example main file for connecting to the Mosquitto MQTT broker.
  * @attention
  * This file demonstrates connecting to the public Mosquitto MQTT broker,
  * publishing a test message, and maintaining the connection.
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "Timer.h"           // Timer implementation
#include "MQTTInterface.h"   // MQTT network interface (defines Network and wrappers)

/* Undefine SUCCESS to avoid conflicts with the MQTT client's enum definition */
#ifdef SUCCESS
#undef SUCCESS
#endif

#include "MQTTClient.h"      // Paho Embedded MQTT Client

/* Private defines -----------------------------------------------------------*/
#define SSID                "YOUR_WIFI_SSID"
#define PASSWORD            "YOUR_WIFI_PASSWORD"


/* Mosquitto broker settings */
#define MQTT_BROKER_HOST    "test.mosquitto.org"
#define MQTT_BROKER_PORT    1883

#define MQTT_BUFFER_SIZE    256

#define TERMINAL_USE

#ifdef TERMINAL_USE
  #define LOG(a) printf a
#else
  #define LOG(a)
#endif

/* Private variables ---------------------------------------------------------*/
#if defined (TERMINAL_USE)
extern UART_HandleTypeDef hDiscoUart;
#endif

I2C_HandleTypeDef hi2c2;

/* MQTT communication buffers */
unsigned char mqtt_sendbuf[MQTT_BUFFER_SIZE];
unsigned char mqtt_readbuf[MQTT_BUFFER_SIZE];

/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);

/* Include Wi-Fi driver headers */
#include "wifi.h"
#include "es_wifi.h"

/* Global variable for IP address */
uint8_t IP_Addr[4];

/*------------------------------------------------------------------------------
  wifi_connect() - Connect to Wi-Fi using the ES-WiFi APIs.
------------------------------------------------------------------------------*/
int wifi_connect(void)
{
    uint8_t MAC_Addr[6];

    /* Initialize the Wi-Fi module */
    if (WIFI_Init() != WIFI_STATUS_OK) {
        LOG(("ERROR: Failed to initialize Wi-Fi module.\n"));
        return -1;
    }
    LOG(("ES-WiFi Initialized.\n"));
    
    /* Get MAC address */
    if (WIFI_GetMAC_Address(MAC_Addr, sizeof(MAC_Addr)) == WIFI_STATUS_OK) {
        LOG(("> ES-WiFi module MAC Address: %02X:%02X:%02X:%02X:%02X:%02X\n",
             MAC_Addr[0], MAC_Addr[1], MAC_Addr[2],
             MAC_Addr[3], MAC_Addr[4], MAC_Addr[5]));
    } else {
        LOG(("ERROR: Cannot get MAC address.\n"));
        return -1;
    }
    
    /* Connect to the Access Point */
    LOG(("\nConnecting to %s , %s\n", SSID, PASSWORD));
    if (WIFI_Connect(SSID, PASSWORD, WIFI_ECN_WPA2_PSK) != WIFI_STATUS_OK) {
        LOG(("ERROR: ES-WiFi module NOT connected.\n"));
        return -1;
    }
    
    /* Retrieve IP address */
    if (WIFI_GetIP_Address(IP_Addr, sizeof(IP_Addr)) != WIFI_STATUS_OK) {
        LOG(("ERROR: ES-WiFi module CANNOT get IP address.\n"));
        return -1;
    }
    
    LOG(("> ES-WiFi module connected: got IP Address: %d.%d.%d.%d\n",
         IP_Addr[0], IP_Addr[1], IP_Addr[2], IP_Addr[3]));
    
    return 0;
}

/*------------------------------------------------------------------------------
  SystemClock_Config() - Configure the system clock.
------------------------------------------------------------------------------*/
static void SystemClock_Config(void)
{
    RCC_ClkInitTypeDef RCC_ClkInitStruct;
    RCC_OscInitTypeDef RCC_OscInitStruct;

    /* Enable MSI and activate PLL with MSI as source */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
    RCC_OscInitStruct.MSIState = RCC_MSI_ON;
    RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
    RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
    RCC_OscInitStruct.PLL.PLLM = 1;
    RCC_OscInitStruct.PLL.PLLN = 40;
    RCC_OscInitStruct.PLL.PLLR = 2;
    RCC_OscInitStruct.PLL.PLLP = 7;
    RCC_OscInitStruct.PLL.PLLQ = 4;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        while(1);
    }

    /* Configure clocks dividers and select PLL as system clock source */
    RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
                                   RCC_CLOCKTYPE_PCLK1  | RCC_CLOCKTYPE_PCLK2);
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK) {
        while(1);
    }
}

static void MX_I2C2_Init(void)
{
  hi2c2.Instance = I2C2;
  hi2c2.Init.Timing = 0x00100D14;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
}

/*------------------------------------------------------------------------------
  main() - Entry point.
------------------------------------------------------------------------------*/
int main(void)
{
    HAL_Init();
    SystemClock_Config();
		MX_I2C2_Init();
    BSP_LED_Init(LED2);
		BSP_ACCELERO_Init();
		BSP_GYRO_Init();
		BSP_HSENSOR_Init();
		BSP_MAGNETO_Init();
		BSP_PSENSOR_Init();
		BSP_TSENSOR_Init();
		
		HAL_Delay(1000);

	
#if defined (TERMINAL_USE)
    /* Initialize UART for debugging */
    hDiscoUart.Instance = DISCOVERY_COM1;
    hDiscoUart.Init.BaudRate = 115200;
    hDiscoUart.Init.WordLength = UART_WORDLENGTH_8B;
    hDiscoUart.Init.StopBits = UART_STOPBITS_1;
    hDiscoUart.Init.Parity = UART_PARITY_NONE;
    hDiscoUart.Init.Mode = UART_MODE_TX_RX;
    hDiscoUart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    hDiscoUart.Init.OverSampling = UART_OVERSAMPLING_16;
    hDiscoUart.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    hDiscoUart.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    BSP_COM_Init(COM1, &hDiscoUart);
    printf("****** MQTT Mosquitto Broker Demo ******\n\n");
#endif
		
    /* Connect to Wi-Fi */
    if (wifi_connect() != 0) {
        printf("Wi-Fi connection failed!\n");
        while (1);
    }
    printf("Wi-Fi connected successfully.\n");

    /* Resolve the MQTT broker hostname to an IP address */
    uint8_t brokerIP[4];
    if (WIFI_GetHostAddress(MQTT_BROKER_HOST, brokerIP, sizeof(brokerIP)) != WIFI_STATUS_OK) {
        printf("Failed to resolve broker hostname: %s\n", MQTT_BROKER_HOST);
        while (1);
    }
    printf("Broker IP: %d.%d.%d.%d\n", brokerIP[0], brokerIP[1], brokerIP[2], brokerIP[3]);

    /* Open a TCP client connection to the MQTT broker using socket 0 */
    if (WIFI_OpenClientConnection(0, WIFI_TCP_PROTOCOL, "MQTT", brokerIP, MQTT_BROKER_PORT, 0) != WIFI_STATUS_OK) {
        printf("Failed to open client connection to broker\n");
        while (1);
    }
    printf("TCP client connection to MQTT broker established\n");

    /* Set up the MQTT network interface */
    Network network;
    network.socket = 0;  // Using socket 0 as opened above
    network.mqttread = mqtt_network_read;
    network.mqttwrite = mqtt_network_write;

    /* Initialize the MQTT client */
    MQTTClient client;
    MQTTClientInit(&client, &network, 3000, mqtt_sendbuf, sizeof(mqtt_sendbuf),
                   mqtt_readbuf, sizeof(mqtt_readbuf));

    /* Set up MQTT connection parameters */
    MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;
    connectData.MQTTVersion = 4;     // Protocol level 4 for MQTT 3.1.1
    connectData.clientID.cstring = "B-L475E-IOT01A1_Client";
    connectData.cleansession = 1;
    connectData.keepAliveInterval = 60;

    int rc = MQTTConnect(&client, &connectData);
    if (rc != MQTT_SUCCESS) {
        printf("MQTT connect failed with return code %d\n", rc);
        while (1);
    }
    printf("MQTT connected successfully\n");

    /* Main loop: process incoming MQTT messages and maintain the connection */
    while (1) {
				int16_t accelData[3];
				BSP_ACCELERO_AccGetXYZ(accelData);
				
				float gyroData[3];
				BSP_GYRO_GetXYZ(gyroData);
				
				float humidity = BSP_HSENSOR_ReadHumidity();
				
				int16_t magnetoData[3];
				BSP_MAGNETO_GetXYZ(magnetoData);
			
				float pressure = BSP_PSENSOR_ReadPressure();
			
				float temperature = BSP_TSENSOR_ReadTemp();

				char payload[256];
				snprintf(payload, sizeof(payload),
								 "{"
								 "\"temperature\":%.2f,"
								 "\"humidity\":%.2f,"
								 "\"pressure\":%.2f,"
								 "\"magnetic_field\":{\"x\":%d,\"y\":%d,\"z\":%d},"
								 "\"accelerometer\":{\"x\":%d,\"y\":%d,\"z\":%d},"
								 "\"gyroscope\":{\"x\":%f,\"y\":%f,\"z\":%f}"
								 "}",
								 temperature, humidity, pressure,
								 magnetoData[0], magnetoData[1], magnetoData[2],
								 accelData[0], accelData[1], accelData[2],
								 gyroData[0], gyroData[1], gyroData[2]);
			
				printf("Payload: %s\n", payload);
				
			  /* Publish a test message */
				MQTTMessage message;
				message.payload = payload;
				message.payloadlen = strlen(payload);
				message.qos = QOS0;
				message.retained = 0;

				rc = MQTTPublish(&client, "sensor/data", &message);
				if (rc != MQTT_SUCCESS) {
						printf("MQTT publish failed with return code %d\n", rc);
				} else {
						printf("MQTT publish succeeded\n");
				}
				HAL_Delay(1000);
        MQTTYield(&client, 1000);
        // Additional application logic can be added here.
    }
}

/*------------------------------------------------------------------------------
  Retarget printf to USART for debugging (if TERMINAL_USE is defined)
------------------------------------------------------------------------------*/
#if defined (TERMINAL_USE)
#ifdef __GNUC__
int __io_putchar(int ch)
#else
int fputc(int ch, FILE *f)
#endif
{
    HAL_UART_Transmit(&hDiscoUart, (uint8_t *)&ch, 1, 0xFFFF);
    return ch;
}
#endif

/*------------------------------------------------------------------------------
  EXTI line detection callback
------------------------------------------------------------------------------*/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    switch (GPIO_Pin)
    {
        case GPIO_PIN_1:
            SPI_WIFI_ISR();
            break;
        default:
            break;
    }
}

/*------------------------------------------------------------------------------
  SPI3 interrupt handler
------------------------------------------------------------------------------*/
void SPI3_IRQHandler(void)
{
    HAL_SPI_IRQHandler(&hspi);
}
