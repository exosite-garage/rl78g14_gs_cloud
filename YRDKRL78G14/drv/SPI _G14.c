/*-------------------------------------------------------------------------*
 * File:  SPI.c
 *-------------------------------------------------------------------------*
 * Description:
 *     SPI interrupt based driver for the RL78's CSI10 peripheral.
 *-------------------------------------------------------------------------*/

//******************************************************************************
//  GS1011 SPI - slave
//  RL78 SPI - master
//
//  //* VCC must be at 3.3 V for RL78 & GS1011 *//
//
//                                RL78
//                               -----------------
//                          /|\ |               X1|-
//                           |  |                 |
//                            --|RST            X2|-
//                              |                 |
//                              |                 |
//      GS1011                  |                 |
//    ----------       |-SDCard-|P30              |
//   |           |     |-WIFI---|P73              |
//   |    SSPI_CS|<-CS-|-LCD----|P10              |
//   |           |     |-PMOD1--|P71              |
//   |           |     |-PMOD2--|P72              |
//   |           |              |                 |
//   |   SSPI_CLK|<---CLK-------|P04/SCK10        |
//   |  SSPI_DOUT|----MISO----->|P03/SI10         |
//   |   SSPI_DIN|<---MOSI------|P02/SO10         |
//   |     GPIO28|----WAKEUP--->|P05              |
//   |     GPIO29|<---PRGMODE---|PE6              |
//   |  EXT_RESET|<-------------|RSTOUT#          | (optional)
//   |        GND|--------------|GND              |
//
//  CSI10 Configuration:
//  - Single Transfer mode
//  - MSB
//  - Type 4 data timing
//  - Transfer rate is set using SPI_Init(uint32_t bitsPerSecond)
//      note: This will set the fClk divider for SAU0 which may effect other
//              channels on SAU0. This can be avoided by making sure the bit rate
//              inputed into SPI_Init shares the same best case PClk divide
//              as other channels.
//******************************************************************************

/*-------------------------------------------------------------------------*
 * Includes:
 *-------------------------------------------------------------------------*/
#include <string.h>
#include <system/platform.h>
#include "SAU.h"
#include "SPI.h"

/*-------------------------------------------------------------------------*
 * Constants:
 *-------------------------------------------------------------------------*/
#ifndef SPI_RX_INT_PRIORITY
#define SPI_RX_INT_PRIORITY       1U
#endif

#ifndef SPI_TX_INT_PRIORITY
#define SPI_TX_INT_PRIORITY       1U
#endif

/*-------------------------------------------------------------------------*
 * Globals:
 *-------------------------------------------------------------------------*/
static bool G_SPI_IsBusy;
static const uint8_t *G_SPI_SendBuffer;
static uint8_t *G_SPI_ReceiveBuffer;
static uint32_t G_SPI_SendIndex;
static uint32_t G_SPI_SendLength;
static uint32_t G_SPI_ReceiveIndex;
static uint8_t G_SPI_Channel;
static void (*G_SPI_Callback)(void);
static uint8_t G_SPI_OverrunErrorCount;

/* CPI Chip Select Polarity Array - indexed by channel #
   Set in SPI_SetupChannel(..) */
bool G_SPI_CSActiveHigh[SPI_NUM_CHANNELS];

/* CPI Chip Select Length - indexed by channel #
   If this is true for the channel, the CS will be toggled each byte
   Otherwise, the CS will stay active for an entire transfer */
bool G_SPI_CSActivePerByte[SPI_NUM_CHANNELS];

/* RL78RDK SPI Chip Select Port Pointer Array - indexed by channel # */
unsigned char *SPI_CS_P[] = {
    (unsigned char *)&P3, // SD-CS     P30
    (unsigned char *)&P5, // WIFI-CS   P73     G14 -> P55
    (unsigned char *)&P14, // LCD-CS    P10    G14 -> P145
    (unsigned char *)&P7, // PMOD1-CS  P71     
    (unsigned char *)&P7  // PMOD2-CS  P72
};

/* RL78 RDK SPI Chip Select Port Mode Pointer Array - indexed by channel # */
unsigned char *SPI_CS_PM[] = {
    (unsigned char *)&PM3, // SD-CS     P30
    (unsigned char *)&PM5, // WIFI-CS   P73    G14 -> P55
    (unsigned char *)&PM14, // LCD-CS    P10  G14 -> P145
    (unsigned char *)&PM7, // PMOD1-CS  P71
    (unsigned char *)&PM7  // PMOD2-CS  P72
};

