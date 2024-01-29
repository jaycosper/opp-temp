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
 * @file:   debug.c
 * @author: Hugh Spahr
 * @date:   11/29/2015
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
 * This is a debug file.  It saves an initial configuration into flash
 * other utilities.
 *
 *===============================================================================
 */
#include "stdtypes.h"
#include "stdlintf.h"
#include "gen2glob.h"
#include "versglob.h"

/* Minimal configuration:  Only contains input configuration.
 */
const U8 cfg[0x20] = {
   /* First config is for inputs, 0x20 in size, state inputs */
   /* Entry 0        Entry 1           Entry 2           Entry 3 */
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0-7 */
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 8-15 */
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 16-23 */
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 24-31 */
};

#if 0
/* Full configuration:  This is not used, but is here to show the
 *   locations of a maximum configuration using inputs, solenoids,
 *   and neopixel color table.
 */
const U8 cfg[0xf0] = {
   /* First config is for inputs, 0x20 in size, state inputs */
   /* Entry 0        Entry 1           Entry 2           Entry 3 */
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0-7 */
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 8-15 */
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 16-23 */
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 24-31 */

   /* Second config is for solenoids, 0x30 in size, 2 flippers and 2 kickers */
   /* Entry 0        Entry 1           Entry 2           Entry 3 */
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* 0-3 */
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* 4-7 */
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* 8-11 */
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* 12-15 */
   
   /* Third config is color table, 0x60 in size */
   /* Entry 0        Entry 1           Entry 2           Entry 3 */
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* 0-3 */
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* 4-7 */
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* 8-11 */
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* 12-15 */
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* 16-19 */
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* 20-23 */
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* 24-27 */
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* 28-31 */

   /* Unused config */
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};
#endif
   
/*
 * ===============================================================================
 * 
 * Name: debug_save_nv_cfg
 * 
 * ===============================================================================
 */
/**
 * Debug save non-volatile cfg
 * 
 * Save non-volatile configuration settings.
 * 
 * @param   None
 * @param   None 
 * @return  None
 * 
 * @pre     None 
 * @note    None
 * 
 * ===============================================================================
 */
void debug_save_nv_cfg()
{
   U8                         *src_p;
   U8                         *dst_p;
   
   gen2g_info.nvCfgInfo.nvCfgCrc = 0xff;
   gen2g_info.nvCfgInfo.wingCfg[0] = WING_INP;
   gen2g_info.nvCfgInfo.wingCfg[1] = WING_INP;
   gen2g_info.nvCfgInfo.wingCfg[2] = WING_INP;
   gen2g_info.nvCfgInfo.wingCfg[3] = WING_INP;
   appStart.codeVers = VERSG_CODE_VERS;
   
   for (src_p = (U8 *)&cfg[0], dst_p = (U8 *)&gen2g_info.nvCfgInfo.cfgData;
      src_p < (U8 *)&cfg[0] + sizeof(cfg); )
   {
      *dst_p++ = *src_p++;
   }
   
   stdlser_calc_crc8(&gen2g_info.nvCfgInfo.nvCfgCrc, 0xfc,
      &gen2g_info.nvCfgInfo.wingCfg[0]);

   /* Erase sector */
   stdlflash_sector_erase((U16 *)GEN2G_CFG_TBL);
   
   /* Write sector */
   stdlflash_write((U16 *)&gen2g_info.nvCfgInfo,
      (U16 *)GEN2G_CFG_TBL, sizeof(GEN2G_NV_CFG_T));
   
} /* End debug_save_nv_cfg */

/* [] END OF FILE */
