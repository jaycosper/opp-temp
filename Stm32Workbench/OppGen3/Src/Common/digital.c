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
 * @file:   digital.c
 * @author: Hugh Spahr
 * @date:   12/02/2012
 *
 * @note:   Open Pinball Project
 *          Copyright� 2012-2019, Hugh Spahr
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
 * This file controls the digital wing boards (inputs/solenoids).  It is
 * adapted from digital.c in the previous 1003-InpDrv\Sources\digital.c
 *
 *===============================================================================
 */
 
#include <stdlib.h>
#include "stdtypes.h"
#include "stdlintf.h"
#include "rs232intf.h"
#include "fadeintf.h"
#include "gen2glob.h"
#include "procdefs.h"         /* for EnableInterrupts macro */

#include "stm32f1xx_hal.h"

#define STDL_FILE_ID          3

typedef struct
{
   GPIO_TypeDef               *port_p;
   uint16_t                   GPIO_Pin;
} DIG_PIN_INFO_T;

const DIG_PIN_INFO_T dig_pinInfo[RS232I_NUM_GEN2_INP] =
   {  { GPIOA, GPIO_PIN_13 }, { GPIOB, GPIO_PIN_12 },
      { GPIOB, GPIO_PIN_13 }, { GPIOB, GPIO_PIN_14 },
      { GPIOB, GPIO_PIN_15 }, { GPIOA, GPIO_PIN_8 },
      { GPIOA, GPIO_PIN_9 }, { GPIOA, GPIO_PIN_10 },

      { GPIOA, GPIO_PIN_15 }, { GPIOB, GPIO_PIN_3 },
      { GPIOB, GPIO_PIN_4 }, { GPIOB, GPIO_PIN_5 },
      { GPIOB, GPIO_PIN_6 }, { GPIOB, GPIO_PIN_7 },
      { GPIOB, GPIO_PIN_8 }, { GPIOB, GPIO_PIN_9 },

      { GPIOA, GPIO_PIN_14 }, { GPIOC, GPIO_PIN_13 },
      { GPIOC, GPIO_PIN_14 }, { GPIOC, GPIO_PIN_15 },
      { GPIOA, GPIO_PIN_0 }, { GPIOA, GPIO_PIN_1 },
      { GPIOA, GPIO_PIN_2 }, { GPIOA, GPIO_PIN_3 },

      { GPIOA, GPIO_PIN_4 }, { GPIOA, GPIO_PIN_5 },
      { GPIOA, GPIO_PIN_6 }, { GPIOA, GPIO_PIN_7 },
      { GPIOB, GPIO_PIN_0 }, { GPIOB, GPIO_PIN_1 },
      { GPIOB, GPIO_PIN_10 }, { GPIOB, GPIO_PIN_11 } };

typedef enum
{
   SOL_STATE_IDLE             = 0x00,
   SOL_INITIAL_KICK           = 0x01,
   SOL_SUSTAIN_PWM            = 0x02,
   SOL_MIN_TIME_OFF           = 0x03,
   SOL_WAIT_BEFORE_KICK       = 0x04,
   SOL_FULL_ON_SOLENOID       = 0x05,
} __attribute__((packed)) DIG_SOL_STATE_E;

typedef struct
{
   BOOL                       clearRcvd;
   DIG_SOL_STATE_E            solState;
   U8                         offCnt;
   U16                        intenDur;
   U16                        pwmIntenDur;
   INT                        bit;
   INT                        startMs;
   U32                        inpBits;
} DIG_SOL_STATE_T;

typedef struct
{
   INT                        cnt;
} DIG_INP_STATE_T;

#define MATRIX_FIRE_SOL       0x80
#define MATRIX_SOL_MASK       0x07
#define MATRIX_WAIT_CNT_THRESH   2
#define MATRIX_DEBOUNCE_THRESH   4
#define MATRIX_COL_OFFS       16
#define MATRIX_COL_MASK       0x00ff0000

typedef struct
{
   U8                         cnt;
   U8                         sol;
} DIG_MTRX_BIT_INFO_T;

typedef struct
{
   U8                         column;
   U8                         waitCnt;
   U8                         mtrxWaitCntThresh;
   U8                         mtrxDebounceThresh;
   BOOL                       mtrxActHigh;
   DIG_MTRX_BIT_INFO_T        info[RS232I_SW_MATRX_INP];
} DIG_MATRIX_DATA_T;

typedef struct
{
   U16                        port;
   U16                        msk;
} DIG_PORT_DATA_T;

typedef struct
{
   U16                        solMask;
   U32                        prevInputs;
   U32                        filtInputs;
   U32                        mtrxInpMask;
   U32                        stateMask;
   U32                        outputUpd;
   U32                        outputMask;
   DIG_PORT_DATA_T            updPort[STDLI_NUM_DIG_PORT];
   DIG_MATRIX_DATA_T          mtrxData;
   DIG_INP_STATE_T            inpState[RS232I_NUM_GEN2_INP];
   DIG_SOL_STATE_T            solState[RS232I_NUM_GEN2_SOL];
} DIG_GLOB_T;

DIG_GLOB_T                    dig_info;