/* RL78 RDK SPI Chip Select Pin Array - indexed by channel # */
uint8_t SPI_CS_Pin[] = { 
    0, // SD-CS     P30
    5, // WIFI-CS   P73      G14 -> P55
    5, // LCD-CS    P10      G14 -> P145
    1, // PMOD1-CS  P71
    2  // PMOD2-CS  P72
};

/*---------------------------------------------------------------------------*
 * Routine:  SPI_DisableInterrupts
 *---------------------------------------------------------------------------*
 * Description:
 *      Disable SPI interrupts for the RL78 CSI10 peripheral
 * Inputs:
 *      void
 * Outputs:
 *      void
 *---------------------------------------------------------------------------*/
void SPI_DisableInterrupts(void)
{   
    CSIMK31 = 1U;    /* disable INTCSI10 interrupt */
    CSIIF31 = 0U;    /* clear INTCSI10 interrupt flag */
}

/*---------------------------------------------------------------------------*
 * Routine:  SPI_DisableInterrupts
 *---------------------------------------------------------------------------*
 * Description:
 *      Enable SPI interrupts for the RL78 CSI10 peripheral
 * Inputs:
 *      void
 * Outputs:
 *      void
 *---------------------------------------------------------------------------*/
void SPI_EnableInterrupts(void)
{
    CSIIF31 = 0U;    /* clear INTCSI10 interrupt flag */
    CSIMK31 = 0U;    /* enable INTCSI10 */
}

/*---------------------------------------------------------------------------*
 * Routine:  SPI_SetBitRate
 *---------------------------------------------------------------------------*
 * Description:
 *      Set the I2C speed in kHz.
 * Inputs:
 *      uint16_t aSpeed -- kHz speed of I2C bus.
 * Outputs:
 *      void
 *---------------------------------------------------------------------------*/
void SPI_SetBitRate(uint32_t bitsPerSecond)
{
    uint16_t fCLK_devisor;
    uint32_t baud_devisor;
    
    /*  Calculate division ratio of the operation clock to be stored in bits 15:9 
    of the SDRmn register. */
    
    baud_devisor = ((RL78_MAIN_SYSTEM_CLOCK / bitsPerSecond / 2) - 1);
    
    /* increase the fCLK devisor each time the baud rate is divided until it fits */
    for(fCLK_devisor = 0; fCLK_devisor<12; fCLK_devisor++)
    {
        /* check if baud_devisor is greater than 7 bits */
        if(baud_devisor > 127)
        {
            baud_devisor = baud_devisor/2;
        }
        else
        {
            SPS1  = fCLK_devisor & 0xF;   /* Serial clock select register */
            SDR13 = baud_devisor<<9;
            break;
        }
    }
}

/*---------------------------------------------------------------------------*
 * Routine:  SPI_Init
 *---------------------------------------------------------------------------*
 * Description:
 *      Configure the SPI driver for the RL78 CSI10.  Configure the bit
 *      rate.
 * Inputs:
 *      uint32_t bitsPerSecond -- bits per second clock rate (Hz)
 * Outputs:
 *      void
 *---------------------------------------------------------------------------*/
