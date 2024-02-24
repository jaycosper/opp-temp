/*
 *===============================================================================
 *
 *                         OOOOOO
 *                       OOOOOOOOOO
 *      PPPPPPPPPPPPP   OOO      OOO   PPPPPPPPPPPPP
 *    PPPPPPPPPPPPPP   OOO        OOO   PPPPPPPPPPPPPP
 *   PPP         PPP   OOO        OOO   PPP         PPP
 *  PPP          PPP   OOO        OOO   PPP          PPP
 *  PPP          PPP   OOO        OOO   PPP          PPP
 *  PPP          PPP   OOO        OOO   PPP          PPP
 *   PPP         PPP   OOO        OOO   PPP         PPP
 *    PPPPPPPPPPPPPP   OOO        OOO   PPPPPPPPPPPPPP
 *     PPPPPPPPPPPPP   OOO        OOO   PPP
 *               PPP   OOO        OOO   PPP
 *               PPP   OOO        OOO   PPP
 *               PPP   OOO        OOO   PPP
 *               PPP    OOO      OOO    PPP
 *               PPP     OOOOOOOOOO     PPP
 *              PPPPP      OOOOOO      PPPPP
 *
 * @file:   booty.c
 * @author: Hugh Spahr
 * @date:   01/27/2024
 *
 * @note:   Open Pinball Project
 *          Copyright� 2024, Hugh Spahr
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *===============================================================================
 */
/**
 * This file contains the main code for the Booty bootloader.
 *
 *===============================================================================
 */
#include <stdint.h>
#include <stdbool.h>
#include "stm32_compat.h"

#define INSTANTIATE_PROC
#include "procdefs.h"

#define BOOTY_STX						0x02
#define BOOTY_CMD_WRITE_PROG_MEM		0x00
#define BOOTY_CMD_READ_PROG_MEM			0x01
#define BOOTY_CMD_GET_VERS				0x05
#define BOOTY_CMD_REBOOT				0x80

#define BOOTY_DATA_SZ                   8
#define BOOTY_MAX_TX_DATA_SZ            15
#define BOOTY_TX_STX_OFF                0
#define BOOTY_TX_CMD_OFF                1
#define BOOTY_TX_ADDR_START_OFF         2
#define BOOTY_TX_DATA_START_OFF         6
#define BOOTY_TX_CRC8_OFF               14

#define BOOTY_STATE_LOOK_FOR_STX        0
#define BOOTY_STATE_LOOK_FOR_CMD        1
#define BOOTY_STATE_RCV_ADDR            2
#define BOOTY_STATE_RCV_DATA            3
#define BOOTY_STATE_RCV_CRC8            4
#define BOOTY_STATE_CMD_RCVD            5

#define BOOTY_FLASH_SIZE                0x00010000
#define BOOTY_FLASH_ADDR                0x08000000
#define BOOTY_APP_START_ADDR            0x08001000
#define BOOTY_FLASH_SECT_SZ             0x400

uint8_t     bootyState = BOOTY_STATE_LOOK_FOR_STX;
uint32_t    rxAddr;
int         rxIndex;
uint8_t     txBuf[BOOTY_MAX_TX_DATA_SZ];

typedef struct
{
  uint32_t              *stack_p;         /* Initial stack pointer */
  void                  (*rstHndlr_fp)(); /* Address of reset handler */
  void                  *nmiHndlr_p;      /* Address of NMI handler */
  void                  *hrdFltHndlr_p;   /* Address of hard fault handler */
  void                  *memMngHndlr_p;   /* Address of memory manager handler */
  void                  *busFltHndlr_p;   /* Address of bus fault handler */
  void                  *useFltHndlr_p;   /* Address of usage fault handler */
  uint32_t              unused1;
  uint32_t              appVers;          /* Application version */
  uint32_t              appLen;           /* Length of application */
} APP_START_T;

APP_START_T* appStart_p = (APP_START_T *)BOOTY_APP_START_ADDR;

/* Prototype declarations */
void stdlflash_unlock_flash();
bool stdlflash_sector_erase(
   uint16_t                   *dest_p);   /* ptr to sector addr in flash */
