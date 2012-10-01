/******************************************************************************
* DISCLAIMER
* This software is supplied by Renesas Electronics Corporation and is only
* intended for use with Renesas products. No other uses are authorized.
* This software is owned by Renesas Electronics Corporation and is protected under
* all applicable laws, including copyright laws.
* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES
* REGARDING THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY,
* INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
* PARTICULAR PURPOSE AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY
* DISCLAIMED.
* TO THE MAXIMUM EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS
* ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES SHALL BE LIABLE
* FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES
* FOR ANY REASON RELATED TO THIS SOFTWARE, EVEN IF RENESAS OR ITS
* AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* Renesas reserves the right, without notice, to make changes to this
* software and to discontinue the availability of this software.
* By using this software, you agree to the additional terms and
* conditions found by accessing the following link:
* http://www.renesas.com/disclaimer
******************************************************************************/
/* Copyright (C) 2011 Renesas Electronics Corporation. All rights reserved. */
/* Code written for Renesas by Future Designs, Inc. www.teamfdi.com */

/******************************************************************************
* File Name : GlyphOpenDrivers.c
* Version : 1.00
* Device(s) : Communications Drivers and LCD Display Drivers
* Tool-Chain : Glyph - The Generic API for Graphics RSPI API
* H/W Platform : YRDKRL78G13
* Description : Sets functions in the Handle.
******************************************************************************/

/******************************************************************************
Includes <system include> �Glyph Includes�
******************************************************************************/
#include "glyph_api.h"

/******************************************************************************
Includes �Glyph LCD Display (0) Include�
******************************************************************************/
#include "Drivers/ST7579_LCD.h"
//#include "Drivers/YRDKRL78_SPI.h"
#include "RDKRL78_spi.h"
#include "r_cg_macrodriver.h"

/******************************************************************************
* There is a built in SPI driver inside the RL78 RDK Glyph Library.
* The built in CSI10 driver sets the baud rate to 312500 bps and does not
* user interrupts.
*
* The default SPI driver can be overwritten. (See example below)
******************************************************************************/

/******************************************************************************
* Function Name: LCD_SPI_Open
* Description : Overwrites the default LCD_SPI_Open in the RL78 Glyph Library.
*   This routine initializes the port io settings and commands a Peripheral Reset
*   which includes the LCD. 
* Note: In the example it assumes there is a 12MHz clock. It is much better to
*   use the interval timer for these delays
******************************************************************************/
T_glyphError LCD_SPI_Open(T_glyphHandle aHandle)
{
    // Setup output pins
    P14 |= 1<<6;
    PM14 &= (uint8_t)~(1<<6);  // Set P146 (LCD RS) to output
    
    return GLYPH_ERROR_NONE;
}
/******************************************************************************
* Function Name: LCD_SPI_Open
* Description : Overwrites the default LCD_SPI_CommandSend in the RL78 Glyph 
*   Library. This routine sends a command byte to the LCD along with toggling 
*   the LCD Chip Select and LCD Register Select Pins
******************************************************************************/
void LCD_SPI_CommandSend(int8_t aCommand)
{
    P14 &= ~(1<<6);      // Assert P146 (LCD RS)
    
    SPI_Send(SPI_LCD, (uint8_t *)&aCommand, 1);
    
    P14 |= (1<<6);       // Deassert P146 (LCD RS)
}
/******************************************************************************
* Function Name: LCD_SPI_Open
* Description : Overwrites the default LCD_SPI_DataSend in the RL78 Glyph 
* Library. This routine sends a data byte to the LCD along with toggling the LCD
*   Chip Select Pin.
******************************************************************************/
void LCD_SPI_DataSend(int8_t aData)
{
    SPI_Send(SPI_LCD, (uint8_t *)&aData, 1);
}


/******************************************************************************
* Function Name: GlyphCommOpen
* Description : Assign the communications workspace.
* Argument : aHandle - the Glyph handle to setup for the LCD and Communications.
* Return Value : 0=success, not 0= error
* Calling Functions : main
******************************************************************************/
T_glyphError GlyphCommOpen(T_glyphHandle aHandle, int16_t aAddress)
{
    T_glyphWorkspace *p_gw = (T_glyphWorkspace *)aHandle;


    switch (aAddress) {
        case 0:
             p_gw->iCommAPI->iOpen        = LCD_SPI_Open;
             p_gw->iCommAPI->iCommandSend = LCD_SPI_CommandSend;
             p_gw->iCommAPI->iDataSend    = LCD_SPI_DataSend;
             break;


        default:
            return GLYPH_ERROR_ILLEGAL_OPERATION;
    }

    return p_gw->iCommAPI->iOpen(aHandle);
}

/******************************************************************************
* Function Name: GlyphLCDOpen
* Description : Assign the global ST7579_LCD to aHandle.
* Argument : aHandle - the Glyph handle to setup for the LCD.
* Return Value : 0=success, not 0= error
* Calling Functions : GlyphOpen
******************************************************************************/
T_glyphError GlyphLCDOpen(T_glyphHandle aHandle, int16_t aAddress)
{
    T_glyphWorkspace *p_gw = (T_glyphWorkspace *)aHandle;


    switch (aAddress) {
        case 0:
             p_gw->iLCDAPI->iOpen  = ST7579_Open;
             p_gw->iLCDAPI->iWrite = ST7579_Write;
             p_gw->iLCDAPI->iRead  = ST7579_Read;
             p_gw->iLCDAPI->iClose = ST7579_Close;
             break;


        default:
            return GLYPH_ERROR_ILLEGAL_OPERATION;
    }

    return GLYPH_ERROR_NONE;
}