void SPI_Init(uint32_t bitsPerSecond)
{
        G_SPI_IsBusy = false;

 //   SAU1EN = 1U;    /* supply SAU0 clock */
    NOP();
    NOP();
    NOP();
    NOP();
    
     ST1 |= _0008_SAU_CH3_STOP_TRG_ON;  /* disable CSI10 */
    
    SPI_DisableInterrupts();
    
    /* Set INTCSI10 low priority */
    CSIPR131 = SPI_RX_INT_PRIORITY;
    CSIPR031 =SPI_TX_INT_PRIORITY;
    
     SIR13 = _0004_SAU_SIRMN_FECTMN | _0002_SAU_SIRMN_PECTMN | _0001_SAU_SIRMN_OVCTMN;    /* clear error flag */
    SMR13 = _0020_SAU_SMRMN_INITIALVALUE | _8000_SAU_CLOCK_SELECT_CK01 | _0000_SAU_CLOCK_MODE_CKS |
            _0000_SAU_TRIGGER_SOFTWARE | _0000_SAU_MODE_CSI | _0000_SAU_TRANSFER_END;
    SCR13 = _C000_SAU_RECEPTION_TRANSMISSION | _3000_SAU_TIMING_4 | _0000_SAU_MSB | _0007_SAU_LENGTH_8;
    
	SO1 &= ~_0800_SAU_CH3_CLOCK_OUTPUT_1;    /* CSI31 clock initial level */
    SO1 &= ~_0008_SAU_CH3_DATA_OUTPUT_1;    /* CSI31 SO initial level */
    SOE1 |= _0008_SAU_CH3_OUTPUT_ENABLE;    /* enable CSI31 output */
    
    SPI_SetBitRate(bitsPerSecond);
    
   /* Set SI31 pin */
    PM5 |= 0x08U;
    /* Set SO31 pin */
    P5 |= 0x04U;
    PM5 &= 0xFBU;
    /* Set SCK31 pin */
    P5 |= 0x10U;
    PM5 &= 0xEFU;
    
    /* Start CSI10 module operation */
    SPI_EnableInterrupts();
     SO1 &= ~_0800_SAU_CH3_CLOCK_OUTPUT_1;   /* CSI31 clock initial level */
    SO1 &= ~_0008_SAU_CH3_DATA_OUTPUT_1;           /* CSI31 SO initial level */
    SOE1 |= _0008_SAU_CH3_OUTPUT_ENABLE;           /* enable CSI31 output */
    SS1 |= _0008_SAU_CH3_START_TRG_ON;             /* enable CSI31 */
        /* enable CSI10 */
}

/*---------------------------------------------------------------------------*
 * Routine:  SPI_CS_Assert
 *---------------------------------------------------------------------------*
 * Description:
 *      
 * Inputs:
 *      uint8_t channel -- Channel for the RDK peripheral
 * Outputs:
 *      void
 *---------------------------------------------------------------------------*/
void SPI_CS_Assert(uint8_t channel)
{
    if(G_SPI_CSActiveHigh[channel]) {
        /* Set CS High */
        *SPI_CS_P[channel] |= (1<<SPI_CS_Pin[channel]);
    }
    else {
        /* Set CS Low */
	    *SPI_CS_P[channel] &= ~(1<<SPI_CS_Pin[channel]);
    }
}

/*---------------------------------------------------------------------------*
 * Routine:  SPI_CS_Clear
 *---------------------------------------------------------------------------*
 * Description:
 *      
 * Inputs:
 *      uint8_t channel -- Channel for the RDK peripheral
 * Outputs:
 *      void
 *---------------------------------------------------------------------------*/
void SPI_CS_Clear(uint8_t channel)
{
    if(G_SPI_CSActiveHigh[channel]) {
        /* Set CS Low */
	    *SPI_CS_P[channel] &= ~(1<<SPI_CS_Pin[channel]);
    }
    else {
        /* Set CS High */
        *SPI_CS_P[channel] |= (1<<SPI_CS_Pin[channel]);
    }
}

/*---------------------------------------------------------------------------*
 * Routine:  SPI_ChannelSetup
 *---------------------------------------------------------------------------*
 * Description:
 *      Configure the channel to be either active high or low.
 * Inputs:
 *      uint8_t channel -- Channel for the RDK peripheral
 *      bool csActiveHigh -- true if active high, else false for active low
 *      bool csActivePerByte -- true if the CS is toggled every byte
 *                           -- false if the CS is active for an entire transfer
 * Outputs:
 *      void
 *---------------------------------------------------------------------------*/
void SPI_ChannelSetup(uint8_t channel, bool csActiveHigh, bool csActivePerByte)
{
    /* Setup the CS polarity */
    G_SPI_CSActiveHigh[channel] = csActiveHigh;
    G_SPI_CSActivePerByte[channel] = csActivePerByte;
    
    SPI_CS_Clear(channel);
    
    /* Set CS pin to output */
    *SPI_CS_PM[channel] &= ~(1<<SPI_CS_Pin[channel]);
}

/*---------------------------------------------------------------------------*
 * Routine:  SPI_Transfer
 *---------------------------------------------------------------------------*
 * Description:
 *      Attempt to transfer an array of bytes over the SPI channel given.
 *      When complete, the given callback routine is called.
 * Inputs:
 *      uint8_t channel -- Channel for the RDK peripheral
 *      uint32_t numBytes -- Number of bytes to transfer
 *      const uint8_t *send_buffer -- Pointer to bytes to send
 *      uint8_t *receive_buffer -- Pointer to buffer to receive SPI bytes.
 *          Can be the same location as the send_buffer.
 *      void (*callback)(void) -- Callback function to call when complete.
 * Outputs:
 *      bool -- true if successfully started, else false (busy).
 *---------------------------------------------------------------------------*/
