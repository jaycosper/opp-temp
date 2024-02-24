#!/usr/bin/env python
#
#===============================================================================
#
#                         OOOO
#                       OOOOOOOO
#      PPPPPPPPPPPPP   OOO    OOO   PPPPPPPPPPPPP
#    PPPPPPPPPPPPPP   OOO      OOO   PPPPPPPPPPPPPP
#   PPP         PPP   OOO      OOO   PPP         PPP
#  PPP          PPP   OOO      OOO   PPP          PPP
#  PPP          PPP   OOO      OOO   PPP          PPP
#  PPP          PPP   OOO      OOO   PPP          PPP
#   PPP         PPP   OOO      OOO   PPP         PPP
#    PPPPPPPPPPPPPP   OOO      OOO   PPPPPPPPPPPPPP
#     PPPPPPPPPPPPP   OOO      OOO   PPP
#               PPP   OOO      OOO   PPP
#               PPP   OOO      OOO   PPP
#               PPP   OOO      OOO   PPP
#               PPP    OOO    OOO    PPP
#               PPP     OOOOOOOO     PPP
#              PPPPP      OOOO      PPPPP
#
# @file:   regrCfg.py
# @author: Hugh Spahr
# @date:   07/10/2016
#
# @note:   Open Pinball Project
#          Copyright 2012-2019, Hugh Spahr
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
#===============================================================================
#
# This configures a processor board to be used as a regression tester for the
# firmware.  Wing 0 is Neopixel, wing 1 is solenoid, wing 2 is input, and wing 3
# is an incandescent board.
#
#===============================================================================

testVers = '00.00.02'

import rs232BIntf

# Config inputs as all state inputs
wingCfg = [ [ rs232BIntf.WING_NEO, rs232BIntf.WING_SOL, rs232BIntf.WING_INP, rs232BIntf.WING_INCAND ] ]

# Config inputs as all state inputs
inpCfg = [ [ rs232BIntf.CFG_INP_STATE, rs232BIntf.CFG_INP_STATE, rs232BIntf.CFG_INP_STATE, rs232BIntf.CFG_INP_STATE, \
             rs232BIntf.CFG_INP_STATE, rs232BIntf.CFG_INP_STATE, rs232BIntf.CFG_INP_STATE, rs232BIntf.CFG_INP_STATE, \
             rs232BIntf.CFG_INP_STATE, rs232BIntf.CFG_INP_STATE, rs232BIntf.CFG_INP_STATE, rs232BIntf.CFG_INP_STATE, \
             rs232BIntf.CFG_INP_STATE, rs232BIntf.CFG_INP_STATE, rs232BIntf.CFG_INP_STATE, rs232BIntf.CFG_INP_STATE, \
             rs232BIntf.CFG_INP_STATE, rs232BIntf.CFG_INP_STATE, rs232BIntf.CFG_INP_STATE, rs232BIntf.CFG_INP_STATE, \
             rs232BIntf.CFG_INP_STATE, rs232BIntf.CFG_INP_STATE, rs232BIntf.CFG_INP_STATE, rs232BIntf.CFG_INP_STATE, \
             rs232BIntf.CFG_INP_STATE, rs232BIntf.CFG_INP_STATE, rs232BIntf.CFG_INP_STATE, rs232BIntf.CFG_INP_STATE, \
             rs232BIntf.CFG_INP_STATE, rs232BIntf.CFG_INP_STATE, rs232BIntf.CFG_INP_STATE, rs232BIntf.CFG_INP_STATE ] ]

# solenoid config
solCfg  = [ [ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                rs232BIntf.CFG_SOL_USE_SWITCH, 0x30, 0x04, rs232BIntf.CFG_SOL_USE_SWITCH, 0x30, 0x04, \
                rs232BIntf.CFG_SOL_USE_SWITCH, 0x20, 0x00, rs232BIntf.CFG_SOL_USE_SWITCH, 0x20, 0x00, \
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00 ] ]

# Config color table
#              bytesPerPxl (1 byte), numPixels (1 byte, if 0, then 256 pixels), unused (95 bytes) 
colorCfg = [ [ 0x03, 0x08, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, \
               0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, \
               0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, \
               0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, \
               0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, \
               0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, \
               0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, \
               0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, \
               0xff, \
            ] ]
