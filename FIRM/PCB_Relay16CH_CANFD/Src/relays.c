/*
  ******************************************************************************
  * File Name          : relays.h
  * Description        : This file provides code for the ad7606 driver using spi.
  * Author             : Jason TigerTE
  * Date               : 2021-01-05
  ******************************************************************************
*/

#include "main.h"
#include "fdcan.h"
#include "tim.h"
#include "gpio.h"
#include "relays.h"

#define CANID_CONFIG       (BordBaseID + BoardIDIndex * 0x100 + 0x0)
#define CANID_ADC0         (BordBaseID + BoardIDIndex * 0x100 + 0x1)
#define CANID_ADC1         (BordBaseID + BoardIDIndex * 0x100 + 0x2)
#define CANID_ADC2         (BordBaseID + BoardIDIndex * 0x100 + 0x3)
#define CANID_VERSION      (BordBaseID + BoardIDIndex * 0x100 + 0xF)

#define LED_Green_Off()           	HAL_GPIO_WritePin(LED_Green_GPIO_Port, LED_Green_Pin, GPIO_PIN_SET)
#define LED_Green_On()            	HAL_GPIO_WritePin(LED_Green_GPIO_Port, LED_Green_Pin, GPIO_PIN_RESET)
#define LED_Red_Off()             	HAL_GPIO_WritePin(LED_Red_GPIO_Port, LED_Red_Pin, GPIO_PIN_SET)
#define LED_Red_On()              	HAL_GPIO_WritePin(LED_Red_GPIO_Port, LED_Red_Pin, GPIO_PIN_RESET)

#define LED_Green_Toggle()          	HAL_GPIO_TogglePin(LED_Green_GPIO_Port, LED_Green_Pin)
#define LED_Red_Toggle()        	HAL_GPIO_TogglePin(LED_Red_GPIO_Port, LED_Red_Pin)



uint8_t TxCANData[64];
uint8_t TxCANDataBuffer[64];
uint8_t RxCANDataBuffer[64];  //recieve buffer1
uint16_t Request_Data = 0x0000;
uint16_t Mask_Data = 0xFFFF;
uint16_t BoardState = 0x0000;
uint16_t BoardStateNext = 0x0000;
uint32_t LEDTimerCount = 0;


void Relays_Start(void) {

	if(FDCAN1_Filter() == 0) {
		HAL_FDCAN_Start(&hfdcan1);   //Start FDCAN
		HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);
	}

	if(HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_BUS_OFF, 0) != HAL_OK) {
		Error_Handler();
	}

	LED_Green_Off();
	LED_Red_Off();
	// Reset Send out
	TxCANData[0] = SW_VERSION_YEAR;
	TxCANData[1] = SW_VERSION_MONTH;
	TxCANData[2] = SW_VERSION_DAY;
	TxCANData[3] = SW_VERSION_VRIANT;
	FDCAN1_Send_Msg(CANID_VERSION, &TxCANData[0], FDCAN_DLC_BYTES_4, 0);

	HAL_TIM_Base_Start_IT(&htim1);
	//HAL_TIM_Base_Start_IT(&htim1);
}