/* Prototypes */
INT timer_get_ms_count();
U16 timer_get_us_count();
void digital_set_solenoid_input(
   RS232I_SET_SOL_INP_E       inputIndex,
   U8                         solIndex);
void digital_set_kick_pwm(
   U8                         kickPwm,
   U8                         solIndex);
void digital_upd_sol_cfg(
   U16                        updMask);
void digital_upd_inp_cfg(
   U32                        updMask);
void digital_upd_outputs(
   U32                        value,
   U32                        mask);

/*
 * ===============================================================================
 * 
 * Name: digital_init
 * 
 * ===============================================================================
 */
/**
 * Initialize the digital I/O port
 * 
 * Initialize digital I/O port, and the other digital control signals.
 * 
 * @param   None 
 * @return  None
 * 
 * @pre     None 
 * @note    None
 * 
 * ===============================================================================
 */
void digital_init(void) 
{
   DIG_SOL_STATE_T            *solState_p;
   INT                        index;
   BOOL                       foundSol = FALSE;
   RS232I_CFG_INP_TYPE_E      *inpCfg_p;
   RS232I_SOL_CFG_T           *solCfg_p;
   U32                        outputMask = 0;
   GPIO_InitTypeDef           pinCfg;
   BOOL                       usedBit;
   INT                        input;

#define INPUT_BIT_MASK        0xff
#define DBG_INPUT_BIT_MASK    0xfe
#define INCAND_OUTP_MASK      0xff
#define DBG_INCAND_OUTP_MASK  0xfe
#define NEO_INP_BIT_MASK      0xef
#define DBG_NEO_INP_BIT_MASK  0xee
#define NEO_OUT_BIT_MASK      0x10
#define NEO_SOL_INP_BIT_MASK  0x0e
#define NEO_SOL_OUT_BIT_MASK  0xf1
#define DBG_NEO_SOL_OUT_BIT_MASK  0xf0
#define SOL_MASK              0x0f
#define DBG_NEO_SOL_MASK      0x0e
#define SOL_INP_BIT_MASK      0x0f  
#define DBG_SOL_INP_BIT_MASK  0x0e
#define SOL_OUTP_BIT_MASK     0xf0

#define MTRX_INPUT_BIT_MASK   0xff000000
#define MTRX_OUTPUT_BIT_MASK  0x00ff0000
#define DBG_MTRX_OUT_BIT_MASK 0x00fe0000
   
   if (gen2g_info.inpCfg_p == NULL)
   {
      for (solState_p = &dig_info.solState[0], index = 0;
         index < RS232I_NUM_GEN2_SOL; index++, solState_p++)
      {
         solState_p->solState = SOL_STATE_IDLE;
         solState_p->bit = 1 << (((index >> 2) << 3) + (index & 0x03) + 4);
         solState_p->inpBits = 0;
         solState_p->intenDur = 1000;
      }
      dig_info.solMask = 0;
      dig_info.filtInputs = 0;
      dig_info.mtrxInpMask = 0;
     
      /* Set the location of the input configuration data */
      gen2g_info.inpCfg_p = (GEN2G_INP_CFG_T *)gen2g_info.freeCfg_p;
      gen2g_info.freeCfg_p += sizeof(GEN2G_INP_CFG_T);

      /* Set up digital ports, walk through wing boards */
      for (index = 0; index < RS232I_NUM_WING; index++)
      {
         /* Check if this wing board is a input driver */
         if (gen2g_info.nvCfgInfo.wingCfg[index] == WING_INP)
         {
#if GEN2G_DEBUG_PORT == 0
             /* Set up bit mask of valid inputs */
        	 gen2g_info.inpMask |= (INPUT_BIT_MASK << (index << 3));
#else
             if ((index == 0) || (index == 2))
             {
            	 gen2g_info.inpMask |= (DBG_INPUT_BIT_MASK << (index << 3));
             }
             else
             {
            	 gen2g_info.inpMask |= (INPUT_BIT_MASK << (index << 3));
             }
#endif
             /* Check if there are any servo outputs, since pins 8 to 16 are inputs,
              *  they can be specially configured as servo outputs
              */
             if (index == 1)
             {
                 for (input = GEN2G_SERVO_FIRST_INDX;
                    input < GEN2G_SERVO_FIRST_INDX + GEN2G_SERVO_NUM_INP_WING_SERVO; input++)
                 {
                	if (gen2g_info.inpCfg_p->inpCfg[input] >= SERVO_OUTPUT_THRESH)
                	{
                		gen2g_info.inpMask &= ~(1 << input);
                		gen2g_info.servoMask |= (1 << (input - GEN2G_SERVO_FIRST_INDX));
                		outputMask |= (1 << input);
                	}
                 }
             }
         }
         /* Check if this wing board is a solenoid driver */
         else if (gen2g_info.nvCfgInfo.wingCfg[index] == WING_SOL)
         {
            foundSol = TRUE;
            
            outputMask |= (SOL_OUTP_BIT_MASK << (index << 3));
            dig_info.solMask |= (SOL_MASK << (index << 2));
            
#if GEN2G_DEBUG_PORT == 0
            /* Set up bit mask of valid inputs */
            gen2g_info.inpMask |= (SOL_INP_BIT_MASK << (index << 3));
#else
             if ((index == 0) || (index == 2))
             {
            	 gen2g_info.inpMask |= (DBG_SOL_INP_BIT_MASK << (index << 3));
             }
             else
             {
            	 gen2g_info.inpMask |= (SOL_INP_BIT_MASK << (index << 3));
             }
#endif
             /* Check if there are any servo outputs, since pins 8 to 16 are inputs,
              *  they can be specially configured as servo outputs
              */
             if (index == 1)
             {
                 for (input = GEN2G_SERVO_FIRST_INDX;
                    input < GEN2G_SERVO_FIRST_INDX + GEN2G_SERVO_NUM_SOL_WING_SERVO; input++)
                 {
                	if (gen2g_info.inpCfg_p->inpCfg[input] >= SERVO_OUTPUT_THRESH)
                	{
                		gen2g_info.inpMask &= ~(1 << input);
                		gen2g_info.servoMask |= (1 << (input - GEN2G_SERVO_FIRST_INDX));
                		outputMask |= (1 << input);
                	}
                 }
             }
         }
         /* Check if wing 2 is WING_SW_MATRIX_OUT, and wing 3 is WING_SW_MATRIX_IN.
          *   Other positions are not allowed.
          */
         else if ((gen2g_info.nvCfgInfo.wingCfg[index] == WING_SW_MATRIX_OUT) ||
            (gen2g_info.nvCfgInfo.wingCfg[index] == WING_SW_MATRIX_OUT_LOW))
         {
            if ((index == 2) && (gen2g_info.nvCfgInfo.wingCfg[3] == WING_SW_MATRIX_IN))
            {
               dig_info.mtrxData.column = 0;
               dig_info.mtrxData.waitCnt = 0;
#if GEN2G_DEBUG_PORT == 0
               outputMask |= MTRX_OUTPUT_BIT_MASK;
#else
               outputMask |= DBG_MTRX_OUT_BIT_MASK;
#endif
               dig_info.mtrxInpMask |= MTRX_INPUT_BIT_MASK;
               if (gen2g_info.nvCfgInfo.wingCfg[index] == WING_SW_MATRIX_OUT)
               {
            	   gen2g_info.switchMtrxActHigh = TRUE;
               }
            }
            else
            {
               gen2g_info.error = ERR_SW_MATRIX_WING_BAD_LOC;
            }
         }
         /* Neo wing can only be wing 0 */
         else if (gen2g_info.nvCfgInfo.wingCfg[index] == WING_NEO)
         {
            if (index == 0)
            {
#if GEN2G_DEBUG_PORT == 0
               gen2g_info.inpMask |= NEO_INP_BIT_MASK;
#else
               gen2g_info.inpMask |= DBG_NEO_INP_BIT_MASK;
#endif
               outputMask |= NEO_OUT_BIT_MASK;
            }
            else
            {
               gen2g_info.error = ERR_NEO_WING_BAD_LOC;
            }
         }
         /* NeoSol wing can only be wing 0 */
         else if (gen2g_info.nvCfgInfo.wingCfg[index] == WING_NEO_SOL)
         {
            if (index == 0)
            {
               foundSol = TRUE;
               gen2g_info.inpMask |= NEO_SOL_INP_BIT_MASK;
#if GEN2G_DEBUG_PORT == 0
               dig_info.solMask |= SOL_MASK;
               outputMask |= NEO_SOL_OUT_BIT_MASK;
#else
               dig_info.solMask |= DBG_NEO_SOL_MASK;
               outputMask |= DBG_NEO_SOL_OUT_BIT_MASK;
#endif
               dig_info.solState[0].bit = 1;
            }
            else
            {
               gen2g_info.error = ERR_NEO_WING_BAD_LOC;
            }
         }
         else if ((gen2g_info.nvCfgInfo.wingCfg[index] == WING_INCAND) ||
            (gen2g_info.nvCfgInfo.wingCfg[index] == WING_HI_SIDE_INCAND) ||
			(gen2g_info.nvCfgInfo.wingCfg[index] == WING_LAMP_MATRIX_COL) ||
			(gen2g_info.nvCfgInfo.wingCfg[index] == WING_LAMP_MATRIX_ROW))
         {
            outputMask |= (INCAND_OUTP_MASK << (index << 3));
#if GEN2G_DEBUG_PORT == 0
            outputMask |= (INCAND_OUTP_MASK << (index << 3));
#else
            if ((index == 0) || (index == 2))
            {
               outputMask |= (DBG_INCAND_OUTP_MASK << (index << 3));
            }
            else
            {
               outputMask |= (INCAND_OUTP_MASK << (index << 3));
            }
#endif
         }
      }
      for (index = 0; index < RS232I_NUM_GEN2_INP; index++)
      {
    	  usedBit = FALSE;
    	  pinCfg.Pin = dig_pinInfo[index].GPIO_Pin;
          if ((gen2g_info.inpMask & (1 << index)) != 0)
          {
        	  pinCfg.Mode = GPIO_MODE_INPUT;
              pinCfg.Pull = GPIO_PULLUP;
        	  usedBit = TRUE;
          }
          else if ((dig_info.mtrxInpMask & (1 << index)) != 0)
          {
        	  pinCfg.Mode = GPIO_MODE_INPUT;
              if (gen2g_info.switchMtrxActHigh)
              {
                 pinCfg.Pull = GPIO_PULLDOWN;
              }
              else
              {
                 pinCfg.Pull = GPIO_PULLUP;
              }
        	  usedBit = TRUE;
          }
          else if ((outputMask & (1 << index)) != 0)
          {
              /* Check if servo output which uses alternate function */
              pinCfg.Speed = GPIO_SPEED_FREQ_LOW;
              if ((index >= GEN2G_SERVO_FIRST_INDX) &&
                 (index < GEN2G_SERVO_FIRST_INDX + GEN2G_SERVO_NUM_INP_WING_SERVO) &&
				 (gen2g_info.servoMask & 1 << (index - GEN2G_SERVO_FIRST_INDX)))
              {
                 pinCfg.Mode = GPIO_MODE_AF_PP;
              }
              else
              {
                 pinCfg.Mode = GPIO_MODE_OUTPUT_PP;
              }
        	  usedBit = TRUE;
          }
          if (usedBit)
          {
             HAL_GPIO_Init(dig_pinInfo[index].port_p, &pinCfg);
          }
      }
      dig_info.filtInputs = stdldigio_read_all_ports(gen2g_info.inpMask | dig_info.mtrxInpMask);

      /* Setup GPB2 as output for status (available on STM32F103CB boards) */
      pinCfg.Pin = GPIO_PIN_2;
      pinCfg.Mode = GPIO_MODE_OUTPUT_PP;
      pinCfg.Speed = GPIO_SPEED_FREQ_LOW;
      HAL_GPIO_Init(GPIOB, &pinCfg);
      
      /* Set the location of solenoid configuration data if it exists */
      if (foundSol)
      {
         gen2g_info.solDrvCfg_p = (GEN2G_SOL_DRV_CFG_T *)gen2g_info.freeCfg_p;
         gen2g_info.freeCfg_p += sizeof(GEN2G_SOL_DRV_CFG_T);
      }
      
      /* Set up initial configuration of solenoids from NV config */
      for (index = 0; index < RS232I_NUM_WING; index++)
      {
         if (gen2g_info.nvCfgInfo.wingCfg[index] == WING_SOL)
         {
            /* Configure the inputs for the solenoids */
            for (inpCfg_p = &gen2g_info.inpCfg_p->inpCfg[index << 3];
               inpCfg_p < &gen2g_info.inpCfg_p->inpCfg[(index << 3) + 8];
               inpCfg_p++)
            {
               if (*inpCfg_p < SERVO_OUTPUT_THRESH)
               {
            	   *inpCfg_p = STATE_INPUT;
               }
            }
         }
      }

      /* If a matrix wing exists, set up the solenoid inputs */
      if ((gen2g_info.typeWingBrds & (1 << WING_SW_MATRIX_IN)) != 0)
      {
         /* Grab mtrxWaitCntThresh from config if set */
         if (gen2g_info.inpCfg_p->inpCfg[RS232I_MTRX_WAIT_THRESH_INDX] == 0)
         {
            dig_info.mtrxData.mtrxWaitCntThresh = MATRIX_WAIT_CNT_THRESH;
         }
         else
         {
            dig_info.mtrxData.mtrxWaitCntThresh = gen2g_info.inpCfg_p->inpCfg[RS232I_MTRX_WAIT_THRESH_INDX];
         }

         /* Grab mtrxDebounceThresh from config if set */
         if (gen2g_info.inpCfg_p->inpCfg[RS232I_MTRX_DEBOUNCE_THRESH_INDX] == 0)
         {
            dig_info.mtrxData.mtrxDebounceThresh = MATRIX_DEBOUNCE_THRESH;
         }
         else
         {
            dig_info.mtrxData.mtrxDebounceThresh = gen2g_info.inpCfg_p->inpCfg[RS232I_MTRX_DEBOUNCE_THRESH_INDX];
         }


         /* Initialize the counts and solenoid info */
         for (index = 0; index < RS232I_SW_MATRX_INP; index++)
         {
            dig_info.mtrxData.info[index].cnt = 0;
            dig_info.mtrxData.info[index].sol = 0;
         }
         /* Set up the solenoids to autofire using the switch matrix */
         for (index = 0, solCfg_p = &gen2g_info.solDrvCfg_p->solCfg[0];
            index < RS232I_NUM_GEN2_SOL; index++, solCfg_p++)
         {
            if (solCfg_p->cfg & USE_MATRIX_INP)
            {
               dig_info.mtrxData.info[solCfg_p->minOffDuty].sol = MATRIX_FIRE_SOL | index;
            }
         }
         /* Initialize matrix input for rs232 reports */
         if (!gen2g_info.switchMtrxActHigh)
         {
            for (index = 0; index < RS232I_MATRX_COL; index++)
            {
               gen2g_info.matrixInp[index] = 0xff;
            }
         }
      }
      
      /* Set up the initial state */
      digital_upd_sol_cfg((1 << RS232I_NUM_GEN2_SOL) - 1);
      digital_upd_inp_cfg(gen2g_info.inpMask);
   }

} /* End digital_init */

