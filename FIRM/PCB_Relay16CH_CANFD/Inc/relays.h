/*
  ******************************************************************************
  * File Name          : spi_ad7606.h
  * Description        : This file provides code for the ad7606 driver using spi.
  * Author             : Jason TigerTE
  * Date               : 2023-09-14
  ******************************************************************************
*/
#ifndef __RELAYS_H_
#define __RELAYS_H_

/*! -------------------------------------------------------------------------- */
/*! Include headers */
#include <stdint.h>
#include "base_ID_general_api.h"

#define	RELAY0_NO()	  HAL_GPIO_WritePin(Relay0_GPIO_Port,Relay0_Pin,GPIO_PIN_SET)
#define	RELAY1_NO()	  HAL_GPIO_WritePin(Relay1_GPIO_Port,Relay1_Pin,GPIO_PIN_SET)
#define	RELAY2_NO()	  HAL_GPIO_WritePin(Relay2_GPIO_Port,Relay2_Pin,GPIO_PIN_SET)
#define	RELAY3_NO()	  HAL_GPIO_WritePin(Relay3_GPIO_Port,Relay3_Pin,GPIO_PIN_SET)
#define	RELAY4_NO()  	HAL_GPIO_WritePin(Relay4_GPIO_Port,Relay4_Pin,GPIO_PIN_SET)
#define	RELAY5_NO()	  HAL_GPIO_WritePin(Relay5_GPIO_Port,Relay5_Pin,GPIO_PIN_SET)
#define	RELAY6_NO()  	HAL_GPIO_WritePin(Relay6_GPIO_Port,Relay6_Pin,GPIO_PIN_SET)
#define	RELAY7_NO()	  HAL_GPIO_WritePin(Relay7_GPIO_Port,Relay7_Pin,GPIO_PIN_SET)
#define	RELAY8_NO()	  HAL_GPIO_WritePin(Relay8_GPIO_Port,Relay8_Pin,GPIO_PIN_SET)
#define	RELAY9_NO()  	HAL_GPIO_WritePin(Relay9_GPIO_Port,Relay9_Pin,GPIO_PIN_SET)
#define	RELAY10_NO()	HAL_GPIO_WritePin(Relay10_GPIO_Port,Relay10_Pin,GPIO_PIN_SET)
#define	RELAY11_NO()	HAL_GPIO_WritePin(Relay11_GPIO_Port,Relay11_Pin,GPIO_PIN_SET)
#define	RELAY12_NO()	HAL_GPIO_WritePin(Relay12_GPIO_Port,Relay12_Pin,GPIO_PIN_SET)
#define	RELAY13_NO()	HAL_GPIO_WritePin(Relay13_GPIO_Port,Relay13_Pin,GPIO_PIN_SET)
#define	RELAY14_NO()	HAL_GPIO_WritePin(Relay14_GPIO_Port,Relay14_Pin,GPIO_PIN_SET)
#define	RELAY15_NO()	HAL_GPIO_WritePin(Relay15_GPIO_Port,Relay15_Pin,GPIO_PIN_SET)



#define	RELAY0_NC()	  HAL_GPIO_WritePin(Relay0_GPIO_Port,Relay0_Pin,GPIO_PIN_RESET)
#define	RELAY1_NC()	  HAL_GPIO_WritePin(Relay1_GPIO_Port,Relay1_Pin,GPIO_PIN_RESET)
#define	RELAY2_NC()	  HAL_GPIO_WritePin(Relay2_GPIO_Port,Relay2_Pin,GPIO_PIN_RESET)
#define	RELAY3_NC()	  HAL_GPIO_WritePin(Relay3_GPIO_Port,Relay3_Pin,GPIO_PIN_RESET)
#define	RELAY4_NC()  	HAL_GPIO_WritePin(Relay4_GPIO_Port,Relay4_Pin,GPIO_PIN_RESET)
#define	RELAY5_NC()	  HAL_GPIO_WritePin(Relay5_GPIO_Port,Relay5_Pin,GPIO_PIN_RESET)
#define	RELAY6_NC()  	HAL_GPIO_WritePin(Relay6_GPIO_Port,Relay6_Pin,GPIO_PIN_RESET)
#define	RELAY7_NC()	  HAL_GPIO_WritePin(Relay7_GPIO_Port,Relay7_Pin,GPIO_PIN_RESET)
#define	RELAY8_NC()	  HAL_GPIO_WritePin(Relay8_GPIO_Port,Relay8_Pin,GPIO_PIN_RESET)
#define	RELAY9_NC()  	HAL_GPIO_WritePin(Relay9_GPIO_Port,Relay9_Pin,GPIO_PIN_RESET)
#define	RELAY10_NC()	HAL_GPIO_WritePin(Relay10_GPIO_Port,Relay10_Pin,GPIO_PIN_RESET)
#define	RELAY11_NC()	HAL_GPIO_WritePin(Relay11_GPIO_Port,Relay11_Pin,GPIO_PIN_RESET)
#define	RELAY12_NC()	HAL_GPIO_WritePin(Relay12_GPIO_Port,Relay12_Pin,GPIO_PIN_RESET)
#define	RELAY13_NC()	HAL_GPIO_WritePin(Relay13_GPIO_Port,Relay13_Pin,GPIO_PIN_RESET)
#define	RELAY14_NC()	HAL_GPIO_WritePin(Relay14_GPIO_Port,Relay14_Pin,GPIO_PIN_RESET)
#define	RELAY15_NC()	HAL_GPIO_WritePin(Relay15_GPIO_Port,Relay15_Pin,GPIO_PIN_RESET)


// #define EE_ADDRx16_ISCANFD         0x01
// #define EE_ADDRx16_CANFREQDIV      0x02
// #define EE_ADDRx16_OVERSAMPLE      0x03
// #define EE_ADDRx16_CHVARIANT       0x04

/*! -------------------------------------------------------------------------- */
/*! Public function declarations */
void Relays_Start(void);

#endif
/*! end of the file */
