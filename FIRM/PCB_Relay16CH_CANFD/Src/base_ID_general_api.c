/*
  ******************************************************************************
  * File Name          : ad7606.h
  * Description        : This file provides code for the ad7606 driver using spi.
  * Author             : Jason TigerTE
  * Date               : 2021-01-05
  ******************************************************************************
*/

#include "main.h"
#include "fdcan.h"
#include "tim.h"
#include "gpio.h"
#include "base_ID_general_api.h"

#define M93C46_SPI2_DO_High()  	HAL_GPIO_WritePin(SPI2_MOSI_GPIO_Port, SPI2_MOSI_Pin, GPIO_PIN_SET)
#define M93C46_SPI2_DO_Low()   	HAL_GPIO_WritePin(SPI2_MOSI_GPIO_Port, SPI2_MOSI_Pin, GPIO_PIN_RESET)
#define M93C46_SPI2_SK_High()  	HAL_GPIO_WritePin(SPI2_SCK_GPIO_Port, SPI2_SCK_Pin, GPIO_PIN_SET)
#define M93C46_SPI2_SK_Low()   	HAL_GPIO_WritePin(SPI2_SCK_GPIO_Port, SPI2_SCK_Pin, GPIO_PIN_RESET)
#define M93C46_SPI2_CS_High()  	HAL_GPIO_WritePin(SPI2_CS_GPIO_Port, SPI2_CS_Pin, GPIO_PIN_SET)
#define M93C46_SPI2_CS_Low()   	HAL_GPIO_WritePin(SPI2_CS_GPIO_Port, SPI2_CS_Pin, GPIO_PIN_RESET)
#define Read_M93C46_SPI2_DI()   HAL_GPIO_ReadPin(SPI2_MISO_GPIO_Port, SPI2_MISO_Pin)
#define M93C46_X16_ADDR_BITS 	6
#define SPI2_CLOCK_US	 		50

/*
  ******************************************************************************
                            CAN Genral Function
  ******************************************************************************
*/
/**
  * @brief CAN Filter Configuration
  * @param None
  * @retval None
  */
uint8_t FDCAN1_Filter(void) {
    FDCAN_FilterTypeDef FDCAN1_RXFilter;

    FDCAN1_RXFilter.IdType = FDCAN_STANDARD_ID;                       //standard ID
    FDCAN1_RXFilter.FilterIndex = 0;                                  //Filter Index                
    FDCAN1_RXFilter.FilterType = FDCAN_FILTER_MASK;                   //Filter Type
    FDCAN1_RXFilter.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;           //Filter 0 -> FIFO0
    FDCAN1_RXFilter.FilterID1 = 0x0000;                               //32bit ID
    FDCAN1_RXFilter.FilterID2 = 0x0000;                               //when classic mode, 32bit Mask
    if(HAL_FDCAN_ConfigFilter(&hfdcan1, &FDCAN1_RXFilter) != HAL_OK) {  //Filter Initilization
        return 1;
    }
    return 0;

}
/**
  * @brief CAN Send Function
  * @param ST_ID: CANID; msg:Message Buffer; len: data length; Mode: 0-classic can, 1-CANFD
  * @retval None
  */

uint8_t FDCAN1_Send_Msg(uint32_t ST_ID, uint8_t* msg, uint32_t len, uint8_t Mode) {
    FDCAN_TxHeaderTypeDef FCanTx;

    FCanTx.Identifier = ST_ID;                           //32bit ID
    FCanTx.IdType = FDCAN_EXTENDED_ID;                  //ID Type
    FCanTx.TxFrameType = FDCAN_DATA_FRAME;              //Frame Type
    FCanTx.DataLength = len;                            //Data Length
    FCanTx.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    FCanTx.BitRateSwitch = FDCAN_BRS_ON;               //BRS Switch on
    if(Mode == 0) {
        FCanTx.FDFormat = FDCAN_CLASSIC_CAN;                //Classic Mode
    }
    else {
        FCanTx.FDFormat = FDCAN_FD_CAN;
    }
    FCanTx.TxEventFifoControl = FDCAN_NO_TX_EVENTS;      //no send event
    FCanTx.MessageMarker = 0;

    if(HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &FCanTx, msg) != HAL_OK) {
        return 1;
    }
    return 0;

}

/*
  ******************************************************************************
                            EEPROM General Function
  ******************************************************************************
*/
/**
 * @brief Set up SPI2 interface for M93C46 EEPROM chip
 * Initializes SPI2 pins to default states.
*/
void M93C46_SPI2_Setup(void) {
    M93C46_SPI2_DO_Low();
    M93C46_SPI2_SK_Low();
    M93C46_SPI2_CS_Low();
    Delay1ms(1);
}

/**
 * @brief Start communication with M93C46 EEPROM chip over SPI2
 * Sets chip select line high to enable communication.
 * Adds a small delay to allow bus to initialize.
*/
void M93C46_SPI2_Start(void) {
    M93C46_SPI2_CS_High();
    Delay1us(SPI2_CLOCK_US);
    //Delay1ms(1);
}