/*
 * ===============================================================================
 * 
 * Name: digital_task
 * 
 * ===============================================================================
 */
/**
 * Task for polling inputs
 * 
 * @param   None 
 * @return  None
 * 
 * @pre     None 
 * @note    None
 * 
 * ===============================================================================
 */
void digital_task(void)
{
   DIG_INP_STATE_T            *inpState_p;
   DIG_SOL_STATE_T            *solState_p;
   RS232I_SOL_CFG_T           *solCfg_p;
   DIG_MTRX_BIT_INFO_T        *matrixBitInfo_p;
   U32                        inputs = 0;
   U32                        changedBits;
   U32                        updFilterHi;
   U32                        updFilterLow = 0;
   INT                        index;
   U8                         data;
   U8                         chngU8;
   RS232I_CFG_INP_TYPE_E      cfg;
   U32                        currBit;
   INT                        elapsedTimeMs;
   U16                        currUsTime;

#define SWITCH_THRESH         16
#define PWM_PERIOD            16
#define MIN_OFF_INC           0x10
   
   if (gen2g_info.validCfg)
   {
      /* Grab the inputs */
      inputs = stdldigio_read_all_ports(gen2g_info.inpMask);
      if ((gen2g_info.typeWingBrds & ((1 << WING_INP) | (1 << WING_SOL))) != 0)
      {
         /* See what bits have changed */
         changedBits = (dig_info.prevInputs ^ inputs) & gen2g_info.inpMask;
         updFilterHi = 0;
         updFilterLow = 0;
         
         /* Perform input processing for both input and solenoid boards */
         for (index = 0, currBit = 1, inpState_p = &dig_info.inpState[0];
            index < RS232I_NUM_GEN2_INP; index++, currBit <<= 1, inpState_p++)
         {
            if (currBit & gen2g_info.inpMask)
            {
               /* Check if this count has changed */
               if (changedBits & currBit)
               {
                  inpState_p->cnt = 0;
               }
               else
               {
                  if (inpState_p->cnt <= SWITCH_THRESH)
                  {
                     inpState_p->cnt++;
                  }
                  if (inpState_p->cnt == SWITCH_THRESH)
                  {
                     cfg = gen2g_info.inpCfg_p->inpCfg[index];
                     if (inputs & currBit)
                     {
                        updFilterHi |= currBit;
                        dig_info.filtInputs |= currBit;
                        if (cfg == RISE_EDGE)
                        {
                           DisableInterrupts;
                           gen2g_info.validSwitch |= currBit;
                           EnableInterrupts;
                           gen2g_info.inpTimestamp[index] = (U16)timer_get_ms_count();
                        }
                     }
                     else
                     {
                        updFilterLow |= currBit;
                        dig_info.filtInputs &= ~currBit;
                        if (cfg == FALL_EDGE)
                        {
                           DisableInterrupts;
                           gen2g_info.validSwitch |= currBit;
                           EnableInterrupts;
                           gen2g_info.inpTimestamp[index] = (U16)timer_get_ms_count();
                        }
                     }
                  }
               }
            }
         }
      }

      if ((gen2g_info.typeWingBrds & (1 << WING_SW_MATRIX_IN)) != 0)
      {
         if (dig_info.mtrxData.waitCnt >= dig_info.mtrxData.mtrxWaitCntThresh)
         {
            dig_info.mtrxData.waitCnt = 0;
    	    data = (U8)(stdldigio_read_all_ports(dig_info.mtrxInpMask) >> 24);
            chngU8 = data ^ gen2g_info.matrixPrev[dig_info.mtrxData.column];
            gen2g_info.matrixPrev[dig_info.mtrxData.column] = data;
            matrixBitInfo_p = &dig_info.mtrxData.info[dig_info.mtrxData.column * 8];
            for (index = 0, currBit = 1; index < 8; index++, currBit <<= 1, matrixBitInfo_p++)
            {
               /* If bit has changed, reset count */
               if ((currBit & chngU8) != 0)
               {
                  matrixBitInfo_p->cnt = 0;
               }
               else
               {
                  if (matrixBitInfo_p->cnt <= dig_info.mtrxData.mtrxDebounceThresh)
                  {
                     matrixBitInfo_p->cnt++;
                  }
                  /* Just passed the threshold so update data bit */
                  if (matrixBitInfo_p->cnt == dig_info.mtrxData.mtrxDebounceThresh)
                  {
                     if (((data & currBit) && gen2g_info.switchMtrxActHigh) ||
                       (((data & currBit) == 0) && !gen2g_info.switchMtrxActHigh))
                     {
                        /* Set or clear bit depending if active high or low */
                        if (gen2g_info.switchMtrxActHigh)
                        {
                           gen2g_info.matrixInp[dig_info.mtrxData.column] |= currBit;
                        }
                        else
                        {
                           gen2g_info.matrixInp[dig_info.mtrxData.column] &= ~currBit;
                        }
                        if (matrixBitInfo_p->sol != 0)
                        {
                           gen2g_info.solDrvProcCtl |=
                              (1 << (matrixBitInfo_p->sol & MATRIX_SOL_MASK));
                        }
                     }
                  }
               }
            }
         }

         if (dig_info.mtrxData.waitCnt == 0)
         {
            /* Move to next column */
            dig_info.mtrxData.column++;
            if (dig_info.mtrxData.column >= RS232I_MATRX_COL)
            {
               dig_info.mtrxData.column = 0;
            }
         
#if GEN2G_DEBUG_PORT == 0
            dig_info.outputMask |= MTRX_OUTPUT_BIT_MASK;
#else
            dig_info.outputMask |= DBG_MTRX_OUT_BIT_MASK;
#endif
            /* Reverse column numbering to match Bally documentation */
            if (gen2g_info.switchMtrxActHigh)
            {
               dig_info.outputUpd |= (1 << (MATRIX_COL_OFFS + RS232I_MATRX_COL - 1 - dig_info.mtrxData.column));
            }
            else
            {
               dig_info.outputUpd |= ((~(1 << (MATRIX_COL_OFFS + RS232I_MATRX_COL - 1 - dig_info.mtrxData.column))) &
                  MATRIX_COL_MASK);
            }
         }
         dig_info.mtrxData.waitCnt++;
      }
      
      if ((gen2g_info.typeWingBrds & (1 << WING_SOL)) != 0)
      {
         /* Perform solenoid processing */
         currUsTime = timer_get_us_count();
         for (index = 0, currBit = 1, solState_p = &dig_info.solState[0],
            solCfg_p = &gen2g_info.solDrvCfg_p->solCfg[0];
            index < RS232I_NUM_GEN2_SOL; index++, currBit <<= 1, solState_p++, solCfg_p++)
         {
            /* Update sol output bits every time */
            if (solState_p->solState != SOL_STATE_IDLE)
            {
               dig_info.outputMask |= solState_p->bit;
            }

            /* Check if processor is requesting a kick, or an input changed */
            if ((solState_p->solState == SOL_STATE_IDLE) &&
               ((gen2g_info.solDrvProcCtl & currBit) ||
               (updFilterLow & solState_p->inpBits)))
            {
               /* Check if processor is kicking normal solenoid */
               dig_info.outputMask |= solState_p->bit;
               if ((solCfg_p->cfg & (ON_OFF_SOL | DLY_KICK_SOL)) == 0)
               {
                  /* Start the solenoid kick */
                  solState_p->solState = SOL_INITIAL_KICK;
                  solState_p->startMs = timer_get_ms_count();
                  dig_info.outputUpd |= solState_p->bit;
               }
               else if ((solCfg_p->cfg & ON_OFF_SOL) != 0)
               {
                  solState_p->solState = SOL_FULL_ON_SOLENOID;
                  dig_info.outputUpd |= solState_p->bit;
               }
               else if ((solCfg_p->cfg & DLY_KICK_SOL) != 0)
               {
                  solState_p->solState = SOL_WAIT_BEFORE_KICK;
                  solState_p->startMs = timer_get_ms_count();
               }
               if ((solCfg_p->cfg & AUTO_CLR) &&
                  (gen2g_info.solDrvProcCtl & currBit))
               {
                  gen2g_info.solDrvProcCtl &= ~currBit;
                  solState_p->clearRcvd = TRUE;
               }
               else
               {
                  solState_p->clearRcvd = FALSE;
               }
            }
            else if (solState_p->solState == SOL_INITIAL_KICK)
            {
               if ((solCfg_p->cfg & CAN_CANCEL) != 0)
               {
                  if (((gen2g_info.solDrvProcCtl & currBit) == 0) &&
                     ((solState_p->inpBits == 0) ||
                     ((solState_p->inpBits & inputs) == solState_p->inpBits)))
                  {
                     /* Switch is inactive, move to idle */
                     solState_p->solState = SOL_STATE_IDLE;
                  }
               }
            
               if (solState_p->solState == SOL_INITIAL_KICK)
               {
                  /* Check if elapsed time is over initial kick time */
                  elapsedTimeMs = timer_get_ms_count() - solState_p->startMs;
                  if (elapsedTimeMs >= solCfg_p->initKick)
                  {
                     /* If this is a normal solenoid */
                     if ((solCfg_p->cfg & (ON_OFF_SOL | DLY_KICK_SOL | USE_MATRIX_INP)) == 0)
                     {
                        /* See if this has a sustaining PWM */
                        if (solCfg_p->minOffDuty & DUTY_CYCLE_MASK)
                        {
                           /* Make sure the input continues to be set */
                           if (solState_p->clearRcvd)
                           {
                              solState_p->solState = SOL_MIN_TIME_OFF;
                              solState_p->offCnt = 0;
                           }
                           else
                           {
                              /* Grab pwmIntenDur */
                              solState_p->solState = SOL_SUSTAIN_PWM;
                              solState_p->pwmIntenDur =
                                 FADE_USEC_DUR[(solCfg_p->minOffDuty & DUTY_CYCLE_MASK) << 1];
                              if (currUsTime <= solState_p->pwmIntenDur)
                              {
                                 dig_info.outputUpd |= solState_p->bit;
                              }
                           }
                        }
                        else
                        {
                           solState_p->solState = SOL_MIN_TIME_OFF;
                           solState_p->offCnt = 0;
                        }
                     }
                     else if ((solCfg_p->cfg & (DLY_KICK_SOL | USE_MATRIX_INP)) != 0)
                     {
                        solState_p->solState = SOL_MIN_TIME_OFF;
                        solState_p->offCnt = 0;
                     }
                     solState_p->startMs = timer_get_ms_count();
                  }
                  else if (currUsTime <= solState_p->intenDur)
                  {
                     dig_info.outputUpd |= solState_p->bit;
                  }
               }
            }
            else if (solState_p->solState == SOL_SUSTAIN_PWM)
            {
               if (((gen2g_info.solDrvProcCtl & currBit) == 0) &&
			     ((solState_p->inpBits == 0) ||
                 ((solState_p->inpBits & inputs) == solState_p->inpBits)))
               {
                  solState_p->clearRcvd = TRUE;
               }
               if (!solState_p->clearRcvd)
               {
                  /* Do faster PWM function by testing us timer */
                  if (currUsTime <= solState_p->pwmIntenDur)
                  {
                     dig_info.outputUpd |= solState_p->bit;
                  }
               }
               else
               {
                  /* Switch is inactive, move to idle */
                  solState_p->solState = SOL_STATE_IDLE;
               }
            }
            else if (solState_p->solState == SOL_MIN_TIME_OFF)
            {
               /* Check if an off time increment has happened */
               elapsedTimeMs = timer_get_ms_count() - solState_p->startMs;
               if (elapsedTimeMs >= solCfg_p->initKick)
               {
                  solState_p->offCnt += MIN_OFF_INC;
                  if (solState_p->offCnt >= (solCfg_p->minOffDuty & MIN_OFF_MASK))
                  {
                     solState_p->solState = SOL_STATE_IDLE;
                  }
                  else
                  {
                     solState_p->startMs = timer_get_ms_count();
                  }
               }
            }
            else if (solState_p->solState == SOL_WAIT_BEFORE_KICK)
            {
               /* Check if elapsed time is over the wait time
                * (stored in duty cycle nibble * 2)
                */
               elapsedTimeMs = timer_get_ms_count() - solState_p->startMs;
               if (elapsedTimeMs >= ((solCfg_p->minOffDuty & DUTY_CYCLE_MASK) << 1))
               {
                  /* Start the solenoid kick */
                  solState_p->solState = SOL_INITIAL_KICK;
                  solState_p->startMs = timer_get_ms_count();
                  dig_info.outputUpd |= solState_p->bit;
               }
            }
            else if (solState_p->solState == SOL_FULL_ON_SOLENOID)
            {
               if (((gen2g_info.solDrvProcCtl & currBit) == 0) &&
			     ((solState_p->inpBits == 0) ||
                 ((solState_p->inpBits & inputs) == solState_p->inpBits)))
               {
                  /* Switch is inactive, move to idle */
                  solState_p->solState = SOL_STATE_IDLE;
               }
               else if (currUsTime <= solState_p->intenDur)
               {
                  dig_info.outputUpd |= solState_p->bit;
               }
            }
         }
      }

      if ((gen2g_info.typeWingBrds & ((1 << WING_INP) | (1 << WING_SOL))) != 0)
      {
         DisableInterrupts;
         gen2g_info.validSwitch = (gen2g_info.validSwitch & ~dig_info.stateMask) |
            (dig_info.filtInputs & dig_info.stateMask);
         EnableInterrupts;
         dig_info.prevInputs = inputs;
      }
   }
} /* End digital_task */