bool stdlflash_write(
   uint8_t                    *src_p,     /* ptr to source of data */
   uint16_t                   *dest_p,    /* ptr to destination of data in flash */
   int                        numBytes);  /* number of bytes */

/*
 * ===============================================================================
 *
 * Name: booty_calc_crc8
 *
 * ===============================================================================
 */
/**
 * Function for calculating the CRC8 of a data stream
 *
 * Set the initial value of the CRC8 to 0xff, and run the CRC8 calculation using
 * a 16 entry lookup table.
 *
 * @param   crc8_p      [in]    Ptr to the CRC8.  Data should be 0xff initially
 * @param   length      [in]    Num chars in data stream
 * @param   data_p      [in]    Ptr to data stream
 * @return  None
 *
 * @pre     None
 * @note    Generator polynomial is x^8+x^2+x+1.  Initial value is 0xff to
 * conform to the ATM HEC CRC8
 *
 * ===============================================================================
 */
const uint8_t               BOOTY_CRC8_LOOKUP[16] =
  { 0x00, 0x07, 0x0e, 0x09, 0x1c, 0x1b, 0x12, 0x15,
    0x38, 0x3f, 0x36, 0x31, 0x24, 0x23, 0x2a, 0x2d };

void booty_calc_crc8(
  uint8_t                   *crc8_p,      /* Ptr to crc8 */
  int                       length,       /* Num chars in data stream */
  uint8_t                   *data_p)      /* Ptr to data stream */
{
  uint8_t                   currCrc;
  int                       count;

  for (count = 0, currCrc = *crc8_p; count < length; count++)
  {
    currCrc = ((currCrc << 4) & 0xf0) ^ BOOTY_CRC8_LOOKUP[((currCrc ^ data_p[count]) >> 4) & 0x0f];
    currCrc = ((currCrc << 4) & 0xf0) ^ BOOTY_CRC8_LOOKUP[(((currCrc >> 4) & 0x0f) ^ (data_p[count])) & 0x0f];
  }
  *crc8_p = currCrc;
} /* End booty_calc_crc8 */

/*
 * ===============================================================================
 *
 * Name: booty_calc_crc32
 *
 * ===============================================================================
 */
/**
 * Calculate the CRC32 data
 *
 * @param   data_p    [in]  ptr to data to be CRC'd
 * @param   length    [in]  length of data (in 32 bit words) to be CRC'd
 * @param   crc_p     [in/out] ptr to interim CRC value
 * @return  None
 *
 * @pre     None
 * @note    None
 *
 * ===============================================================================
 */
void booty_calc_crc32(
   uint32_t                *data_p,
   uint32_t                length,
   uint32_t                *crc_p)
{
   int                     index;

   // Enable CRC clock and reset CRC
   rccBase_p->AHBENR |= AHBENR_CRCEN;
   crcBase_p->CR = 0x00000001;

   for (index = 0; index < length; index++, data_p++)
   {
      crcBase_p->DR = *data_p;
   }
   *crc_p = crcBase_p->DR;
} /* End booty_calc_crc32 */

/*
 * ===============================================================================
 *
 * Name: booty_rxtx_proc
 *
 * ===============================================================================
 */
/**
 * Booty processing for rx/tx data
 *
 * @param   rxData_p  [in]  ptr to received data
 * @param   rxLen     [in]  number of received bytes
 * @param   txData_pp [out] ptr to ptr of data to transmit
 * @return  Number of bytes to transmit
 *
 * @pre     None
 * @note    None
 *
 * ===============================================================================
 */