/**
 * @brief Stop communication with M93C46 EEPROM chip over SPI2
 * Sets chip select line low to disable communication.
*/
void M93C46_SPI2_Stop(void) {
    M93C46_SPI2_CS_Low();
    Delay1us(SPI2_CLOCK_US);
    //Delay1ms(1);
}

/**
 * @brief Generate a clock pulse on the SPI2 bus to the M93C46 chip
 * Sets the clock line high, delays, sets it low, delays.
 * Used to clock in/out bits during SPI communication.
 */
void M93C46_SPI2_Clock_Pulse(void) {
    M93C46_SPI2_SK_Low();
    Delay1us(SPI2_CLOCK_US);
    M93C46_SPI2_SK_High();
    Delay1us(SPI2_CLOCK_US);
}

/**
 * Sends a single bit over the SPI2 bus to the M93C46 chip.
 * Sets the data line high or low based on the input bit.
 * Then generates a clock pulse to clock the data into the chip.
*/
void M93C46_SPI2_Send_Bit(uint8_t aBit) {
    if(aBit) M93C46_SPI2_DO_High();
    else M93C46_SPI2_DO_Low();
    //Delay1us(SPI2_CLOCK_US);
    M93C46_SPI2_Clock_Pulse();
}

/**
 * Reads a 16-bit data value from the specified address
 * in the M93C46 EEPROM chip using SPI2 communication.
 *
 * Sends a READ command and address, then clocks in
 * 16 data bits and returns them as an uint16_t.
 */
uint16_t M93C46_SPI2_Read(uint16_t address) {
    M93C46_SPI2_Start();

    // 发送READ指令 
    M93C46_SPI2_Send_Bit(1);
    M93C46_SPI2_Send_Bit(1);
    M93C46_SPI2_Send_Bit(0);

    // 发送地址（根据您的EEPROM大小调整位数）
    for(int i = (M93C46_X16_ADDR_BITS - 1); i >= 0; i--) {
        M93C46_SPI2_Send_Bit(address & (1 << i));
    }
    uint16_t data = 0;
    // 读取数据
    for(int i = 15; i >= 0; i--) {
        M93C46_SPI2_SK_Low();
        Delay1us(SPI2_CLOCK_US);
        M93C46_SPI2_SK_High();
        Delay1us(SPI2_CLOCK_US);
        if(Read_M93C46_SPI2_DI()) {
            data |= (1 << i);
        }

        //Delay1us(SPI2_CLOCK_US);
    }

    M93C46_SPI2_Stop();
    return data;
}

/**
 * Writes a 16-bit data value to the specified address
 * in the M93C46 EEPROM chip using SPI2 communication.
 *
 * Sends a WRITE command, address and data bits over SPI.
 *
 * @param address Memory location to write to
 * @param data 16-bit data value to write
 */
void M93C46_SPI2_Write_OP(uint16_t address, uint16_t data) {
    M93C46_SPI2_Start();

    // 发送Write指令 
    M93C46_SPI2_Send_Bit(1);
    M93C46_SPI2_Send_Bit(0);
    M93C46_SPI2_Send_Bit(1);

    // 发送地址（根据您的EEPROM大小调整位数），6bit
    for(int i = (M93C46_X16_ADDR_BITS - 1); i >= 0; i--) {
        M93C46_SPI2_Send_Bit(address & (1 << i));
    }

    // 发送数据（根据您的EEPROM大小调整位数），16bit，MSB first
    for(int i = 15; i >= 0; i--) {
        int b = data & (1 << i);
        if(b) M93C46_SPI2_Send_Bit(1);
        else M93C46_SPI2_Send_Bit(0);
        //M93C46_SPI2_Send_Bit(data & (1 << i));
    }
    M93C46_SPI2_Stop();
}

/**
 * Enables writing to the M93C46 EEPROM chip using SPI2.
 *
 * Sends a WEN (Write Enable) command over SPI2 to allow
 * writing to the EEPROM before a write operation.
 */
void M93C46_Write_Enable(void) {
    M93C46_SPI2_Start();

    // 发送WEN指令 
    M93C46_SPI2_Send_Bit(1);
    M93C46_SPI2_Send_Bit(0);
    M93C46_SPI2_Send_Bit(0);
    uint16_t address = 0x30;
    // 发送地址（根据您的EEPROM大小调整位数），6bit
    for(int i = (M93C46_X16_ADDR_BITS - 1); i >= 0; i--) {
        M93C46_SPI2_Send_Bit(address & (1 << i));
    }
    M93C46_SPI2_Stop();
}

/**
 * Disables write operations to the M93C46 EEPROM chip using SPI2.
 *
 * Sends a WDS (write disable) command over SPI to prevent
 * further writes to the EEPROM until the next WEN (write enable).
 */