/*
 * ===============================================================================
 * 
 * Name: digital_set_solenoid_input
 * 
 * ===============================================================================
 */
/**
 * Set a solenoid input
 *
 * Set a solenoid input.  To disable a solenoid, the input solenoid number can be
 * set to SOL_INP_CLEAR_SOL.
 * 
 * @param   None 
 * @return  None
 * 
 * @pre     None 
 * @note    If multiple inputs are used to fire a solenoid, the inputs are
 *    logically OR'd together.
 * 
 * ===============================================================================
 */
void digital_set_solenoid_input(
   U8                         inpIndex,
   RS232I_SET_SOL_INP_E       solIndex)
{
   DIG_SOL_STATE_T            *solState_p;
   
   if (inpIndex < RS232I_NUM_GEN2_INP)
   {
      solState_p = &dig_info.solState[solIndex & SOL_INP_SOL_MASK];
      if ((solIndex & SOL_INP_CLEAR_SOL) == 0)
      {
         solState_p->inpBits |= (1 << inpIndex);
      }
      else
      {
         solState_p->inpBits &= ~(1 << inpIndex);
      }
   }
   else if ((inpIndex - RS232I_NUM_GEN2_INP) < RS232I_SW_MATRX_INP)
   {
      /* Inputs 32 to 96 are from the switch matrix, first verify
       * there is a switch matrix.
       */
      if ((gen2g_info.typeWingBrds & (1 << WING_SW_MATRIX_IN)) != 0)
      {
         if (solIndex & SOL_INP_CLEAR_SOL)
         {
            dig_info.mtrxData.info[inpIndex - RS232I_NUM_GEN2_INP].sol = 0;
         }
         else
         {
            dig_info.mtrxData.info[inpIndex - RS232I_NUM_GEN2_INP].sol =
               MATRIX_FIRE_SOL | (solIndex & SOL_INP_SOL_MASK);
         }
      }
   }
} /* End digital_set_solenoid_input */

