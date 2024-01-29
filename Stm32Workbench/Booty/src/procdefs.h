/*
 *===============================================================================
 *
 *          SSSS
 *        SSSSSSSs                 DDD
 *       SSS    SSS     TTT        DDD            Hugh Spahr
 *      SSS  LLL SSS    TTT        DDD            Standard
 *       SSS LLL      TTTTTTT      DDD            Library
 *         SSSSL      TTTTTTT      DDD  
 *          SSSSS       TTT        DDD
 *           LSSSS      TTT    DDDDDDD
 *           LLLSSS     TTT  DDDDDDDDD
 *      SSS  LLL SSS    TTT DDD    DDD
 *       SSS LLLSSS     TTT DDD    DDD
 *        SSSSSSSS      TTT  DDDDDDDDD
 *          SSSS        TTT    DDDD DD
 *           LLL
 *           LLL   I    BBB
 *           LLL  III   BBB
 *           LLL   I    BBB
 *           LLL        BBB
 *           LLL  III   BBB
 *           LLL  III   BBB
 *           LLL  III   BBBBBBB
 *           LLL  III   BBBBBBBBB
 *           LLL  III   BBB    BBB
 *           LLL        BBB    BBB
 *           LLLLLLLLL  BBBBBBBBB
 *           LLLLLLLLL  BB BBBB
 *
 * @file:   procdefs.h
 * @author: Hugh Spahr
 * @date:   9/22/2015
 *
 * @note:   Copyright© 2015, Hugh Spahr
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
 * Processor definitions.  It contains generic defines for processor registers,
 * and macros for enable/disable interrupts.
 *
 *===============================================================================
 */

#pragma once

#include <stdint.h>

#define SRAM_FIRST_ADDR 0x20000000

typedef volatile uint8_t R8;
typedef volatile uint16_t R16;
typedef volatile uint32_t R32;

typedef struct GPIOxRegs
{
   R32 CRL;
   R32 CRH;
   R32 IDR;
   R32 ODR;
   R32 BSRR;
   R32 BRR;
   R32 LCKR;
} GPIOxRegs;

#ifndef INSTANTIATE_PROC
extern
#endif
   GPIOxRegs * volatile gpioCBase_p
#ifdef INSTANTIATE_PROC
= (GPIOxRegs * volatile)0x40011000
#endif
;

typedef struct RCCRegs
{
   R32 CR;
   R32 CFGR;
   R32 CIR;
   R32 APB2RSTR;
   R32 APB1RSTR;
   R32 AHBENR;
   R32 APB2ENR;
   R32 APB1ENR;
   R32 BDCR;
   R32 CSR;
   R32 AHBSTR;
   R32 CFGR2;
} RCCRegs;

#ifndef INSTANTIATE_PROC
extern
#endif
   RCCRegs * volatile rccBase_p
#ifdef INSTANTIATE_PROC
   = (RCCRegs * volatile)0x40021000
#endif
;

#define AHBENR_CRCEN     0x00000040

typedef struct FlashRegs
{
   R32 ACR;
   R32 KEYR;
   R32 OPTKEYR;
   R32 SR;
   R32 CR;
   R32 AR;
   R32 Unused;
   R32 OBR;
   R32 WRPR;
} FlashRegs;

#ifndef INSTANTIATE_PROC
extern
#endif
   FlashRegs * volatile flashBase_p
#ifdef INSTANTIATE_PROC
   = (FlashRegs * volatile)0x40022000
#endif
;

#define FLSH_SR_BSY       0x00000001
#define FLSH_SR_PGERR     0x00000004
#define FLSH_SR_WRPRTERR  0x00000010
#define FLSH_CR_PG        0x00000001
#define FLSH_CR_PER       0x00000002
#define FLSH_CR_STRT      0x00000040

typedef struct SCBRegs
{
   R32 CPUID;
   R32 ICSR;
   R32 VTOR;
   R32 AIRCR;
   R32 SCR;
   R32 CCR;
   R8  SHP[12];
} SCBRegs;

#ifndef INSTANTIATE_PROC
extern
#endif
SCBRegs * volatile scbBase_p
#ifdef INSTANTIATE_PROC
= (SCBRegs * volatile)0xe000ed00
#endif
;

#define AIRCR_SYSRESETREQ 0x05fa0004

typedef struct UsbRegs
{
  R16 EP0R;
  R16 Unused1;
  R16 EP1R;
  R16 Unused2;
  R16 EP2R;
  R16 Unused3;
  R16 EP3R;
  R16 Unused4;
  R16 EP4R;
  R16 Unused5;
  R16 EP5R;
  R16 Unused6;
  R16 EP6R;
  R16 Unused7;
  R16 EP7R;
  R16 Unused8[17];
  R16 CNTR;
  R16 Unused9;
  R16 ISTR;
  R16 Unused10;
  R16 FNR;
  R16 Unused11;
  R16 DADDR;
  R16 Unused12;
  R16 BTABLE;
  R16 Unused13;
} UsbRegs;

#ifndef INSTANTIATE_PROC
extern
#endif
UsbRegs * volatile usbBase_p
#ifdef INSTANTIATE_PROC
= (UsbRegs * volatile)0x40005c00
#endif
;

typedef struct CrcRegs
{
  R32 DR;
  R32 IDR;
  R32 CR;
} CrcRegs;

#ifndef INSTANTIATE_PROC
extern
#endif
CrcRegs * volatile crcBase_p
#ifdef INSTANTIATE_PROC
= (CrcRegs * volatile)0x40023000
#endif
;