void M93C46_Write_Disable(void) {
    M93C46_SPI2_Start();

    // 发送WDS指令 
    M93C46_SPI2_Send_Bit(1);
    M93C46_SPI2_Send_Bit(0);
    M93C46_SPI2_Send_Bit(0);
    uint16_t address = 0x00;
    // 发送地址（根据您的EEPROM大小调整位数），6bit
    for(int i = (M93C46_X16_ADDR_BITS - 1); i >= 0; i--) {
        M93C46_SPI2_Send_Bit(address & (1 << i));
    }
    M93C46_SPI2_Stop();
}

/**
 * Erases a word at the given EEPROM address.
 *
 * Sends an ERASE command over the SPI bus to erase a
 * 16-bit word at the specified address. The address is
 * sent bit-by-bit, MSB first, matching the EEPROM's
 * address size.
 *
 * @param address EEPROM address to erase
 */
void M93C46_Erase_Word(uint16_t address) {
    M93C46_SPI2_Start();

    // 发送ERASE指令 
    M93C46_SPI2_Send_Bit(1);
    M93C46_SPI2_Send_Bit(1);
    M93C46_SPI2_Send_Bit(1);
    // 发送地址（根据您的EEPROM大小调整位数），6bit
    for(int i = (M93C46_X16_ADDR_BITS - 1); i >= 0; i--) {
        M93C46_SPI2_Send_Bit(address & (1 << i));
    }
    M93C46_SPI2_Stop();
}


/**
 * Erases all memory in the 93C46 EEPROM by sending the ERAL command.
 * Sends the erase all command and address over the SPI interface.
*/
void M93C46_Erase_All(void) {
    M93C46_SPI2_Start();

    // 发送ERAL指令 
    M93C46_SPI2_Send_Bit(1);
    M93C46_SPI2_Send_Bit(0);
    M93C46_SPI2_Send_Bit(0);
    uint16_t address = 0x20;
    // 发送地址（根据您的EEPROM大小调整位数），6bit
    for(int i = (M93C46_X16_ADDR_BITS - 1); i >= 0; i--) {
        M93C46_SPI2_Send_Bit(address & (1 << i));
    }
    M93C46_SPI2_Stop();
}


/**
 * Writes 16-bit data to the specified EEPROM address
 * using the SPI2 interface. Performs write enable,
 * write operation, and write disable.
 *
 * @param address EEPROM address to write to
 * @param data 16-bit data to write
 */

 /**
  * Initializes the EEPROM SPI2 interface, reads
  * initial data, writes test data, and reads back.
  */
void M93C46_SPI2_Write(uint16_t address, uint16_t data) {
    // Enable Write
    M93C46_Write_Enable();
    Delay1ms(1);
    // Write Data
    M93C46_SPI2_Write_OP(address, data);
    Delay1ms(1);
    //Disable Write
    M93C46_Write_Disable();
    Delay1ms(1);
}

void SW_EEPROM_Init(void) {
    M93C46_SPI2_Setup();
    uint16_t data = M93C46_SPI2_Read(0x00);
    M93C46_SPI2_Read(0x00);
    M93C46_SPI2_Write(0x00, 0x5566);
}

/**
 * Write data to all memory locations.
 *
 * This function writes the given 16-bit data value to all locations in the
 * EEPROM memory. It uses the WRAL command to indicate a write-all operation,
 * followed by the memory address and data to write.
 */
void M93C46_Write_All(uint16_t data) {
    M93C46_SPI2_Start();

    // 发送WRAL指令 
    M93C46_SPI2_Send_Bit(1);
    M93C46_SPI2_Send_Bit(0);
    M93C46_SPI2_Send_Bit(0);
    uint16_t address = 0x10;
    // 发送地址（根据您的EEPROM大小调整位数），6bit
    for(int i = (M93C46_X16_ADDR_BITS - 1); i >= 0; i--) {
        M93C46_SPI2_Send_Bit(address & (1 << i));
    }
    // 发送数据（根据您的EEPROM大小调整位数），16bit，MSB first
    for(int i = 15; i >= 0; i--) {
        if(data & (1 << i)) M93C46_SPI2_Send_Bit(1);
        else M93C46_SPI2_Send_Bit(0);
        //M93C46_SPI2_Send_Bit(data & (1 << i));
    }
    M93C46_SPI2_Stop();
}

/*
  ******************************************************************************
                            Delay General Function
  ******************************************************************************
*/

/**
 * Delay for the given number of microseconds.
 *
 * This uses a simple busy-wait loop to delay for the specified number of
 * microseconds. The delay time is approximated using NOP operations.
 *
 * @param usdelay Number of microseconds to delay
 */
void Delay1us(uint32_t usdelay) {
    uint32_t Delay = usdelay * 24u;
    do {
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();

    }
    while(Delay--);
}


/**
 * Delay for the given number of milliseconds.
 *
 * This uses Delay1us() to delay for the specified number
 * of milliseconds. The delay time is approximated by
 * calling Delay1us() 1000 times per millisecond.
 *
 * @param msdelay Number of milliseconds to delay
 */
void Delay1ms(uint32_t msdelay) {
    uint32_t Delay = msdelay;
    do {
        Delay1us(1000);
    }
    while(Delay--);
}
/*! end of the file */