/*
 * ===============================================================================
 * 
 * Name: digital_set_kick_pwm
 *
 * ===============================================================================
 */
/**
 * Set kick PWM
 *
 * Set a solenoid's power during the kick portion.  PWM is a number from 0-32
 * where 0 is off, and 32 is 100% power.
 *
 * @param   kickPwm value from 0-32
 * @param   solIndex index of solenoid (0-15)
 * @return  None
 *
 * @pre     None
 *
 * ===============================================================================
 */
void digital_set_kick_pwm(
   U8                         kickPwm,
   U8                         solIndex)
{
   if (solIndex < RS232I_NUM_GEN2_SOL)
   {
      dig_info.solState[solIndex].intenDur = FADE_USEC_DUR[kickPwm];
   }
} /* End digital_set_kick_pwm */

/*
 * ===============================================================================
 *
 * Name: digital_upd_sol_cfg
 * 
 * ===============================================================================
 */
/**
 * Update solenoid configuration
 *
 * Update a solenoid configuration.
 * 
 * @param   updMask - Mask of solenoids to be updated. 
 * @return  None
 * 
 * @pre     None 
 * @note    None
 * 
 * ===============================================================================
 */
void digital_upd_sol_cfg(
   U16                        updMask)
{
   INT                        index;
   INT                        currBit;
   DIG_SOL_STATE_T            *solState_p;
   RS232I_SOL_CFG_T           *solDrvCfg_p;
   DIG_INP_STATE_T            *inpState_p;
   U32                        inputBit;
   
   /* Clear the solenoid state machines */
   updMask &= dig_info.solMask;
   for (index = 0, solState_p = &dig_info.solState[0], currBit = 1;
      index < RS232I_NUM_GEN2_SOL; index++, solState_p++, currBit <<= 1)
   {
      if ((updMask & currBit) != 0)
      {
         solState_p->solState = SOL_STATE_IDLE;
         dig_info.outputMask |= solState_p->bit;
         solDrvCfg_p = &gen2g_info.solDrvCfg_p->solCfg[index];
         if (solDrvCfg_p->cfg & USE_SWITCH)
         {
            inpState_p = &dig_info.inpState[((index & 0x0c) << 1) + (index & 0x03)];
            inpState_p->cnt = 0;

            /* Don't set certain input bits for NeoSol and SPI clock if configured */
            if ((currBit & gen2g_info.disSolInp) == 0)
            {
               inputBit = (1 << (((index & 0x0c) << 1) + (index & 0x03)));
               if (inputBit & gen2g_info.inpMask)
               {
                  solState_p->inpBits |= inputBit;
               }
            }
         }
         else
         {
            solState_p->inpBits &= ~(1 << (((index & 0x0c) << 1) + (index & 0x03)));
         }
      }
   }
} /* End digital_upd_sol_cfg */

