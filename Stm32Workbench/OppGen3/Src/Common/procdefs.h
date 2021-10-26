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

#ifndef PROCDEFS_H
#define PROCDEFS_H

typedef struct TIMx
{
   R32 CR1;
   R32 CR2;
   R32 SMCR;
   R32 DIER;
   R32 SR;
   R32 EGR;
   R32 CCMR[2];
   R32 CCER;
   R32 CNT;
   R32 PSC;
   R32 ARR;
   R32 Unused1;
   R32 CCR[4];
   R32 BDTR;
   R32 DCR;
   R32 DMAR;
} TIMxRegs;

#ifndef INSTANTIATE_PROC
extern
#endif
   TIMxRegs * volatile tim1Base_p
#ifdef INSTANTIATE_PROC
   = (TIMxRegs * volatile)0x40012c00
#endif
;

#ifndef INSTANTIATE_PROC
extern
#endif
   TIMxRegs * volatile tim2Base_p
#ifdef INSTANTIATE_PROC
   = (TIMxRegs * volatile)0x40000000
#endif
;

#ifndef INSTANTIATE_PROC
extern
#endif
   TIMxRegs * volatile tim3Base_p
#ifdef INSTANTIATE_PROC
   = (TIMxRegs * volatile)0x40000400
#endif
;

#ifndef INSTANTIATE_PROC
extern
#endif
   TIMxRegs * volatile tim4Base_p
#ifdef INSTANTIATE_PROC
   = (TIMxRegs * volatile)0x40000800
#endif
;

#define TIMx_SR_UIF   0x00000001

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
   GPIOxRegs * volatile gpioABase_p
#ifdef INSTANTIATE_PROC
= (GPIOxRegs * volatile)0x40010800
#endif
;

#ifndef INSTANTIATE_PROC
extern
#endif
   GPIOxRegs * volatile gpioBBase_p
#ifdef INSTANTIATE_PROC
= (GPIOxRegs * volatile)0x40010c00
#endif
;

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

typedef struct AFIORegs
{
   R32 EVCR;
   R32 MAPR;
   R32 EXTICR[4];
   R32 MAPR2;
} AFIORegs;

#ifndef INSTANTIATE_PROC
extern
#endif
   AFIORegs * volatile afioBase_p
#ifdef INSTANTIATE_PROC
   = (AFIORegs * volatile)0x40010000
#endif
;

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

typedef struct SPIx
{
   R32 CR1;
   R32 CR2;
   R32 SR;
   R32 DR;
   R32 CRCPR;
   R32 RXCRCR;
   R32 TXCRCR;
   R32 I2SCFGR;
   R32 I2SPR;
} SPIxRegs;

#ifndef INSTANTIATE_PROC
extern
#endif
SPIxRegs * volatile spi1Base_p
#ifdef INSTANTIATE_PROC
= (SPIxRegs * volatile)0x40013000
#endif
;

#ifndef INSTANTIATE_PROC
extern
#endif
SPIxRegs * volatile spi2Base_p
#ifdef INSTANTIATE_PROC
= (SPIxRegs * volatile)0x40003800
#endif
;

#define SPIx_CR1_SPE      0x00000040
#define SPIx_CR1_BR_16    0x00000018
#define SPIx_CR1_BR_8     0x00000010
#define SPIx_CR1_SSM      0x00000200
#define SPIx_CR1_SSI      0x00000100
#define SPIx_CR1_MSTR     0x00000004
#define SPIx_CR1_CPOL     0x00000002
#define SPIx_CR1_CPHA     0x00000001
#define SPIx_CR2_TXDMAEN  0x00000002

typedef struct DMAx
{
   R32 ISR;
   R32 IFCR;
   R32 CCR1;
   R32 CNDTR1;
   R32 CPAR1;
   R32 CMAR1;
   R32 Unused1;
   R32 CCR2;
   R32 CNDTR2;
   R32 CPAR2;
   R32 CMAR2;
   R32 Unused2;
   R32 CCR3;
   R32 CNDTR3;
   R32 CPAR3;
   R32 CMAR3;
   R32 Unused3;
   R32 CCR4;
   R32 CNDTR4;
   R32 CPAR4;
   R32 CMAR4;
   R32 Unused4;
   R32 CCR5;
   R32 CNDTR5;
   R32 CPAR5;
   R32 CMAR5;
   R32 Unused5;
   R32 CCR6;
   R32 CNDTR6;
   R32 CPAR6;
   R32 CMAR6;
   R32 Unused6;
   R32 CCR7;
   R32 CNDTR7;
   R32 CPAR7;
   R32 CMAR7;
   R32 Unused7;
} DMAxRegs;

#ifndef INSTANTIATE_PROC
extern
#endif
DMAxRegs * volatile dma1Base_p
#ifdef INSTANTIATE_PROC
= (DMAxRegs * volatile)0x40020000
#endif
;

#ifndef INSTANTIATE_PROC
extern
#endif
DMAxRegs * volatile dma2Base_p
#ifdef INSTANTIATE_PROC
= (DMAxRegs * volatile)0x40020400
#endif
;

#define DMAx_ISR_TEIF5    0x00080000
#define DMAx_ISR_TCIF5    0x00020000
#define DMAx_ISR_TEIF3    0x00000800
#define DMAx_ISR_TCIF3    0x00000200

#define DMAx_CCR_PSIZE16  0x00000100
#define DMAx_CCR_MINC     0x00000080
#define DMAx_CCR_DIR      0x00000010
#define DMAx_CCR_EN       0x00000001

#define EnableInterrupts         do                      \
                                 {                       \
                                    __asm("CPSIE   i");  \
                                 } while ( 0 )

#define DisableInterrupts        do                      \
                                 {                       \
                                    __asm("CPSID   i");  \
                                 } while ( 0 )

void Bootloadable_Load();
   
#endif

/* [] END OF FILE */