int booty_rxtx_proc(
	uint8_t                 *rxData_p,
	int                     rxLen,
	uint8_t                 **txData_pp)
{
    uint8_t crc8;
    bool fail;
    int index;
    bool flashLocked = true;
    int txLen = 0;

	index = 0;
	while ((index < rxLen) && (bootyState != BOOTY_STATE_CMD_RCVD))
	{
		if (bootyState == BOOTY_STATE_LOOK_FOR_STX)
		{
			if (rxData_p[index] == BOOTY_STX)
			{
				txBuf[0] = BOOTY_STX;
				bootyState = BOOTY_STATE_LOOK_FOR_CMD;
			}
		}
		else if (bootyState == BOOTY_STATE_LOOK_FOR_CMD)
		{
			txBuf[BOOTY_TX_CMD_OFF] = rxData_p[index];
			if ((txBuf[BOOTY_TX_CMD_OFF] == BOOTY_CMD_GET_VERS) ||
				(txBuf[BOOTY_TX_CMD_OFF] == BOOTY_CMD_REBOOT))
			{
				bootyState = BOOTY_STATE_CMD_RCVD;
			}
			else if ((txBuf[BOOTY_TX_CMD_OFF] == BOOTY_CMD_WRITE_PROG_MEM) ||
				(txBuf[BOOTY_TX_CMD_OFF] == BOOTY_CMD_READ_PROG_MEM))
			{
				bootyState = BOOTY_STATE_RCV_ADDR;
				rxAddr = 0;
				rxIndex = 0;
			}
			else if (txBuf[BOOTY_TX_CMD_OFF] != BOOTY_STX)
			{
				bootyState = BOOTY_STATE_LOOK_FOR_STX;
			}
			/* If command was invalid and STX stay in BOOTY_STATE_LOOK_FOR_CMD */
		}
		else if (bootyState == BOOTY_STATE_RCV_ADDR)
		{
			txBuf[BOOTY_TX_ADDR_START_OFF + rxIndex] = rxData_p[index];
			rxAddr |= (uint32_t)rxData_p[index] << (8 * (3 - rxIndex++));
			if (rxIndex >= sizeof(rxAddr))
			{
    			if (txBuf[BOOTY_TX_CMD_OFF] == BOOTY_CMD_WRITE_PROG_MEM)
    			{
    				bootyState = BOOTY_STATE_RCV_DATA;
    				rxIndex = 0;
    			}
    			else
    			{
    				/* command is BOOTY_CMD_READ_PROG_MEM */
    				bootyState = BOOTY_STATE_CMD_RCVD;
    			}
			}
		}
		else if (bootyState == BOOTY_STATE_RCV_DATA)
		{
			txBuf[BOOTY_TX_DATA_START_OFF + rxIndex++] = rxData_p[index];
			if (rxIndex >= BOOTY_DATA_SZ)
			{
				bootyState = BOOTY_STATE_RCV_CRC8;
			}
		}
		else if (bootyState == BOOTY_STATE_RCV_CRC8)
		{
			/* Verify CRC8 */
			crc8 = 0xff;
			booty_calc_crc8(&crc8, 13, &txBuf[BOOTY_TX_CMD_OFF]);
			if (crc8 == rxData_p[index])
			{
				bootyState = BOOTY_STATE_CMD_RCVD;
			}
			else
			{
				/* Bad CRC8 so wait for next command */
				bootyState = BOOTY_STATE_LOOK_FOR_STX;
			}
		}
		else
		{
			/* Should never happen but reset state machine */
			bootyState = BOOTY_STATE_LOOK_FOR_STX;
		}
		index++;
	}
	if (bootyState == BOOTY_STATE_CMD_RCVD)
	{
		if (txBuf[BOOTY_TX_CMD_OFF] == BOOTY_CMD_GET_VERS)
		{
			*txData_pp = (uint8_t *)"Boot0.0\r";
			txLen = 8;
		}
		else if (txBuf[BOOTY_TX_CMD_OFF] == BOOTY_CMD_REBOOT)
		{
			/* Request a processor reset */
			scbBase_p->AIRCR = AIRCR_SYSRESETREQ;
		}
		else if (txBuf[BOOTY_TX_CMD_OFF] == BOOTY_CMD_WRITE_PROG_MEM)
		{
			if (flashLocked)
			{
				stdlflash_unlock_flash();
				flashLocked = false;
			}
			fail = false;
			if ((rxAddr & (BOOTY_FLASH_SECT_SZ - 1)) == 0)
			{
				fail = stdlflash_sector_erase((uint16_t *)rxAddr);
			}
			if (!fail)
			{
				fail = stdlflash_write(&txBuf[BOOTY_TX_DATA_START_OFF], (uint16_t *)rxAddr, 8);
			}
			if (!fail)
			{
				*txData_pp = &txBuf[0];
				txLen = 6;
			}
		}
		else if (txBuf[BOOTY_TX_CMD_OFF] == BOOTY_CMD_READ_PROG_MEM)
		{
			for (index = 0; index < BOOTY_DATA_SZ; index++)
			{
				/* Convert to big endian */
				txBuf[BOOTY_TX_DATA_START_OFF + (index & 0x4) +
					(3 - (index & 0x3))] = *(uint8_t *)(rxAddr + index);
			}
			crc8 = 0xff;
			booty_calc_crc8(&crc8, 13, &txBuf[BOOTY_TX_CMD_OFF]);
			txBuf[BOOTY_TX_CRC8_OFF] = crc8;
			*txData_pp = &txBuf[0];
			txLen = 15;
		}
		bootyState = BOOTY_STATE_LOOK_FOR_STX;
	}
	return (txLen);
}