bool SPI_Transfer(
        uint8_t channel,
        uint32_t numBytes,
        const uint8_t *send_buffer,
        uint8_t *receive_buffer,
        void(*callback)(void))
{
    if (G_SPI_IsBusy)
        return false;
    
    G_SPI_IsBusy = true;

    G_SPI_SendBuffer = send_buffer;
    G_SPI_ReceiveBuffer = receive_buffer;
    G_SPI_Callback = callback;
    G_SPI_SendIndex = 0;
    G_SPI_ReceiveIndex = 0;
    G_SPI_SendLength = numBytes;
    G_SPI_Channel = channel;
    
    SPI_DisableInterrupts();
    
    SPI_CS_Assert(G_SPI_Channel);
    
    /* started by writing data to SDR[7:0] */
    SIO31 = G_SPI_SendBuffer[G_SPI_SendIndex++];   // SIO10 G14 -> SIO31
    G_SPI_SendLength--;
    
    SPI_EnableInterrupts();
    
    return true;
}

/*---------------------------------------------------------------------------*
 * Routine:  SPI_IsBusy
 *---------------------------------------------------------------------------*
 * Description:
 *      Returns true if the RSPI peripheral is busy.
 * Inputs:
 *      uint8_t channel -- Channel for the RDK peripheral
 *      uint32_t numBytes -- Number of bytes to transfer
 *      const uint8_t *send_buffer -- Pointer to bytes to send
 *      uint8_t *receive_buffer -- Pointer to buffer to receive SPI bytes.
 *          Can be the same location as the send_buffer.
 *      void (*callback)(void) -- Callback function to call when complete.
 * Outputs:
 *      bool -- true if successfully started, else false (busy).
 *---------------------------------------------------------------------------*/
bool SPI_IsBusy(uint8_t channel)
{
    return G_SPI_IsBusy;
}

/*---------------------------------------------------------------------------*
 * Routine:  SPI_ISRHandler
 *---------------------------------------------------------------------------*
 * Description:
 *      Interrupt service routine for completed receptions (occurs after
 *      transmits).  Bytes are put in the receive buffer as they come in.
 * Inputs:
 *      void
 * Outputs:
 *      void
 *---------------------------------------------------------------------------*/
#pragma vector = INTCSI31_vect         //  G14  INTCSI10_vect  -> INTCSI31_vect
__interrupt static void SPI_ISRHandler(void)
{
    uint8_t err_type;

    err_type = (uint8_t)(SSR13 & _SAU_OVERRUN_ERROR);
    SIR13 = (uint16_t)err_type;

    SPI_CS_Clear(G_SPI_Channel);
    
    if (1U == err_type) {
        /* overrun error occurs */
        G_SPI_OverrunErrorCount++;
    }
    else {
      
        /* Receive a character */
        G_SPI_ReceiveBuffer[G_SPI_ReceiveIndex++] = SIO31;
    
        /* Is there more data to send? */
        if (G_SPI_SendLength > 0) {
           
            /* Send the next character */
            SPI_CS_Assert(G_SPI_Channel);
            SIO31 = G_SPI_SendBuffer[G_SPI_SendIndex++];
            G_SPI_SendLength--;
        }
        else {
            /* Data transfer complete */
            SPI_CS_Clear(G_SPI_Channel);
            
            if (G_SPI_Callback)
                G_SPI_Callback();
            
            G_SPI_IsBusy = false;
        }
    }
}
void writeREG_SIO31( uint8_t data)
{
   P5 &= ~(1<<5); // WIFI-SPI-CS = LOW
   SIO31 = data;
   P5 |= (1<<5); // WIFI-SPI-CS = HIGH
 /*
   while(1)
   {
     MSTimerDelay(4);
     P5 &= ~(1<<5); // WIFI-SPI-CS = LOW
     MSTimerDelay(4);
      P5 |= (1<<5); // WIFI-SPI-CS = HIGH
   }
  */
}
/*-------------------------------------------------------------------------*
 * End of File:  SPI.c
 *-------------------------------------------------------------------------*/