/*
 * ===============================================================================
 * 
 * Name: digital_upd_inp_cfg
 * 
 * ===============================================================================
 */
/**
 * Update input configuration
 *
 * Update an input configuration.
 * 
 * @param   updMask - Mask of inputs to be updated. 
 * @return  None
 * 
 * @pre     None 
 * @note    None
 * 
 * ===============================================================================
 */
void digital_upd_inp_cfg(
   U32                        updMask)
{
   INT                        index;
   INT                        currBit;
   DIG_INP_STATE_T            *inpState_p;
   RS232I_CFG_INP_TYPE_E      *inpCfg_p;
   
   /* Clear the input counts */
   for (index = 0, inpState_p = &dig_info.inpState[0],
      inpCfg_p = &gen2g_info.inpCfg_p->inpCfg[0], currBit = 1;
      index < RS232I_NUM_GEN2_INP;
      index++, inpState_p++, inpCfg_p++, currBit <<= 1)
   {
      if ((updMask & currBit) != 0)
      {
         inpState_p->cnt = 0;
         if (*inpCfg_p < SERVO_OUTPUT_THRESH)
         {
             if (*inpCfg_p == STATE_INPUT)
             {
                dig_info.stateMask |= currBit;
             }
             else
             {
                dig_info.stateMask &= ~currBit;
             }
         }
      }
   }
} /* End digital_upd_inp_cfg */

/*
 * ===============================================================================
 *
 * Name: digital_upd_outputs
 *
 * ===============================================================================
 */
/**
 * Update outputs
 *
 * Update output bits in temporary registers.
 *
 * @param   value - Value of outputs to be updated.
 * @param   mask - Mask of outputs to be updated.
 * @return  None
 *
 * @pre     None
 * @note    None
 *
 * ===============================================================================
 */
void digital_upd_outputs(
   U32                        value,
   U32                        mask)
{
   dig_info.outputUpd |= value;
   dig_info.outputMask |= mask;
} /* End digital_upd_outputs */

/*
 * ===============================================================================
 *
 * Name: digital_write_outputs
 *
 * ===============================================================================
 */
/**
 * Write outputs to pins
 *
 * Write cached output updates to pins.
 *
 * @param   None
 * @return  None
 *
 * @pre     None
 * @note    None
 *
 * ===============================================================================
 */
void digital_write_outputs()
{
   stdldigio_write_all_ports(dig_info.outputUpd, dig_info.outputMask);

   dig_info.outputUpd = 0;
   dig_info.outputMask = 0;
} /* End digital_write_outputs */
