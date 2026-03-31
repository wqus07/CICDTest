/*
  ******************************************************************************
  * File Name          : spi_ad7606.h
  * Description        : This file provides code for the ad7606 driver using spi.
  * Author             : Jason TigerTE
  * Date               : 2023-09-14
  ******************************************************************************
*/
#ifndef __BASE_ID_GENERAL_API_H_
#define __BASE_ID_GENERAL_API_H_
#include "stdint.h"

/*! CAN Base ID  */
#define BordBaseID     (0x300 - 0x1)

#define BoardIDIndex   0


/*! Software Version Control */
#define SW_VERSION_YEAR       24
#define SW_VERSION_MONTH      10
#define SW_VERSION_DAY        31
#define SW_VERSION_VRIANT     0

#define CANID_CONFIG              (BordBaseID + BoardIDIndex * 0x100 + 0x0)
#define CANID_RELAY_ACTION        (BordBaseID + BoardIDIndex * 0x100 + 0x1)
#define CANID_RELAY_RESPONSE      (BordBaseID + BoardIDIndex * 0x100 + 0x2)

#define CANID_VERSION             (BordBaseID + BoardIDIndex * 0x100 + 0xF)


/*! -------------------------------------------------------------------------- */
/*! Public function declarations */
void M93C46_SPI2_Setup(void);
void M93C46_SPI2_Start(void);
void M93C46_SPI2_Stop(void);
void M93C46_SPI2_Clock_Pulse(void);
void M93C46_SPI2_Send_Bit(uint8_t aBit);
uint16_t M93C46_SPI2_Read(uint16_t address);
void M93C46_SPI2_Write(uint16_t address, uint16_t data);
void M93C46_Write_Enable(void);
void M93C46_Write_Disable(void);
void M93C46_Erase_Word(uint16_t address);
void M93C46_Erase_All(void);
void M93C46_Write_All(uint16_t data);

//delay function, 400MHz
void Delay1us(uint32_t usdelay);
void Delay1ms(uint32_t msdelay);

//CAN function
uint8_t FDCAN1_Send_Msg(uint32_t ST_ID, uint8_t* msg, uint32_t len, uint8_t Mode);
uint8_t FDCAN1_Filter(void);
#endif
/*! end of the file */