/*
 * ===============================================================================
 *
 * Name: booty_verify_app
 *
 * ===============================================================================
 */
/**
 * Booty verify the application or stay in bootloader if bad.
 *
 * @return  None
 *
 * @pre     None
 * @note    This function does not return if it jumps to the application
 *
 * ===============================================================================
 */
void booty_verify_app()
{
    uint32_t appCrc;
    bool goodApp = false;

#define MAX_U32 0xffffffff

    /* Check for stay in boot condition, either PB2 held high during boot
     * or the first SRAM address set to 0xdeadbeef.
     */
    rccBase_p->APB2ENR = 0x00000008;
    if ((*(uint32_t *)SRAM_FIRST_ADDR == 0xdeadbeef) ||
      ((gpioBBase_p->IDR & 0x00000004) == 0x00000004))
    {
    	*(uint32_t *)SRAM_FIRST_ADDR = 0;
    }
    /* Verify appLen is valid, < 64K - bootSize and length divisible by 4 */
    else if ((appStart_p->appLen < (BOOTY_FLASH_SIZE - (BOOTY_APP_START_ADDR - BOOTY_FLASH_ADDR)) &&
      ((appStart_p->appLen & 0x00000003) == 0)))
    {
      booty_calc_crc32((uint32_t *)appStart_p, (appStart_p->appLen) >> 2, &appCrc);
      appCrc ^= MAX_U32;
      if (appCrc == *(uint32_t *)(BOOTY_APP_START_ADDR + appStart_p->appLen))
      {
        goodApp = true;
      }
    }
    if (goodApp)
    {
      /* Copy stack pointer and update VTOR.  Note:  setting the stack
       *   pointer is done using the bootloader's exceptions just in case
       *   the address is bogus.
       */
      __ASM volatile ("MSR msp, %0" : : "r" (appStart_p->stack_p) : );
      scbBase_p->VTOR = BOOTY_APP_START_ADDR;

      /* Jump to application reset handler, never returns from jump */
      ((APP_START_T *)BOOTY_APP_START_ADDR)->rstHndlr_fp();
    }
}

/*
 * ===============================================================================
 *
 * Name: booty_clock_init
 *
 * ===============================================================================
 */
/**
 * Minimal clock initialization for the USB bootloader
 *
 * @return  None
 *
 * @pre     None
 * @note    None
 *
 * ===============================================================================
 */
void booty_clock_init()
{
	flashBase_p->ACR = 0x00000012;  // Flash latency is 1 wait state
	rccBase_p->CFGR = 0x00680000;  // PLL * 12, OTG USB FS is 48MHz
	rccBase_p->CR = 0x01000083; // Turn PLL on
	while ((rccBase_p->CR & 0x02000000) == 0);  // Wait for PLL to be ready
	rccBase_p->CFGR = 0x00680002;  // PLL is system clock
	while ((rccBase_p->CFGR & 0x0000000c) != 0x00000008);  // Wait for system clock switch
}
