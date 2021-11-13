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
 * @file:   timer.c
 * @author: Hugh Spahr
 * @date:   9/16/2015
 *
 * @note:   Open Pinball Project
 *          Copyright© 2015, Hugh Spahr
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
 * This is the timer function.  It is currently expecting a 1ms timer, and
 * simply calls the neopixel stuff on every tick.
 *
 *===============================================================================
 */
#include "stdtypes.h"
#include "procdefs.h"
#include "gen2glob.h"

typedef struct
{
   U16               loopUsec;
   INT               msCnt;
   INT               cnt10ms;
   INT               incandCnt;
} TMR_INFO;

TMR_INFO tmrInfo;

/* Prototypes */
void neo_10ms_tick();
void fade_10ms_tick();

/*
 * ===============================================================================
 * 
 * Name: timer_init
 * 
 * ===============================================================================
 */
/**
 * Initialize timer
 * 
 * Clear the msCnt.
 * 
 * @param   None 
 * @return  None
 * 
 * @pre     None 
 * @note    None
 * 
 * ===============================================================================
 */
void timer_init()
{
   tmrInfo.msCnt = 0;
   tmrInfo.cnt10ms = 0;
   tmrInfo.incandCnt = 5;   /* Offset cnt10ms by 5 ms */
    
   // Enable TIM1 clock
   rccBase_p->APB2ENR |= 0x00000800;

   // Initialize TIM1 1 ms timer
   tim1Base_p->PSC = 47;
   tim1Base_p->ARR = 999;
   tim1Base_p->CR1 = 0x0001;
   tim1Base_p->CR2 = 0x0000;
   tim1Base_p->SMCR = 0x0000;
   tim1Base_p->DIER = 0x0000;

   /* Register the timer isr, start the timer */
}

/*
 * ===============================================================================
 * 
 * Name: timer_overflow_isr
 * 
 * ===============================================================================
 */
/**
 * Timer overflow ISR
 * 
 * On an overflow interrupt, it increments the ms timer.
 * 
 * @param   None 
 * @return  None
 * 
 * @pre     None 
 * @note    None
 * 
 * ===============================================================================
 */
void timer_overflow_isr()
{
   tmrInfo.loopUsec = (U16)tim1Base_p->CNT;

   /* Statement added so interface can be polled instead of interrupt driven */
   if (tim1Base_p->SR & TIMx_SR_UIF)
   {
      tmrInfo.msCnt++;

      /* If timer overflowed after reading usec, just set it to 0 */
      if (tmrInfo.loopUsec > 900)
      {
    	  tmrInfo.loopUsec = 0;
      }
       
      /* Clear isr pending bit */
      tim1Base_p->SR = 0;
        
      /* Call the neopixel timer.  This will eventually be a registered
       *  recurring timer event with a function pointer.
       */
      tmrInfo.incandCnt++;
      if (tmrInfo.incandCnt >= 40)
      {
         tmrInfo.incandCnt = 0;

         /* Move to next LED state */
         gen2g_info.ledStateNum++;
         gen2g_info.ledStateNum &= (GEN2G_MAX_STATE_NUM - 1);
         if ((gen2g_info.ledStateNum & 0x7) == 0)
         {
            gen2g_info.ledStatus ^= GEN2G_STAT_BLINK_FAST_ON;
         }
         if (gen2g_info.ledStateNum == 0)
         {
            gen2g_info.ledStatus ^= GEN2G_STAT_BLINK_SLOW_ON;

            /* Blink status LED */
            gen2g_info.statusBlink ^= GEN2G_STAT_TOGGLE_LED;
            *((R32 *)GEN2G_STAT_BSRR_PTR) = gen2g_info.statusBlink;
         }
      }

      tmrInfo.cnt10ms++;
      if (tmrInfo.cnt10ms >= 10)
      {
         tmrInfo.cnt10ms = 0;
         neo_10ms_tick();
         fade_10ms_tick();
     }
   }
}

/*
 * ===============================================================================
 * 
 * Name: timer_get_ms_count
 * 
 * ===============================================================================
 */
/**
 * Get ms count
 * 
 * @param   None 
 * @return  ms count
 * 
 * @pre     None 
 * @note    None
 * 
 * ===============================================================================
 */
INT timer_get_ms_count()
{
   return(tmrInfo.msCnt);
}

/*
 * ===============================================================================
 *
 * Name: timer_get_us_count
 *
 * ===============================================================================
 */
/**
 * Get us count
 *
 * @param   None
 * @return  us count
 *
 * @pre     None
 * @note    None
 *
 * ===============================================================================
 */
U16 timer_get_us_count()
{
   return(tmrInfo.loopUsec);
}

/* [] END OF FILE */