/**
  * @brief CAN Recieve_Callback
  * @param hfdcan: CAN handel; RxFifo0ITs: revieve buffer
  * @retval None
  */
  //-----------------------------------
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef* hfdcan, uint32_t RxFifo0ITs) {
	uint32_t mid;
	FDCAN_RxHeaderTypeDef FDCAN1_RxHeader;
	HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &FDCAN1_RxHeader, &RxCANDataBuffer[0]);
	mid = FDCAN1_RxHeader.Identifier;
	
	if(FDCAN1_RxHeader.IdType == FDCAN_EXTENDED_ID) {
        
		if((mid == CANID_RELAY_ACTION) && (FDCAN1_RxHeader.DataLength == FDCAN_DLC_BYTES_4)) {
			LED_Red_Toggle();
			Request_Data = *(uint16_t*) (&RxCANDataBuffer[0]);
			Mask_Data = *(uint16_t*) (&RxCANDataBuffer[2]);
			BoardStateNext = BoardState & (~Mask_Data);
			BoardStateNext = BoardStateNext | (Request_Data & Mask_Data);
			//RELAY1
			if(BoardStateNext & 0x0001) RELAY0_NO();
			else RELAY0_NC();
			//RELAY2
			if((BoardStateNext >> 1) & 0x0001) RELAY1_NO();
			else RELAY1_NC();
			//RELAY3
			if((BoardStateNext >> 2) & 0x0001) RELAY2_NO();
			else RELAY2_NC();
			//RELAY4
			if((BoardStateNext >> 3) & 0x0001) RELAY3_NO();
			else RELAY3_NC();
			//RELAY5
			if((BoardStateNext >> 4) & 0x0001) RELAY4_NO();
			else RELAY4_NC();
			//RELAY6
			if((BoardStateNext >> 5) & 0x0001) RELAY5_NO();
			else RELAY5_NC();
			//RELAY7
			if((BoardStateNext >> 6) & 0x0001) RELAY6_NO();
			else RELAY6_NC();
			//RELAY8
			if((BoardStateNext >> 7) & 0x0001) RELAY7_NO();
			else RELAY7_NC();
			//RELAY9
			if((BoardStateNext >> 8) & 0x0001) RELAY8_NO();
			else RELAY8_NC();
			//RELAY10
			if((BoardStateNext >> 9) & 0x0001) RELAY9_NO();
			else RELAY9_NC();
			//RELAY11
			if((BoardStateNext >> 10) & 0x0001) RELAY10_NO();
			else RELAY10_NC();
			//RELAY12
			if((BoardStateNext >> 11) & 0x0001) RELAY11_NO();
			else RELAY11_NC();
			//RELAY13
			if((BoardStateNext >> 12) & 0x0001) RELAY12_NO();
			else RELAY12_NC();
			//RELAY14
			if((BoardStateNext >> 13) & 0x0001) RELAY13_NO();
			else RELAY13_NC();
			//RELAY15
			if((BoardStateNext >> 14) & 0x0001) RELAY14_NO();
			else RELAY14_NC();
			//RELAY16
			if((BoardStateNext >> 15) & 0x0001) RELAY15_NO();
			else RELAY15_NC();
			BoardState = BoardStateNext;
			*(uint16_t*) (&TxCANData[0]) = BoardState;
			FDCAN1_Send_Msg(CANID_RELAY_RESPONSE, &TxCANData[0], FDCAN_DLC_BYTES_2, 0);
		}
		if(mid == CANID_CONFIG) {
			TxCANData[0] = SW_VERSION_YEAR;
			TxCANData[1] = SW_VERSION_MONTH;
			TxCANData[2] = SW_VERSION_DAY;
			TxCANData[3] = SW_VERSION_VRIANT;
			FDCAN1_Send_Msg(CANID_VERSION, &TxCANData[0], FDCAN_DLC_BYTES_4, 0);
		}
	}

	HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);

}

/**
  * @brief Timer_Callback
  * @param htim: timer handel
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim) {
	if(htim->Instance == htim1.Instance) {
		LEDTimerCount++;
		if(LEDTimerCount >= 1000) {
			LEDTimerCount = 0;
			//LED_Red_Toggle();
			LED_Green_Toggle();
		}

	}
}

/**
  * @brief HAL FDCAN ErrorStatusCallback
  * @param1 hfdcan: fdcan handel
  * @param2 ErrorStatusITs:
  * @retval None
  */
void HAL_FDCAN_ErrorStatusCallback(FDCAN_HandleTypeDef* hfdcan, uint32_t ErrorStatusITs) {
	FDCAN_ProtocolStatusTypeDef protocol_status;

	HAL_FDCAN_GetProtocolStatus(hfdcan, &protocol_status);

	if(protocol_status.BusOff == 1) {
		if(hfdcan->Instance == FDCAN1) {
			HAL_FDCAN_DeInit(hfdcan);
			MX_FDCAN1_Init();
			if(FDCAN1_Filter() == 0) {
				HAL_FDCAN_Start(hfdcan);
				HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);
			}
			if(HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_BUS_OFF, 0) != HAL_OK) {
				Error_Handler();
			}

		}
	}

}
/**
  * @brief HAL_WWDG_EarlyWakeupCallback
  * @param1 hwwdg: window watch dog handel
  * @retval None
  */
//void HAL_WWDG_EarlyWakeupCallback(WWDG_HandleTypeDef* hwwdg) {

//	HAL_WWDG_Refresh(hwwdg);
//}


/*! end of the file */
