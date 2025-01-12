/******************************************************************************/
/* Copyright (c) 2013-2023 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_AHB_H
#define RT_RTARCH_AHB_H

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_aHB.h: Implementation of AArch64 half+byte BASE instructions.
 *
 * This file is a part of the unified SIMD assembler framework (rtarch.h)
 * designed to be compatible with different processor architectures,
 * while maintaining strictly defined common API.
 *
 * Recommended naming scheme for instructions:
 *
 * cmdxx_ri - applies [cmd] to [r]egister from [i]mmediate
 * cmdxx_mi - applies [cmd] to [m]emory   from [i]mmediate
 * cmdxx_rz - applies [cmd] to [r]egister from [z]ero-arg
 * cmdxx_mz - applies [cmd] to [m]emory   from [z]ero-arg
 *
 * cmdxx_rm - applies [cmd] to [r]egister from [m]emory
 * cmdxx_ld - applies [cmd] as above
 * cmdxx_mr - applies [cmd] to [m]emory   from [r]egister
 * cmdxx_st - applies [cmd] as above (arg list as cmdxx_ld)
 *
 * cmdxx_rr - applies [cmd] to [r]egister from [r]egister
 * cmdxx_mm - applies [cmd] to [m]emory   from [m]emory
 * cmdxx_rx - applies [cmd] to [r]egister (one-operand cmd)
 * cmdxx_mx - applies [cmd] to [m]emory   (one-operand cmd)
 *
 * cmdxx_rx - applies [cmd] to [r]egister from x-register
 * cmdxx_mx - applies [cmd] to [m]emory   from x-register
 * cmdxx_xr - applies [cmd] to x-register from [r]egister
 * cmdxx_xm - applies [cmd] to x-register from [m]emory
 *
 * cmd*x_** - applies [cmd] to unsigned integer args, [x] - default
 * cmd*n_** - applies [cmd] to   signed integer args, [n] - negatable
 *
 * cmd**Z** - applies [cmd] while setting condition flags, [Z] - zero flag.
 * Regular cmd*x_**, cmd*n_** instructions may or may not set flags depending
 * on the target architecture, thus no assumptions can be made for jezxx/jnzxx.
 *
 * Interpretation of instruction parameters:
 *
 * upper-case params have triplet structure and require W to pass-forward
 * lower-case params are singular and can be used/passed as such directly
 *
 * RD - BASE register serving as destination only, if present
 * RG - BASE register serving as destination and first source
 * RS - BASE register serving as second source (first if any)
 * RT - BASE register serving as third source (second if any)
 *
 * MD - BASE addressing mode (Oeax, M***, I***) (memory-dest)
 * MG - BASE addressing mode (Oeax, M***, I***) (memory-dsrc)
 * MS - BASE addressing mode (Oeax, M***, I***) (memory-src2)
 * MT - BASE addressing mode (Oeax, M***, I***) (memory-src3)
 *
 * DD - displacement value (DP, DF, DG, DH, DV) (memory-dest)
 * DG - displacement value (DP, DF, DG, DH, DV) (memory-dsrc)
 * DS - displacement value (DP, DF, DG, DH, DV) (memory-src2)
 * DT - displacement value (DP, DF, DG, DH, DV) (memory-src3)
 *
 * IS - immediate value (is used as a second or first source)
 * IT - immediate value (is used as a third or second source)
 *
 * Alphabetical view of current/future instruction namespaces is in rtzero.h.
 * Configurable BASE/SIMD subsets (cmdx*, cmdy*, cmdp*) are defined in rtconf.h.
 * Mixing of 64/32-bit fields in backend structures may lead to misalignment
 * of 64-bit fields to 4-byte boundary, which is not supported on some targets.
 * Place fields carefully to ensure natural alignment for all data types.
 * Note that within cmdx*_** subset most of the instructions follow in-heap
 * address size (RT_ADDRESS or A) and only label_ld/st, jmpxx_xr/xm follow
 * pointer size (RT_POINTER or P) as code/data/stack segments are fixed.
 * Stack ops always work with full registers regardless of the mode chosen.
 *
 * 64/32-bit subsets are both self-consistent within themselves, 32-bit results
 * cannot be used in 64-bit subset without proper sign/zero-extend bridges,
 * cmdwn/wz bridges for 32-bit subset are provided in 64-bit headers.
 * 16/8-bit subsets are both self-consistent within themselves, their results
 * cannot be used in larger subsets without proper sign/zero-extend bridges,
 * cmdhn/hz and cmdbn/bz bridges for 16/8-bit are provided in 32-bit headers.
 * The results of 8-bit subset cannot be used within 16-bit subset consistently.
 * There is no sign/zero-extend bridge from 8-bit to 16-bit, use 32-bit instead.
 *
 * 32-bit and 64-bit BASE subsets are not easily compatible on all targets,
 * thus any register modified with 32-bit op cannot be used in 64-bit subset.
 * Alternatively, data flow must not exceed 31-bit range for 32-bit operations
 * to produce consistent results usable in 64-bit subsets across all targets.
 * Registers written with 64-bit op aren't always compatible with 32-bit either,
 * as m64 requires the upper half to be all 0s or all 1s for m32 arithmetic.
 * Only a64 and x64 have a complete 32-bit support in 64-bit mode both zeroing
 * the upper half of the result, while m64 sign-extending all 32-bit operations
 * and p64 overflowing 32-bit arithmetic into the upper half. Similar reasons
 * of inconsistency prohibit use of IW immediate type within 64-bit subsets,
 * where a64 and p64 zero-extend, while x64 and m64 sign-extend 32-bit value.
 *
 * Note that offset correction for endianness E is only applicable for addresses
 * within pointer fields, when (in-heap) address and pointer sizes don't match.
 * Working with 32-bit data in 64-bit fields in any other circumstances must be
 * done consistently within a subset of one size (32-bit, 64-bit or C/C++).
 * Alternatively, data written natively in C/C++ can be worked on from within
 * a given (one) subset if appropriate offset correction is used from rtbase.h.
 *
 * Setting-flags instruction naming scheme was changed twice in the past for
 * better orthogonality with operand size, type and args-list. It is therefore
 * recommended to use combined-arithmetic-jump (arj) for better API stability
 * and maximum efficiency across all supported targets. For similar reasons
 * of higher performance on MIPS and POWER use combined-compare-jump (cmj).
 * Not all canonical forms of BASE instructions have efficient implementation.
 * For example, some forms of shifts and division use stack ops on x86 targets,
 * while standalone remainders can only be done natively on MIPSr6 and POWER9.
 * Consider using special fixed-register forms for maximum performance.
 *
 * Argument x-register (implied) is fixed by the implementation.
 * Some formal definitions are not given below to encourage
 * use of friendly aliases for better code readability.
 *
 * Only the first 4 registers are available for byte BASE logic/arithmetic and
 * shifts on legacy 32-bit targets with 8 BASE registers (ARMv7, x86).
 */

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

/******************************************************************************/
/**********************************   BASE   **********************************/
/******************************************************************************/

/************************* 16-bit subset instructions *************************/

/* mov (D = S)
 * set-flags: no */

#define movhx_ri(RD, IS)                                                    \
        AUW(EMPTY,    VAL(IS), REG(RD), EMPTY,   EMPTY,   EMPTY2, G3(IS))

#define movhx_mi(MD, DD, IS)                                                \
        AUW(SIB(MD),  VAL(IS), TIxx,    MOD(MD), VAL(DD), C1(DD), G3(IS))   \
        EMITW(0x78000000 | MDM(TIxx,    MOD(MD), VHL(DD), B1(DD), P1(DD)))

#define movhx_rr(RD, RS)                                                    \
        EMITW(0x2A000000 | MRM(REG(RD), TZxx,    REG(RS)))

#define movhx_ld(RD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78400000 | MDM(REG(RD), MOD(MS), VHL(DS), B1(DS), P1(DS)))

#define movhx_st(RS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), C1(DD), EMPTY2)   \
        EMITW(0x78000000 | MDM(REG(RS), MOD(MD), VHL(DD), B1(DD), P1(DD)))

/* and (G = G & S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define andhx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)))

#define andhx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))

#define andhx_rr(RG, RS)                                                    \
        EMITW(0x0A000000 | MRM(REG(RG), REG(RG), REG(RS)))

#define andhx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MS), VHL(DS), B1(DS), P1(DS)))  \
        EMITW(0x0A000000 | MRM(REG(RG), REG(RG), TMxx))

#define andhx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x0A000000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))

#define andhx_mr(MG, DG, RS)                                                \
        andhx_st(W(RS), W(MG), W(DG))


#define andhxZri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0x72003C00 | MRM(REG(RG), REG(RG), 0x00))/* <- set flags (Z) */

#define andhxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x60000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))

#define andhxZrr(RG, RS)                                                    \
        EMITW(0x0A000000 | MRM(REG(RG), REG(RG), REG(RS)))                  \
        EMITW(0x72003C00 | MRM(REG(RG), REG(RG), 0x00))/* <- set flags (Z) */

#define andhxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MS), VHL(DS), B1(DS), P1(DS)))  \
        EMITW(0x6A000000 | MRM(REG(RG), REG(RG), TMxx))

#define andhxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x6A000000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))

#define andhxZmr(MG, DG, RS)                                                \
        andhxZst(W(RS), W(MG), W(DG))

/* ann (G = ~G & S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define annhx_ri(RG, IS)                                                    \
        nothx_rx(W(RG))                                                     \
        andhx_ri(W(RG), W(IS))

#define annhx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x2A200000 | MRM(TMxx,    TZxx,    TMxx))                     \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))

#define annhx_rr(RG, RS)                                                    \
        EMITW(0x0A200000 | MRM(REG(RG), REG(RS), REG(RG)))

#define annhx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MS), VHL(DS), B1(DS), P1(DS)))  \
        EMITW(0x0A200000 | MRM(REG(RG), TMxx,    REG(RG)))

#define annhx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x0A200000 | MRM(TMxx,    REG(RS), TMxx))                     \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))

#define annhx_mr(MG, DG, RS)                                                \
        annhx_st(W(RS), W(MG), W(DG))


#define annhxZri(RG, IS)                                                    \
        nothx_rx(W(RG))                                                     \
        andhxZri(W(RG), W(IS))

#define annhxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x2A200000 | MRM(TMxx,    TZxx,    TMxx))                     \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x72003C00 | MRM(TMxx,    TMxx,    0x00))/* <- set flags (Z) */

#define annhxZrr(RG, RS)                                                    \
        EMITW(0x0A200000 | MRM(REG(RG), REG(RS), REG(RG)))                  \
        EMITW(0x72003C00 | MRM(REG(RG), REG(RG), 0x00))/* <- set flags (Z) */

#define annhxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MS), VHL(DS), B1(DS), P1(DS)))  \
        EMITW(0x6A200000 | MRM(REG(RG), TMxx,    REG(RG)))

#define annhxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x0A200000 | MRM(TMxx,    REG(RS), TMxx))                     \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x72003C00 | MRM(TMxx,    TMxx,    0x00))/* <- set flags (Z) */

#define annhxZmr(MG, DG, RS)                                                \
        annhxZst(W(RS), W(MG), W(DG))

/* orr (G = G | S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define orrhx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0x20000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)))

#define orrhx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x20000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))

#define orrhx_rr(RG, RS)                                                    \
        EMITW(0x2A000000 | MRM(REG(RG), REG(RG), REG(RS)))

#define orrhx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MS), VHL(DS), B1(DS), P1(DS)))  \
        EMITW(0x2A000000 | MRM(REG(RG), REG(RG), TMxx))

#define orrhx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x2A000000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))

#define orrhx_mr(MG, DG, RS)                                                \
        orrhx_st(W(RS), W(MG), W(DG))


#define orrhxZri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0x20000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0x72003C00 | MRM(REG(RG), REG(RG), 0x00))/* <- set flags (Z) */

#define orrhxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x20000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x72003C00 | MRM(TMxx,    TMxx,    0x00))/* <- set flags (Z) */

#define orrhxZrr(RG, RS)                                                    \
        EMITW(0x2A000000 | MRM(REG(RG), REG(RG), REG(RS)))                  \
        EMITW(0x72003C00 | MRM(REG(RG), REG(RG), 0x00))/* <- set flags (Z) */

#define orrhxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MS), VHL(DS), B1(DS), P1(DS)))  \
        EMITW(0x2A000000 | MRM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0x72003C00 | MRM(REG(RG), REG(RG), 0x00))/* <- set flags (Z) */

#define orrhxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x2A000000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x72003C00 | MRM(TMxx,    TMxx,    0x00))/* <- set flags (Z) */

#define orrhxZmr(MG, DG, RS)                                                \
        orrhxZst(W(RS), W(MG), W(DG))

/* orn (G = ~G | S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define ornhx_ri(RG, IS)                                                    \
        nothx_rx(W(RG))                                                     \
        orrhx_ri(W(RG), W(IS))

#define ornhx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x2A200000 | MRM(TMxx,    TZxx,    TMxx))                     \
        EMITW(0x20000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))

#define ornhx_rr(RG, RS)                                                    \
        EMITW(0x2A200000 | MRM(REG(RG), REG(RS), REG(RG)))

#define ornhx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MS), VHL(DS), B1(DS), P1(DS)))  \
        EMITW(0x2A200000 | MRM(REG(RG), TMxx,    REG(RG)))

#define ornhx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x2A200000 | MRM(TMxx,    REG(RS), TMxx))                     \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))

#define ornhx_mr(MG, DG, RS)                                                \
        ornhx_st(W(RS), W(MG), W(DG))


#define ornhxZri(RG, IS)                                                    \
        nothx_rx(W(RG))                                                     \
        orrhxZri(W(RG), W(IS))

#define ornhxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x2A200000 | MRM(TMxx,    TZxx,    TMxx))                     \
        EMITW(0x20000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x72003C00 | MRM(TMxx,    TMxx,    0x00))/* <- set flags (Z) */

#define ornhxZrr(RG, RS)                                                    \
        EMITW(0x2A200000 | MRM(REG(RG), REG(RS), REG(RG)))                  \
        EMITW(0x72003C00 | MRM(REG(RG), REG(RG), 0x00))/* <- set flags (Z) */

#define ornhxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MS), VHL(DS), B1(DS), P1(DS)))  \
        EMITW(0x2A200000 | MRM(REG(RG), TMxx,    REG(RG)))                  \
        EMITW(0x72003C00 | MRM(REG(RG), REG(RG), 0x00))/* <- set flags (Z) */

#define ornhxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x2A200000 | MRM(TMxx,    REG(RS), TMxx))                     \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x72003C00 | MRM(TMxx,    TMxx,    0x00))/* <- set flags (Z) */

#define ornhxZmr(MG, DG, RS)                                                \
        ornhxZst(W(RS), W(MG), W(DG))

/* xor (G = G ^ S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define xorhx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0x40000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)))

#define xorhx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x40000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))

#define xorhx_rr(RG, RS)                                                    \
        EMITW(0x4A000000 | MRM(REG(RG), REG(RG), REG(RS)))

#define xorhx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MS), VHL(DS), B1(DS), P1(DS)))  \
        EMITW(0x4A000000 | MRM(REG(RG), REG(RG), TMxx))

#define xorhx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x4A000000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))

#define xorhx_mr(MG, DG, RS)                                                \
        xorhx_st(W(RS), W(MG), W(DG))


#define xorhxZri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0x40000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0x72003C00 | MRM(REG(RG), REG(RG), 0x00))/* <- set flags (Z) */

#define xorhxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x40000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x72003C00 | MRM(TMxx,    TMxx,    0x00))/* <- set flags (Z) */

#define xorhxZrr(RG, RS)                                                    \
        EMITW(0x4A000000 | MRM(REG(RG), REG(RG), REG(RS)))                  \
        EMITW(0x72003C00 | MRM(REG(RG), REG(RG), 0x00))/* <- set flags (Z) */

#define xorhxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MS), VHL(DS), B1(DS), P1(DS)))  \
        EMITW(0x4A000000 | MRM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0x72003C00 | MRM(REG(RG), REG(RG), 0x00))/* <- set flags (Z) */

#define xorhxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x4A000000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x72003C00 | MRM(TMxx,    TMxx,    0x00))/* <- set flags (Z) */

#define xorhxZmr(MG, DG, RS)                                                \
        xorhxZst(W(RS), W(MG), W(DG))

/* not (G = ~G)
 * set-flags: no */

#define nothx_rx(RG)                                                        \
        EMITW(0x2A200000 | MRM(REG(RG), TZxx,    REG(RG)))

#define nothx_mx(MG, DG)                                                    \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x2A200000 | MRM(TMxx,    TZxx,    TMxx))                     \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))

/* neg (G = -G)
 * set-flags: undefined (*_*), yes (*Z*) */

#define neghx_rx(RG)                                                        \
        EMITW(0x4B000000 | MRM(REG(RG), TZxx,    REG(RG)))

#define neghx_mx(MG, DG)                                                    \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x4B000000 | MRM(TMxx,    TZxx,    TMxx))                     \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))


#define neghxZrx(RG)                                                        \
        EMITW(0x4B000000 | MRM(REG(RG), TZxx,    REG(RG)))                  \
        EMITW(0x72003C00 | MRM(REG(RG), REG(RG), 0x00))/* <- set flags (Z) */

#define neghxZmx(MG, DG)                                                    \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x6B000000 | MRM(TMxx,    TZxx,    TMxx))                     \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))

/* add (G = G + S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define addhx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0x01000000 | MIM(REG(RG), REG(RG), VAL(IS), T1(IS), M1(IS)))

#define addhx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G1(IS))   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x01000000 | MIM(TMxx,    TMxx,    VAL(IS), T1(IS), M1(IS)))  \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))

#define addhx_rr(RG, RS)                                                    \
        EMITW(0x0B000000 | MRM(REG(RG), REG(RG), REG(RS)))

#define addhx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MS), VHL(DS), B1(DS), P1(DS)))  \
        EMITW(0x0B000000 | MRM(REG(RG), REG(RG), TMxx))

#define addhx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x0B000000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))

#define addhx_mr(MG, DG, RS)                                                \
        addhx_st(W(RS), W(MG), W(DG))


#define addhxZri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0x01000000 | MIM(REG(RG), REG(RG), VAL(IS), T1(IS), M1(IS)))  \
        EMITW(0x72003C00 | MRM(REG(RG), REG(RG), 0x00))/* <- set flags (Z) */

#define addhxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G1(IS))   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x01000000 | MIM(TMxx,    TMxx,    VAL(IS), T1(IS), M1(IS)))  \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x72003C00 | MRM(TMxx,    TMxx,    0x00))/* <- set flags (Z) */

#define addhxZrr(RG, RS)                                                    \
        EMITW(0x0B000000 | MRM(REG(RG), REG(RG), REG(RS)))                  \
        EMITW(0x72003C00 | MRM(REG(RG), REG(RG), 0x00))/* <- set flags (Z) */

#define addhxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MS), VHL(DS), B1(DS), P1(DS)))  \
        EMITW(0x0B000000 | MRM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0x72003C00 | MRM(REG(RG), REG(RG), 0x00))/* <- set flags (Z) */

#define addhxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x0B000000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x72003C00 | MRM(TMxx,    TMxx,    0x00))/* <- set flags (Z) */

#define addhxZmr(MG, DG, RS)                                                \
        addhxZst(W(RS), W(MG), W(DG))

/* sub (G = G - S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define subhx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0x41000000 | MIM(REG(RG), REG(RG), VAL(IS), T1(IS), M1(IS)))

#define subhx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G1(IS))   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x41000000 | MIM(TMxx,    TMxx,    VAL(IS), T1(IS), M1(IS)))  \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))

#define subhx_rr(RG, RS)                                                    \
        EMITW(0x4B000000 | MRM(REG(RG), REG(RG), REG(RS)))

#define subhx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MS), VHL(DS), B1(DS), P1(DS)))  \
        EMITW(0x4B000000 | MRM(REG(RG), REG(RG), TMxx))

#define subhx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x4B000000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))

#define subhx_mr(MG, DG, RS)                                                \
        subhx_st(W(RS), W(MG), W(DG))


#define subhxZri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0x41000000 | MIM(REG(RG), REG(RG), VAL(IS), T1(IS), M1(IS)))  \
        EMITW(0x72003C00 | MRM(REG(RG), REG(RG), 0x00))/* <- set flags (Z) */

#define subhxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G1(IS))   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x41000000 | MIM(TMxx,    TMxx,    VAL(IS), T1(IS), M1(IS)))  \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x72003C00 | MRM(TMxx,    TMxx,    0x00))/* <- set flags (Z) */

#define subhxZrr(RG, RS)                                                    \
        EMITW(0x4B000000 | MRM(REG(RG), REG(RG), REG(RS)))                  \
        EMITW(0x72003C00 | MRM(REG(RG), REG(RG), 0x00))/* <- set flags (Z) */

#define subhxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MS), VHL(DS), B1(DS), P1(DS)))  \
        EMITW(0x4B000000 | MRM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0x72003C00 | MRM(REG(RG), REG(RG), 0x00))/* <- set flags (Z) */

#define subhxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x4B000000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x72003C00 | MRM(TMxx,    TMxx,    0x00))/* <- set flags (Z) */

#define subhxZmr(MG, DG, RS)                                                \
        subhxZst(W(RS), W(MG), W(DG))

/* shl (G = G << S)
 * set-flags: undefined (*_*), yes (*Z*)
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlhx_rx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x1AC02000 | MRM(REG(RG), REG(RG), Tecx))

#define shlhx_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x1AC02000 | MRM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))

#define shlhx_ri(RG, IS)                                                    \
        EMITW(0x53000000 | MRM(REG(RG), REG(RG), 0x00) |                    \
        (-VAL(IS)&0x1F)<<16 | (31-(VAL(IS)&0x1F))<<10)

#define shlhx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x53000000 | MRM(TMxx,    TMxx,    0x00) |                    \
        (-VAL(IS)&0x1F)<<16 | (31-(VAL(IS)&0x1F))<<10)                      \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))

#define shlhx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x1AC02000 | MRM(REG(RG), REG(RG), REG(RS)))

#define shlhx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MS), VHL(DS), B1(DS), P1(DS)))  \
        EMITW(0x1AC02000 | MRM(REG(RG), REG(RG), TMxx))

#define shlhx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x1AC02000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))

#define shlhx_mr(MG, DG, RS)                                                \
        shlhx_st(W(RS), W(MG), W(DG))


#define shlhxZrx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x1AC02000 | MRM(REG(RG), REG(RG), Tecx))                     \
        EMITW(0x72003C00 | MRM(REG(RG), REG(RG), 0x00))/* <- set flags (Z) */

#define shlhxZmx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x1AC02000 | MRM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x72003C00 | MRM(TMxx,    TMxx,    0x00))/* <- set flags (Z) */

#define shlhxZri(RG, IS)                                                    \
        EMITW(0x53000000 | MRM(REG(RG), REG(RG), 0x00) |                    \
        (-VAL(IS)&0x1F)<<16 | (31-(VAL(IS)&0x1F))<<10)                      \
        EMITW(0x72003C00 | MRM(REG(RG), REG(RG), 0x00))/* <- set flags (Z) */

#define shlhxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x53000000 | MRM(TMxx,    TMxx,    0x00) |                    \
        (-VAL(IS)&0x1F)<<16 | (31-(VAL(IS)&0x1F))<<10)                      \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x72003C00 | MRM(TMxx,    TMxx,    0x00))/* <- set flags (Z) */

#define shlhxZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x1AC02000 | MRM(REG(RG), REG(RG), REG(RS)))                  \
        EMITW(0x72003C00 | MRM(REG(RG), REG(RG), 0x00))/* <- set flags (Z) */

#define shlhxZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MS), VHL(DS), B1(DS), P1(DS)))  \
        EMITW(0x1AC02000 | MRM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0x72003C00 | MRM(REG(RG), REG(RG), 0x00))/* <- set flags (Z) */

#define shlhxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x1AC02000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x72003C00 | MRM(TMxx,    TMxx,    0x00))/* <- set flags (Z) */

#define shlhxZmr(MG, DG, RS)                                                \
        shlhxZst(W(RS), W(MG), W(DG))

/* shr (G = G >> S), unsigned (logical)
 * set-flags: undefined (*_*), yes (*Z*)
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrhx_rx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x53003C00 | MRM(REG(RG), REG(RG), 0x00))                     \
        EMITW(0x1AC02400 | MRM(REG(RG), REG(RG), Tecx))

#define shrhx_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x1AC02400 | MRM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))

#define shrhx_ri(RG, IS)                                                    \
        EMITW(0x53003C00 | MRM(REG(RG), REG(RG), 0x00))                     \
        EMITW(0x53007C00 | MRM(REG(RG), REG(RG), 0x00) |(VAL(IS)&0x1F)<<16)

#define shrhx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x53007C00 | MRM(TMxx,    TMxx,    0x00) |(VAL(IS)&0x1F)<<16) \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))

#define shrhx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x53003C00 | MRM(REG(RG), REG(RG), 0x00))                     \
        EMITW(0x1AC02400 | MRM(REG(RG), REG(RG), REG(RS)))

#define shrhx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MS), VHL(DS), B1(DS), P1(DS)))  \
        EMITW(0x53003C00 | MRM(REG(RG), REG(RG), 0x00))                     \
        EMITW(0x1AC02400 | MRM(REG(RG), REG(RG), TMxx))

#define shrhx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x1AC02400 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))

#define shrhx_mr(MG, DG, RS)                                                \
        shrhx_st(W(RS), W(MG), W(DG))


#define shrhxZrx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x53003C00 | MRM(REG(RG), REG(RG), 0x00))                     \
        EMITW(0x1AC02400 | MRM(REG(RG), REG(RG), Tecx))                     \
        EMITW(0x72003C00 | MRM(REG(RG), REG(RG), 0x00))/* <- set flags (Z) */

#define shrhxZmx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x1AC02400 | MRM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x72003C00 | MRM(TMxx,    TMxx,    0x00))/* <- set flags (Z) */

#define shrhxZri(RG, IS)                                                    \
        EMITW(0x53003C00 | MRM(REG(RG), REG(RG), 0x00))                     \
        EMITW(0x53007C00 | MRM(REG(RG), REG(RG), 0x00) |(VAL(IS)&0x1F)<<16) \
        EMITW(0x72003C00 | MRM(REG(RG), REG(RG), 0x00))/* <- set flags (Z) */

#define shrhxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x53007C00 | MRM(TMxx,    TMxx,    0x00) |(VAL(IS)&0x1F)<<16) \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x72003C00 | MRM(TMxx,    TMxx,    0x00))/* <- set flags (Z) */

#define shrhxZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x53003C00 | MRM(REG(RG), REG(RG), 0x00))                     \
        EMITW(0x1AC02400 | MRM(REG(RG), REG(RG), REG(RS)))                  \
        EMITW(0x72003C00 | MRM(REG(RG), REG(RG), 0x00))/* <- set flags (Z) */

#define shrhxZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MS), VHL(DS), B1(DS), P1(DS)))  \
        EMITW(0x53003C00 | MRM(REG(RG), REG(RG), 0x00))                     \
        EMITW(0x1AC02400 | MRM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0x72003C00 | MRM(REG(RG), REG(RG), 0x00))/* <- set flags (Z) */

#define shrhxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x1AC02400 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x72003C00 | MRM(TMxx,    TMxx,    0x00))/* <- set flags (Z) */

#define shrhxZmr(MG, DG, RS)                                                \
        shrhxZst(W(RS), W(MG), W(DG))

/* shr (G = G >> S), signed (arithmetic)
 * set-flags: undefined (*_*), yes (*Z*)
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrhn_rx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x13003C00 | MRM(REG(RG), REG(RG), 0x00))                     \
        EMITW(0x1AC02800 | MRM(REG(RG), REG(RG), Tecx))

#define shrhn_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x78800000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x1AC02800 | MRM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))

#define shrhn_ri(RG, IS)                                                    \
        EMITW(0x13003C00 | MRM(REG(RG), REG(RG), 0x00))                     \
        EMITW(0x13007C00 | MRM(REG(RG), REG(RG), 0x00) |(VAL(IS)&0x1F)<<16)

#define shrhn_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x78800000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x13007C00 | MRM(TMxx,    TMxx,    0x00) |(VAL(IS)&0x1F)<<16) \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))

#define shrhn_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x13003C00 | MRM(REG(RG), REG(RG), 0x00))                     \
        EMITW(0x1AC02800 | MRM(REG(RG), REG(RG), REG(RS)))

#define shrhn_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MS), VHL(DS), B1(DS), P1(DS)))  \
        EMITW(0x13003C00 | MRM(REG(RG), REG(RG), 0x00))                     \
        EMITW(0x1AC02800 | MRM(REG(RG), REG(RG), TMxx))

#define shrhn_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x78800000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x1AC02800 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))

#define shrhn_mr(MG, DG, RS)                                                \
        shrhn_st(W(RS), W(MG), W(DG))


#define shrhnZrx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x13003C00 | MRM(REG(RG), REG(RG), 0x00))                     \
        EMITW(0x1AC02800 | MRM(REG(RG), REG(RG), Tecx))                     \
        EMITW(0x72003C00 | MRM(REG(RG), REG(RG), 0x00))/* <- set flags (Z) */

#define shrhnZmx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x78800000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x1AC02800 | MRM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x72003C00 | MRM(TMxx,    TMxx,    0x00))/* <- set flags (Z) */

#define shrhnZri(RG, IS)                                                    \
        EMITW(0x13003C00 | MRM(REG(RG), REG(RG), 0x00))                     \
        EMITW(0x13007C00 | MRM(REG(RG), REG(RG), 0x00) |(VAL(IS)&0x1F)<<16) \
        EMITW(0x72003C00 | MRM(REG(RG), REG(RG), 0x00))/* <- set flags (Z) */

#define shrhnZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x78800000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x13007C00 | MRM(TMxx,    TMxx,    0x00) |(VAL(IS)&0x1F)<<16) \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x72003C00 | MRM(TMxx,    TMxx,    0x00))/* <- set flags (Z) */

#define shrhnZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x13003C00 | MRM(REG(RG), REG(RG), 0x00))                     \
        EMITW(0x1AC02800 | MRM(REG(RG), REG(RG), REG(RS)))                  \
        EMITW(0x72003C00 | MRM(REG(RG), REG(RG), 0x00))/* <- set flags (Z) */

#define shrhnZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MS), VHL(DS), B1(DS), P1(DS)))  \
        EMITW(0x13003C00 | MRM(REG(RG), REG(RG), 0x00))                     \
        EMITW(0x1AC02800 | MRM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0x72003C00 | MRM(REG(RG), REG(RG), 0x00))/* <- set flags (Z) */

#define shrhnZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x78800000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x1AC02800 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x72003C00 | MRM(TMxx,    TMxx,    0x00))/* <- set flags (Z) */

#define shrhnZmr(MG, DG, RS)                                                \
        shrhnZst(W(RS), W(MG), W(DG))

/* mul (G = G * S)
 * set-flags: undefined */

#define mulhx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x1B007C00 | MRM(REG(RG), REG(RG), TIxx))

#define mulhx_rr(RG, RS)                                                    \
        EMITW(0x1B007C00 | MRM(REG(RG), REG(RG), REG(RS)))

#define mulhx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MS), VHL(DS), B1(DS), P1(DS)))  \
        EMITW(0x1B007C00 | MRM(REG(RG), REG(RG), TMxx))


#define mulhx_xr(RS)     /* Reax is in/out, Redx is out(high)-zero-ext */   \
        EMITW(0x1B007C00 | MRM(Teax,    Teax,    REG(RS)))                  \
        EMITW(0x53007C00 | MRM(Tedx,    Teax,    0x00) | 16 << 16)

#define mulhx_xm(MS, DS) /* Reax is in/out, Redx is out(high)-zero-ext */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MS), VHL(DS), B1(DS), P1(DS)))  \
        EMITW(0x1B007C00 | MRM(Teax,    Teax,    TMxx))                     \
        EMITW(0x53007C00 | MRM(Tedx,    Teax,    0x00) | 16 << 16)


#define mulhn_xr(RS)     /* Reax is in/out, Redx is out(high)-sign-ext */   \
        EMITW(0x1B007C00 | MRM(Teax,    Teax,    REG(RS)))                  \
        EMITW(0x13007C00 | MRM(Tedx,    Teax,    0x00) | 16 << 16)

#define mulhn_xm(MS, DS) /* Reax is in/out, Redx is out(high)-sign-ext */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78800000 | MDM(TMxx,    MOD(MS), VHL(DS), B1(DS), P1(DS)))  \
        EMITW(0x1B007C00 | MRM(Teax,    Teax,    TMxx))                     \
        EMITW(0x13007C00 | MRM(Tedx,    Teax,    0x00) | 16 << 16)

/* div (G = G / S)
 * set-flags: undefined */

#define divhx_ri(RG, IS)       /* Reax cannot be used as first operand */   \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x53003C00 | MRM(TIxx,    TIxx,    0x00))                     \
        EMITW(0x53003C00 | MRM(REG(RG), REG(RG), 0x00))                     \
        EMITW(0x1AC00800 | MRM(REG(RG), REG(RG), TIxx))

#define divhx_rr(RG, RS)                /* RG no Reax, RS no Reax/Redx */   \
        EMITW(0x53003C00 | MRM(TIxx,    REG(RS), 0x00))                     \
        EMITW(0x53003C00 | MRM(REG(RG), REG(RG), 0x00))                     \
        EMITW(0x1AC00800 | MRM(REG(RG), REG(RG), TIxx))

#define divhx_ld(RG, MS, DS)            /* RG no Reax, MS no Oeax/Medx */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MS), VHL(DS), B1(DS), P1(DS)))  \
        EMITW(0x53003C00 | MRM(REG(RG), REG(RG), 0x00))                     \
        EMITW(0x1AC00800 | MRM(REG(RG), REG(RG), TMxx))


#define divhn_ri(RG, IS)       /* Reax cannot be used as first operand */   \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x13003C00 | MRM(TIxx,    TIxx,    0x00))                     \
        EMITW(0x13003C00 | MRM(REG(RG), REG(RG), 0x00))                     \
        EMITW(0x1AC00C00 | MRM(REG(RG), REG(RG), TIxx))

#define divhn_rr(RG, RS)                /* RG no Reax, RS no Reax/Redx */   \
        EMITW(0x13003C00 | MRM(TIxx,    REG(RS), 0x00))                     \
        EMITW(0x13003C00 | MRM(REG(RG), REG(RG), 0x00))                     \
        EMITW(0x1AC00C00 | MRM(REG(RG), REG(RG), TIxx))

#define divhn_ld(RG, MS, DS)            /* RG no Reax, MS no Oeax/Medx */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78800000 | MDM(TMxx,    MOD(MS), VHL(DS), B1(DS), P1(DS)))  \
        EMITW(0x13003C00 | MRM(REG(RG), REG(RG), 0x00))                     \
        EMITW(0x1AC00C00 | MRM(REG(RG), REG(RG), TMxx))


#define prehx_xx()   /* to be placed right before divhx_x* or remhx_xx */   \
                                     /* to prepare Redx for int-divide */

#define prehn_xx()   /* to be placed right before divhn_x* or remhn_xx */   \
                                     /* to prepare Redx for int-divide */


#define divhx_xr(RS)     /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        EMITW(0x53003C00 | MRM(TIxx,    REG(RS), 0x00))                     \
        EMITW(0x53003C00 | MRM(Teax,    Teax,    0x00))                     \
        EMITW(0x1AC00800 | MRM(Teax,    Teax,    TIxx))

#define divhx_xm(MS, DS) /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MS), VHL(DS), B1(DS), P1(DS)))  \
        EMITW(0x53003C00 | MRM(Teax,    Teax,    0x00))                     \
        EMITW(0x1AC00800 | MRM(Teax,    Teax,    TMxx))


#define divhn_xr(RS)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        EMITW(0x13003C00 | MRM(TIxx,    REG(RS), 0x00))                     \
        EMITW(0x13003C00 | MRM(Teax,    Teax,    0x00))                     \
        EMITW(0x1AC00C00 | MRM(Teax,    Teax,    TIxx))

#define divhn_xm(MS, DS) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78800000 | MDM(TMxx,    MOD(MS), VHL(DS), B1(DS), P1(DS)))  \
        EMITW(0x13003C00 | MRM(Teax,    Teax,    0x00))                     \
        EMITW(0x1AC00C00 | MRM(Teax,    Teax,    TMxx))

/* rem (G = G % S)
 * set-flags: undefined */

#define remhx_ri(RG, IS)       /* Redx cannot be used as first operand */   \
        EMITW(0x53003C00 | MRM(TMxx,    REG(RG), 0x00))                     \
        divhx_ri(W(RG), W(IS))                                              \
        EMITW(0x1B008000 | MRM(REG(RG), REG(RG), TIxx) | TMxx << 10)

#define remhx_rr(RG, RS)                /* RG no Redx, RS no Reax/Redx */   \
        EMITW(0x53003C00 | MRM(TMxx,    REG(RG), 0x00))                     \
        divhx_rr(W(RG), W(RS))                                              \
        EMITW(0x1B008000 | MRM(REG(RG), REG(RG), TIxx) | TMxx << 10)

#define remhx_ld(RG, MS, DS)            /* RG no Redx, MS no Oeax/Medx */   \
        EMITW(0x53003C00 | MRM(TIxx,    REG(RG), 0x00))                     \
        divhx_ld(W(RG), W(MS), W(DS))                                       \
        EMITW(0x1B008000 | MRM(REG(RG), REG(RG), TMxx) | TIxx << 10)


#define remhn_ri(RG, IS)       /* Redx cannot be used as first operand */   \
        EMITW(0x13003C00 | MRM(TMxx,    REG(RG), 0x00))                     \
        divhn_ri(W(RG), W(IS))                                              \
        EMITW(0x1B008000 | MRM(REG(RG), REG(RG), TIxx) | TMxx << 10)

#define remhn_rr(RG, RS)                /* RG no Redx, RS no Reax/Redx */   \
        EMITW(0x13003C00 | MRM(TMxx,    REG(RG), 0x00))                     \
        divhn_rr(W(RG), W(RS))                                              \
        EMITW(0x1B008000 | MRM(REG(RG), REG(RG), TIxx) | TMxx << 10)

#define remhn_ld(RG, MS, DS)            /* RG no Redx, MS no Oeax/Medx */   \
        EMITW(0x13003C00 | MRM(TIxx,    REG(RG), 0x00))                     \
        divhn_ld(W(RG), W(MS), W(DS))                                       \
        EMITW(0x1B008000 | MRM(REG(RG), REG(RG), TMxx) | TIxx << 10)


#define remhx_xx() /* to be placed before divhx_x*, but after prehx_xx */   \
        movhz_rr(Redx, Reax)         /* to prepare for rem calculation */

#define remhx_xr(RS)        /* to be placed immediately after divhx_xr */   \
        EMITW(0x1B008000 | MRM(Tedx,    Teax,    TIxx) | Tedx << 10)        \
                                                          /* Redx<-rem */

#define remhx_xm(MS, DS)    /* to be placed immediately after divhx_xm */   \
        EMITW(0x1B008000 | MRM(Tedx,    Teax,    TMxx) | Tedx << 10)        \
                                                          /* Redx<-rem */


#define remhn_xx() /* to be placed before divhn_x*, but after prehn_xx */   \
        movhn_rr(Redx, Reax)         /* to prepare for rem calculation */

#define remhn_xr(RS)        /* to be placed immediately after divhn_xr */   \
        EMITW(0x1B008000 | MRM(Tedx,    Teax,    TIxx) | Tedx << 10)        \
                                                          /* Redx<-rem */

#define remhn_xm(MS, DS)    /* to be placed immediately after divhn_xm */   \
        EMITW(0x1B008000 | MRM(Tedx,    Teax,    TMxx) | Tedx << 10)        \
                                                          /* Redx<-rem */

/* arj (G = G op S, if cc G then jump lb)
 * set-flags: undefined
 * refer to individual instruction descriptions
 * to stay within special register limitations */

     /* Definitions for arj's "op" and "cc" parameters
      * are provided in 32-bit rtarch_***.h files. */

#define arjhx_rx(RG, op, cc, lb)                                            \
        AR1(W(RG), op, h, Zrx)                                              \
        CMJ(cc, lb)

#define arjhx_mx(MG, DG, op, cc, lb)                                        \
        AR2(W(MG), W(DG), op, h, Zmx)                                       \
        CMJ(cc, lb)

#define arjhx_ri(RG, IS, op, cc, lb)                                        \
        AR2(W(RG), W(IS), op, h, Zri)                                       \
        CMJ(cc, lb)

#define arjhx_mi(MG, DG, IS, op, cc, lb)                                    \
        AR3(W(MG), W(DG), W(IS), op, h, Zmi)                                \
        CMJ(cc, lb)

#define arjhx_rr(RG, RS, op, cc, lb)                                        \
        AR2(W(RG), W(RS), op, h, Zrr)                                       \
        CMJ(cc, lb)

#define arjhx_ld(RG, MS, DS, op, cc, lb)                                    \
        AR3(W(RG), W(MS), W(DS), op, h, Zld)                                \
        CMJ(cc, lb)

#define arjhx_st(RS, MG, DG, op, cc, lb)                                    \
        AR3(W(RS), W(MG), W(DG), op, h, Zst)                                \
        CMJ(cc, lb)

#define arjhx_mr(MG, DG, RS, op, cc, lb)                                    \
        arjhx_st(W(RS), W(MG), W(DG), op, cc, lb)

/* cmj (flags = S ? T, if cc flags then jump lb)
 * set-flags: undefined */

     /* Definitions for cmj's "cc" parameter
      * are provided in 32-bit rtarch_***.h files. */

#define cmjhx_rz(RS, cc, lb)                                                \
        EMITW(0x13003C00 | MRM(TMxx,    REG(RS), 0x00))                     \
        EMITW(0x71000000 | MRM(TZxx,    TMxx,    0x00))                     \
        CMJ(cc, lb)

#define cmjhx_mz(MS, DS, cc, lb)                                            \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78C00000 | MDM(TMxx,    MOD(MS), VHL(DS), B1(DS), P1(DS)))  \
        EMITW(0x71000000 | MRM(TZxx,    TMxx,    0x00))                     \
        CMJ(cc, lb)

#define cmjhx_ri(RS, IT, cc, lb)                                            \
        cmphx_ri(W(RS), W(IT))                                              \
        CMJ(cc, lb)

#define cmjhx_mi(MS, DS, IT, cc, lb)                                        \
        cmphx_mi(W(MS), W(DS), W(IT))                                       \
        CMJ(cc, lb)

#define cmjhx_rr(RS, RT, cc, lb)                                            \
        cmphx_rr(W(RS), W(RT))                                              \
        CMJ(cc, lb)

#define cmjhx_rm(RS, MT, DT, cc, lb)                                        \
        cmphx_rm(W(RS), W(MT), W(DT))                                       \
        CMJ(cc, lb)

#define cmjhx_mr(MS, DS, RT, cc, lb)                                        \
        cmphx_mr(W(MS), W(DS), W(RT))                                       \
        CMJ(cc, lb)

/* cmp (flags = S ? T)
 * set-flags: yes */

#define cmphx_ri(RS, IT)                                                    \
        AUW(EMPTY,    VAL(IT), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IT))   \
        EMITW(0x13003C00 | MRM(TIxx,    TIxx,    0x00))                     \
        EMITW(0x13003C00 | MRM(TMxx,    REG(RS), 0x00))                     \
        EMITW(0x6B000000 | MRM(TZxx,    TMxx,    TIxx))

#define cmphx_mi(MS, DS, IT)                                                \
        AUW(SIB(MS),  VAL(IT), TIxx,    MOD(MS), VAL(DS), C1(DS), G3(IT))   \
        EMITW(0x78C00000 | MDM(TMxx,    MOD(MS), VHL(DS), B1(DS), P1(DS)))  \
        EMITW(0x13003C00 | MRM(TIxx,    TIxx,    0x00))                     \
        EMITW(0x6B000000 | MRM(TZxx,    TMxx,    TIxx))

#define cmphx_rr(RS, RT)                                                    \
        EMITW(0x13003C00 | MRM(TIxx,    REG(RT), 0x00))                     \
        EMITW(0x13003C00 | MRM(TMxx,    REG(RS), 0x00))                     \
        EMITW(0x6B000000 | MRM(TZxx,    TMxx,    TIxx))

#define cmphx_rm(RS, MT, DT)                                                \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x78C00000 | MDM(TMxx,    MOD(MT), VHL(DT), B1(DT), P1(DT)))  \
        EMITW(0x13003C00 | MRM(TIxx,    REG(RS), 0x00))                     \
        EMITW(0x6B000000 | MRM(TZxx,    TIxx,    TMxx))

#define cmphx_mr(MS, DS, RT)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78C00000 | MDM(TMxx,    MOD(MS), VHL(DS), B1(DS), P1(DS)))  \
        EMITW(0x13003C00 | MRM(TIxx,    REG(RT), 0x00))                     \
        EMITW(0x6B000000 | MRM(TZxx,    TMxx,    TIxx))

/************************** 8-bit subset instructions *************************/

/* mov (D = S)
 * set-flags: no */

#define movbx_ri(RD, IS)                                                    \
        AUW(EMPTY,    VAL(IS), REG(RD), EMPTY,   EMPTY,   EMPTY2, G3(IS))

#define movbx_mi(MD, DD, IS)                                                \
        AUW(SIB(MD),  VAL(IS), TIxx,    MOD(MD), VAL(DD), C1(DD), G3(IS))   \
        EMITW(0x38000000 | MDM(TIxx,    MOD(MD), VBL(DD), B1(DD), P1(DD)))

#define movbx_rr(RD, RS)                                                    \
        EMITW(0x2A000000 | MRM(REG(RD), TZxx,    REG(RS)))

#define movbx_ld(RD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x38400000 | MDM(REG(RD), MOD(MS), VBL(DS), B1(DS), P1(DS)))

#define movbx_st(RS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), C1(DD), EMPTY2)   \
        EMITW(0x38000000 | MDM(REG(RS), MOD(MD), VBL(DD), B1(DD), P1(DD)))

/* and (G = G & S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define andbx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)))

#define andbx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x38400000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0x38000000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))

#define andbx_rr(RG, RS)                                                    \
        EMITW(0x0A000000 | MRM(REG(RG), REG(RG), REG(RS)))

#define andbx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x38400000 | MDM(TMxx,    MOD(MS), VBL(DS), B1(DS), P1(DS)))  \
        EMITW(0x0A000000 | MRM(REG(RG), REG(RG), TMxx))

#define andbx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x38400000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x0A000000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x38000000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))

#define andbx_mr(MG, DG, RS)                                                \
        andbx_st(W(RS), W(MG), W(DG))


#define andbxZri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0x72001C00 | MRM(REG(RG), REG(RG), 0x00))/* <- set flags (Z) */

#define andbxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x38400000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x60000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0x38000000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))

#define andbxZrr(RG, RS)                                                    \
        EMITW(0x0A000000 | MRM(REG(RG), REG(RG), REG(RS)))                  \
        EMITW(0x72001C00 | MRM(REG(RG), REG(RG), 0x00))/* <- set flags (Z) */

#define andbxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x38400000 | MDM(TMxx,    MOD(MS), VBL(DS), B1(DS), P1(DS)))  \
        EMITW(0x6A000000 | MRM(REG(RG), REG(RG), TMxx))

#define andbxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x38400000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x6A000000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x38000000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))

#define andbxZmr(MG, DG, RS)                                                \
        andbxZst(W(RS), W(MG), W(DG))

/* ann (G = ~G & S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define annbx_ri(RG, IS)                                                    \
        notbx_rx(W(RG))                                                     \
        andbx_ri(W(RG), W(IS))

#define annbx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x38400000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x2A200000 | MRM(TMxx,    TZxx,    TMxx))                     \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0x38000000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))

#define annbx_rr(RG, RS)                                                    \
        EMITW(0x0A200000 | MRM(REG(RG), REG(RS), REG(RG)))

#define annbx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x38400000 | MDM(TMxx,    MOD(MS), VBL(DS), B1(DS), P1(DS)))  \
        EMITW(0x0A200000 | MRM(REG(RG), TMxx,    REG(RG)))

#define annbx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x38400000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x0A200000 | MRM(TMxx,    REG(RS), TMxx))                     \
        EMITW(0x38000000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))

#define annbx_mr(MG, DG, RS)                                                \
        annbx_st(W(RS), W(MG), W(DG))


#define annbxZri(RG, IS)                                                    \
        notbx_rx(W(RG))                                                     \
        andbxZri(W(RG), W(IS))

#define annbxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x38400000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x2A200000 | MRM(TMxx,    TZxx,    TMxx))                     \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0x38000000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x72001C00 | MRM(TMxx,    TMxx,    0x00))/* <- set flags (Z) */

#define annbxZrr(RG, RS)                                                    \
        EMITW(0x0A200000 | MRM(REG(RG), REG(RS), REG(RG)))                  \
        EMITW(0x72001C00 | MRM(REG(RG), REG(RG), 0x00))/* <- set flags (Z) */

#define annbxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x38400000 | MDM(TMxx,    MOD(MS), VBL(DS), B1(DS), P1(DS)))  \
        EMITW(0x6A200000 | MRM(REG(RG), TMxx,    REG(RG)))

#define annbxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x38400000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x0A200000 | MRM(TMxx,    REG(RS), TMxx))                     \
        EMITW(0x38000000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x72001C00 | MRM(TMxx,    TMxx,    0x00))/* <- set flags (Z) */

#define annbxZmr(MG, DG, RS)                                                \
        annbxZst(W(RS), W(MG), W(DG))

/* orr (G = G | S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define orrbx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0x20000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)))

#define orrbx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x38400000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x20000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0x38000000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))

#define orrbx_rr(RG, RS)                                                    \
        EMITW(0x2A000000 | MRM(REG(RG), REG(RG), REG(RS)))

#define orrbx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x38400000 | MDM(TMxx,    MOD(MS), VBL(DS), B1(DS), P1(DS)))  \
        EMITW(0x2A000000 | MRM(REG(RG), REG(RG), TMxx))

#define orrbx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x38400000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x2A000000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x38000000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))

#define orrbx_mr(MG, DG, RS)                                                \
        orrbx_st(W(RS), W(MG), W(DG))


#define orrbxZri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0x20000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0x72001C00 | MRM(REG(RG), REG(RG), 0x00))/* <- set flags (Z) */

#define orrbxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x38400000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x20000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0x38000000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x72001C00 | MRM(TMxx,    TMxx,    0x00))/* <- set flags (Z) */

#define orrbxZrr(RG, RS)                                                    \
        EMITW(0x2A000000 | MRM(REG(RG), REG(RG), REG(RS)))                  \
        EMITW(0x72001C00 | MRM(REG(RG), REG(RG), 0x00))/* <- set flags (Z) */

#define orrbxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x38400000 | MDM(TMxx,    MOD(MS), VBL(DS), B1(DS), P1(DS)))  \
        EMITW(0x2A000000 | MRM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0x72001C00 | MRM(REG(RG), REG(RG), 0x00))/* <- set flags (Z) */

#define orrbxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x38400000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x2A000000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x38000000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x72001C00 | MRM(TMxx,    TMxx,    0x00))/* <- set flags (Z) */

#define orrbxZmr(MG, DG, RS)                                                \
        orrbxZst(W(RS), W(MG), W(DG))

/* orn (G = ~G | S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define ornbx_ri(RG, IS)                                                    \
        notbx_rx(W(RG))                                                     \
        orrbx_ri(W(RG), W(IS))

#define ornbx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x38400000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x2A200000 | MRM(TMxx,    TZxx,    TMxx))                     \
        EMITW(0x20000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0x38000000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))

#define ornbx_rr(RG, RS)                                                    \
        EMITW(0x2A200000 | MRM(REG(RG), REG(RS), REG(RG)))

#define ornbx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x38400000 | MDM(TMxx,    MOD(MS), VBL(DS), B1(DS), P1(DS)))  \
        EMITW(0x2A200000 | MRM(REG(RG), TMxx,    REG(RG)))

#define ornbx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x38400000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x2A200000 | MRM(TMxx,    REG(RS), TMxx))                     \
        EMITW(0x38000000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))

#define ornbx_mr(MG, DG, RS)                                                \
        ornbx_st(W(RS), W(MG), W(DG))


#define ornbxZri(RG, IS)                                                    \
        notbx_rx(W(RG))                                                     \
        orrbxZri(W(RG), W(IS))

#define ornbxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x38400000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x2A200000 | MRM(TMxx,    TZxx,    TMxx))                     \
        EMITW(0x20000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0x38000000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x72001C00 | MRM(TMxx,    TMxx,    0x00))/* <- set flags (Z) */

#define ornbxZrr(RG, RS)                                                    \
        EMITW(0x2A200000 | MRM(REG(RG), REG(RS), REG(RG)))                  \
        EMITW(0x72001C00 | MRM(REG(RG), REG(RG), 0x00))/* <- set flags (Z) */

#define ornbxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x38400000 | MDM(TMxx,    MOD(MS), VBL(DS), B1(DS), P1(DS)))  \
        EMITW(0x2A200000 | MRM(REG(RG), TMxx,    REG(RG)))                  \
        EMITW(0x72001C00 | MRM(REG(RG), REG(RG), 0x00))/* <- set flags (Z) */

#define ornbxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x38400000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x2A200000 | MRM(TMxx,    REG(RS), TMxx))                     \
        EMITW(0x38000000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x72001C00 | MRM(TMxx,    TMxx,    0x00))/* <- set flags (Z) */

#define ornbxZmr(MG, DG, RS)                                                \
        ornbxZst(W(RS), W(MG), W(DG))

/* xor (G = G ^ S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define xorbx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0x40000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)))

#define xorbx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x38400000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x40000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0x38000000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))

#define xorbx_rr(RG, RS)                                                    \
        EMITW(0x4A000000 | MRM(REG(RG), REG(RG), REG(RS)))

#define xorbx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x38400000 | MDM(TMxx,    MOD(MS), VBL(DS), B1(DS), P1(DS)))  \
        EMITW(0x4A000000 | MRM(REG(RG), REG(RG), TMxx))

#define xorbx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x38400000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x4A000000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x38000000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))

#define xorbx_mr(MG, DG, RS)                                                \
        xorbx_st(W(RS), W(MG), W(DG))


#define xorbxZri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0x40000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0x72001C00 | MRM(REG(RG), REG(RG), 0x00))/* <- set flags (Z) */

#define xorbxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x38400000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x40000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0x38000000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x72001C00 | MRM(TMxx,    TMxx,    0x00))/* <- set flags (Z) */

#define xorbxZrr(RG, RS)                                                    \
        EMITW(0x4A000000 | MRM(REG(RG), REG(RG), REG(RS)))                  \
        EMITW(0x72001C00 | MRM(REG(RG), REG(RG), 0x00))/* <- set flags (Z) */

#define xorbxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x38400000 | MDM(TMxx,    MOD(MS), VBL(DS), B1(DS), P1(DS)))  \
        EMITW(0x4A000000 | MRM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0x72001C00 | MRM(REG(RG), REG(RG), 0x00))/* <- set flags (Z) */

#define xorbxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x38400000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x4A000000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x38000000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x72001C00 | MRM(TMxx,    TMxx,    0x00))/* <- set flags (Z) */

#define xorbxZmr(MG, DG, RS)                                                \
        xorbxZst(W(RS), W(MG), W(DG))

/* not (G = ~G)
 * set-flags: no */

#define notbx_rx(RG)                                                        \
        EMITW(0x2A200000 | MRM(REG(RG), TZxx,    REG(RG)))

#define notbx_mx(MG, DG)                                                    \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x38400000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x2A200000 | MRM(TMxx,    TZxx,    TMxx))                     \
        EMITW(0x38000000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))

/* neg (G = -G)
 * set-flags: undefined (*_*), yes (*Z*) */

#define negbx_rx(RG)                                                        \
        EMITW(0x4B000000 | MRM(REG(RG), TZxx,    REG(RG)))

#define negbx_mx(MG, DG)                                                    \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x38400000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x4B000000 | MRM(TMxx,    TZxx,    TMxx))                     \
        EMITW(0x38000000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))


#define negbxZrx(RG)                                                        \
        EMITW(0x4B000000 | MRM(REG(RG), TZxx,    REG(RG)))                  \
        EMITW(0x72001C00 | MRM(REG(RG), REG(RG), 0x00))/* <- set flags (Z) */

#define negbxZmx(MG, DG)                                                    \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x38400000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x6B000000 | MRM(TMxx,    TZxx,    TMxx))                     \
        EMITW(0x38000000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))

/* add (G = G + S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define addbx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0x01000000 | MIM(REG(RG), REG(RG), VAL(IS), T1(IS), M1(IS)))

#define addbx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G1(IS))   \
        EMITW(0x38400000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x01000000 | MIM(TMxx,    TMxx,    VAL(IS), T1(IS), M1(IS)))  \
        EMITW(0x38000000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))

#define addbx_rr(RG, RS)                                                    \
        EMITW(0x0B000000 | MRM(REG(RG), REG(RG), REG(RS)))

#define addbx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x38400000 | MDM(TMxx,    MOD(MS), VBL(DS), B1(DS), P1(DS)))  \
        EMITW(0x0B000000 | MRM(REG(RG), REG(RG), TMxx))

#define addbx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x38400000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x0B000000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x38000000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))

#define addbx_mr(MG, DG, RS)                                                \
        addbx_st(W(RS), W(MG), W(DG))


#define addbxZri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0x01000000 | MIM(REG(RG), REG(RG), VAL(IS), T1(IS), M1(IS)))  \
        EMITW(0x72001C00 | MRM(REG(RG), REG(RG), 0x00))/* <- set flags (Z) */

#define addbxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G1(IS))   \
        EMITW(0x38400000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x01000000 | MIM(TMxx,    TMxx,    VAL(IS), T1(IS), M1(IS)))  \
        EMITW(0x38000000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x72001C00 | MRM(TMxx,    TMxx,    0x00))/* <- set flags (Z) */

#define addbxZrr(RG, RS)                                                    \
        EMITW(0x0B000000 | MRM(REG(RG), REG(RG), REG(RS)))                  \
        EMITW(0x72001C00 | MRM(REG(RG), REG(RG), 0x00))/* <- set flags (Z) */

#define addbxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x38400000 | MDM(TMxx,    MOD(MS), VBL(DS), B1(DS), P1(DS)))  \
        EMITW(0x0B000000 | MRM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0x72001C00 | MRM(REG(RG), REG(RG), 0x00))/* <- set flags (Z) */

#define addbxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x38400000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x0B000000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x38000000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x72001C00 | MRM(TMxx,    TMxx,    0x00))/* <- set flags (Z) */

#define addbxZmr(MG, DG, RS)                                                \
        addbxZst(W(RS), W(MG), W(DG))

/* sub (G = G - S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define subbx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0x41000000 | MIM(REG(RG), REG(RG), VAL(IS), T1(IS), M1(IS)))

#define subbx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G1(IS))   \
        EMITW(0x38400000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x41000000 | MIM(TMxx,    TMxx,    VAL(IS), T1(IS), M1(IS)))  \
        EMITW(0x38000000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))

#define subbx_rr(RG, RS)                                                    \
        EMITW(0x4B000000 | MRM(REG(RG), REG(RG), REG(RS)))

#define subbx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x38400000 | MDM(TMxx,    MOD(MS), VBL(DS), B1(DS), P1(DS)))  \
        EMITW(0x4B000000 | MRM(REG(RG), REG(RG), TMxx))

#define subbx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x38400000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x4B000000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x38000000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))

#define subbx_mr(MG, DG, RS)                                                \
        subbx_st(W(RS), W(MG), W(DG))


#define subbxZri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0x41000000 | MIM(REG(RG), REG(RG), VAL(IS), T1(IS), M1(IS)))  \
        EMITW(0x72001C00 | MRM(REG(RG), REG(RG), 0x00))/* <- set flags (Z) */

#define subbxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G1(IS))   \
        EMITW(0x38400000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x41000000 | MIM(TMxx,    TMxx,    VAL(IS), T1(IS), M1(IS)))  \
        EMITW(0x38000000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x72001C00 | MRM(TMxx,    TMxx,    0x00))/* <- set flags (Z) */

#define subbxZrr(RG, RS)                                                    \
        EMITW(0x4B000000 | MRM(REG(RG), REG(RG), REG(RS)))                  \
        EMITW(0x72001C00 | MRM(REG(RG), REG(RG), 0x00))/* <- set flags (Z) */

#define subbxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x38400000 | MDM(TMxx,    MOD(MS), VBL(DS), B1(DS), P1(DS)))  \
        EMITW(0x4B000000 | MRM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0x72001C00 | MRM(REG(RG), REG(RG), 0x00))/* <- set flags (Z) */

#define subbxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x38400000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x4B000000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x38000000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x72001C00 | MRM(TMxx,    TMxx,    0x00))/* <- set flags (Z) */

#define subbxZmr(MG, DG, RS)                                                \
        subbxZst(W(RS), W(MG), W(DG))

/* shl (G = G << S)
 * set-flags: undefined (*_*), yes (*Z*)
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlbx_rx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x1AC02000 | MRM(REG(RG), REG(RG), Tecx))

#define shlbx_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x38400000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x1AC02000 | MRM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0x38000000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))

#define shlbx_ri(RG, IS)                                                    \
        EMITW(0x53000000 | MRM(REG(RG), REG(RG), 0x00) |                    \
        (-VAL(IS)&0x1F)<<16 | (31-(VAL(IS)&0x1F))<<10)

#define shlbx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x38400000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x53000000 | MRM(TMxx,    TMxx,    0x00) |                    \
        (-VAL(IS)&0x1F)<<16 | (31-(VAL(IS)&0x1F))<<10)                      \
        EMITW(0x38000000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))

#define shlbx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x1AC02000 | MRM(REG(RG), REG(RG), REG(RS)))

#define shlbx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x38400000 | MDM(TMxx,    MOD(MS), VBL(DS), B1(DS), P1(DS)))  \
        EMITW(0x1AC02000 | MRM(REG(RG), REG(RG), TMxx))

#define shlbx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x38400000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x1AC02000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x38000000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))

#define shlbx_mr(MG, DG, RS)                                                \
        shlbx_st(W(RS), W(MG), W(DG))


#define shlbxZrx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x1AC02000 | MRM(REG(RG), REG(RG), Tecx))                     \
        EMITW(0x72001C00 | MRM(REG(RG), REG(RG), 0x00))/* <- set flags (Z) */

#define shlbxZmx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x38400000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x1AC02000 | MRM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0x38000000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x72001C00 | MRM(TMxx,    TMxx,    0x00))/* <- set flags (Z) */

#define shlbxZri(RG, IS)                                                    \
        EMITW(0x53000000 | MRM(REG(RG), REG(RG), 0x00) |                    \
        (-VAL(IS)&0x1F)<<16 | (31-(VAL(IS)&0x1F))<<10)                      \
        EMITW(0x72001C00 | MRM(REG(RG), REG(RG), 0x00))/* <- set flags (Z) */

#define shlbxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x38400000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x53000000 | MRM(TMxx,    TMxx,    0x00) |                    \
        (-VAL(IS)&0x1F)<<16 | (31-(VAL(IS)&0x1F))<<10)                      \
        EMITW(0x38000000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x72001C00 | MRM(TMxx,    TMxx,    0x00))/* <- set flags (Z) */

#define shlbxZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x1AC02000 | MRM(REG(RG), REG(RG), REG(RS)))                  \
        EMITW(0x72001C00 | MRM(REG(RG), REG(RG), 0x00))/* <- set flags (Z) */

#define shlbxZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x38400000 | MDM(TMxx,    MOD(MS), VBL(DS), B1(DS), P1(DS)))  \
        EMITW(0x1AC02000 | MRM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0x72001C00 | MRM(REG(RG), REG(RG), 0x00))/* <- set flags (Z) */

#define shlbxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x38400000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x1AC02000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x38000000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x72001C00 | MRM(TMxx,    TMxx,    0x00))/* <- set flags (Z) */

#define shlbxZmr(MG, DG, RS)                                                \
        shlbxZst(W(RS), W(MG), W(DG))

/* shr (G = G >> S), unsigned (logical)
 * set-flags: undefined (*_*), yes (*Z*)
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrbx_rx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x53001C00 | MRM(REG(RG), REG(RG), 0x00))                     \
        EMITW(0x1AC02400 | MRM(REG(RG), REG(RG), Tecx))

#define shrbx_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x38400000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x1AC02400 | MRM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0x38000000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))

#define shrbx_ri(RG, IS)                                                    \
        EMITW(0x53001C00 | MRM(REG(RG), REG(RG), 0x00))                     \
        EMITW(0x53007C00 | MRM(REG(RG), REG(RG), 0x00) |(VAL(IS)&0x1F)<<16)

#define shrbx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x38400000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x53007C00 | MRM(TMxx,    TMxx,    0x00) |(VAL(IS)&0x1F)<<16) \
        EMITW(0x38000000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))

#define shrbx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x53001C00 | MRM(REG(RG), REG(RG), 0x00))                     \
        EMITW(0x1AC02400 | MRM(REG(RG), REG(RG), REG(RS)))

#define shrbx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x38400000 | MDM(TMxx,    MOD(MS), VBL(DS), B1(DS), P1(DS)))  \
        EMITW(0x53001C00 | MRM(REG(RG), REG(RG), 0x00))                     \
        EMITW(0x1AC02400 | MRM(REG(RG), REG(RG), TMxx))

#define shrbx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x38400000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x1AC02400 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x38000000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))

#define shrbx_mr(MG, DG, RS)                                                \
        shrbx_st(W(RS), W(MG), W(DG))


#define shrbxZrx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x53001C00 | MRM(REG(RG), REG(RG), 0x00))                     \
        EMITW(0x1AC02400 | MRM(REG(RG), REG(RG), Tecx))                     \
        EMITW(0x72001C00 | MRM(REG(RG), REG(RG), 0x00))/* <- set flags (Z) */

#define shrbxZmx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x38400000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x1AC02400 | MRM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0x38000000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x72001C00 | MRM(TMxx,    TMxx,    0x00))/* <- set flags (Z) */

#define shrbxZri(RG, IS)                                                    \
        EMITW(0x53001C00 | MRM(REG(RG), REG(RG), 0x00))                     \
        EMITW(0x53007C00 | MRM(REG(RG), REG(RG), 0x00) |(VAL(IS)&0x1F)<<16) \
        EMITW(0x72001C00 | MRM(REG(RG), REG(RG), 0x00))/* <- set flags (Z) */

#define shrbxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x38400000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x53007C00 | MRM(TMxx,    TMxx,    0x00) |(VAL(IS)&0x1F)<<16) \
        EMITW(0x38000000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x72001C00 | MRM(TMxx,    TMxx,    0x00))/* <- set flags (Z) */

#define shrbxZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x53001C00 | MRM(REG(RG), REG(RG), 0x00))                     \
        EMITW(0x1AC02400 | MRM(REG(RG), REG(RG), REG(RS)))                  \
        EMITW(0x72001C00 | MRM(REG(RG), REG(RG), 0x00))/* <- set flags (Z) */

#define shrbxZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x38400000 | MDM(TMxx,    MOD(MS), VBL(DS), B1(DS), P1(DS)))  \
        EMITW(0x53001C00 | MRM(REG(RG), REG(RG), 0x00))                     \
        EMITW(0x1AC02400 | MRM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0x72001C00 | MRM(REG(RG), REG(RG), 0x00))/* <- set flags (Z) */

#define shrbxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x38400000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x1AC02400 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x38000000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x72001C00 | MRM(TMxx,    TMxx,    0x00))/* <- set flags (Z) */

#define shrbxZmr(MG, DG, RS)                                                \
        shrbxZst(W(RS), W(MG), W(DG))

/* shr (G = G >> S), signed (arithmetic)
 * set-flags: undefined (*_*), yes (*Z*)
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrbn_rx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x13001C00 | MRM(REG(RG), REG(RG), 0x00))                     \
        EMITW(0x1AC02800 | MRM(REG(RG), REG(RG), Tecx))

#define shrbn_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x38800000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x1AC02800 | MRM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0x38000000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))

#define shrbn_ri(RG, IS)                                                    \
        EMITW(0x13001C00 | MRM(REG(RG), REG(RG), 0x00))                     \
        EMITW(0x13007C00 | MRM(REG(RG), REG(RG), 0x00) |(VAL(IS)&0x1F)<<16)

#define shrbn_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x38800000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x13007C00 | MRM(TMxx,    TMxx,    0x00) |(VAL(IS)&0x1F)<<16) \
        EMITW(0x38000000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))

#define shrbn_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x13001C00 | MRM(REG(RG), REG(RG), 0x00))                     \
        EMITW(0x1AC02800 | MRM(REG(RG), REG(RG), REG(RS)))

#define shrbn_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x38400000 | MDM(TMxx,    MOD(MS), VBL(DS), B1(DS), P1(DS)))  \
        EMITW(0x13001C00 | MRM(REG(RG), REG(RG), 0x00))                     \
        EMITW(0x1AC02800 | MRM(REG(RG), REG(RG), TMxx))

#define shrbn_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x38800000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x1AC02800 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x38000000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))

#define shrbn_mr(MG, DG, RS)                                                \
        shrbn_st(W(RS), W(MG), W(DG))


#define shrbnZrx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x13001C00 | MRM(REG(RG), REG(RG), 0x00))                     \
        EMITW(0x1AC02800 | MRM(REG(RG), REG(RG), Tecx))                     \
        EMITW(0x72001C00 | MRM(REG(RG), REG(RG), 0x00))/* <- set flags (Z) */

#define shrbnZmx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x38800000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x1AC02800 | MRM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0x38000000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x72001C00 | MRM(TMxx,    TMxx,    0x00))/* <- set flags (Z) */

#define shrbnZri(RG, IS)                                                    \
        EMITW(0x13001C00 | MRM(REG(RG), REG(RG), 0x00))                     \
        EMITW(0x13007C00 | MRM(REG(RG), REG(RG), 0x00) |(VAL(IS)&0x1F)<<16) \
        EMITW(0x72001C00 | MRM(REG(RG), REG(RG), 0x00))/* <- set flags (Z) */

#define shrbnZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x38800000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x13007C00 | MRM(TMxx,    TMxx,    0x00) |(VAL(IS)&0x1F)<<16) \
        EMITW(0x38000000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x72001C00 | MRM(TMxx,    TMxx,    0x00))/* <- set flags (Z) */

#define shrbnZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x13001C00 | MRM(REG(RG), REG(RG), 0x00))                     \
        EMITW(0x1AC02800 | MRM(REG(RG), REG(RG), REG(RS)))                  \
        EMITW(0x72001C00 | MRM(REG(RG), REG(RG), 0x00))/* <- set flags (Z) */

#define shrbnZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x38400000 | MDM(TMxx,    MOD(MS), VBL(DS), B1(DS), P1(DS)))  \
        EMITW(0x13001C00 | MRM(REG(RG), REG(RG), 0x00))                     \
        EMITW(0x1AC02800 | MRM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0x72001C00 | MRM(REG(RG), REG(RG), 0x00))/* <- set flags (Z) */

#define shrbnZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x38800000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x1AC02800 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x38000000 | MDM(TMxx,    MOD(MG), VBL(DG), B1(DG), P1(DG)))  \
        EMITW(0x72001C00 | MRM(TMxx,    TMxx,    0x00))/* <- set flags (Z) */

#define shrbnZmr(MG, DG, RS)                                                \
        shrbnZst(W(RS), W(MG), W(DG))

/* mul (G = G * S)
 * set-flags: undefined */

#define mulbx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x1B007C00 | MRM(REG(RG), REG(RG), TIxx))

#define mulbx_rr(RG, RS)                                                    \
        EMITW(0x1B007C00 | MRM(REG(RG), REG(RG), REG(RS)))

#define mulbx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x38400000 | MDM(TMxx,    MOD(MS), VBL(DS), B1(DS), P1(DS)))  \
        EMITW(0x1B007C00 | MRM(REG(RG), REG(RG), TMxx))


#define mulbx_xr(RS)     /* Reax is in/out, Redx is out(high)-zero-ext */   \
        EMITW(0x1B007C00 | MRM(Teax,    Teax,    REG(RS)))                  \
        EMITW(0x53007C00 | MRM(Tedx,    Teax,    0x00) | 8 << 16)

#define mulbx_xm(MS, DS) /* Reax is in/out, Redx is out(high)-zero-ext */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x38400000 | MDM(TMxx,    MOD(MS), VBL(DS), B1(DS), P1(DS)))  \
        EMITW(0x1B007C00 | MRM(Teax,    Teax,    TMxx))                     \
        EMITW(0x53007C00 | MRM(Tedx,    Teax,    0x00) | 8 << 16)


#define mulbn_xr(RS)     /* Reax is in/out, Redx is out(high)-sign-ext */   \
        EMITW(0x1B007C00 | MRM(Teax,    Teax,    REG(RS)))                  \
        EMITW(0x13007C00 | MRM(Tedx,    Teax,    0x00) | 8 << 16)

#define mulbn_xm(MS, DS) /* Reax is in/out, Redx is out(high)-sign-ext */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x38800000 | MDM(TMxx,    MOD(MS), VBL(DS), B1(DS), P1(DS)))  \
        EMITW(0x1B007C00 | MRM(Teax,    Teax,    TMxx))                     \
        EMITW(0x13007C00 | MRM(Tedx,    Teax,    0x00) | 8 << 16)

/* div (G = G / S)
 * set-flags: undefined */

#define divbx_ri(RG, IS)       /* Reax cannot be used as first operand */   \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x53001C00 | MRM(TIxx,    TIxx,    0x00))                     \
        EMITW(0x53001C00 | MRM(REG(RG), REG(RG), 0x00))                     \
        EMITW(0x1AC00800 | MRM(REG(RG), REG(RG), TIxx))

#define divbx_rr(RG, RS)                /* RG no Reax, RS no Reax/Redx */   \
        EMITW(0x53001C00 | MRM(TIxx,    REG(RS), 0x00))                     \
        EMITW(0x53001C00 | MRM(REG(RG), REG(RG), 0x00))                     \
        EMITW(0x1AC00800 | MRM(REG(RG), REG(RG), TIxx))

#define divbx_ld(RG, MS, DS)            /* RG no Reax, MS no Oeax/Medx */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x38400000 | MDM(TMxx,    MOD(MS), VBL(DS), B1(DS), P1(DS)))  \
        EMITW(0x53001C00 | MRM(REG(RG), REG(RG), 0x00))                     \
        EMITW(0x1AC00800 | MRM(REG(RG), REG(RG), TMxx))


#define divbn_ri(RG, IS)       /* Reax cannot be used as first operand */   \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x13001C00 | MRM(TIxx,    TIxx,    0x00))                     \
        EMITW(0x13001C00 | MRM(REG(RG), REG(RG), 0x00))                     \
        EMITW(0x1AC00C00 | MRM(REG(RG), REG(RG), TIxx))

#define divbn_rr(RG, RS)                /* RG no Reax, RS no Reax/Redx */   \
        EMITW(0x13001C00 | MRM(TIxx,    REG(RS), 0x00))                     \
        EMITW(0x13001C00 | MRM(REG(RG), REG(RG), 0x00))                     \
        EMITW(0x1AC00C00 | MRM(REG(RG), REG(RG), TIxx))

#define divbn_ld(RG, MS, DS)            /* RG no Reax, MS no Oeax/Medx */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x38800000 | MDM(TMxx,    MOD(MS), VBL(DS), B1(DS), P1(DS)))  \
        EMITW(0x13001C00 | MRM(REG(RG), REG(RG), 0x00))                     \
        EMITW(0x1AC00C00 | MRM(REG(RG), REG(RG), TMxx))


#define prebx_xx()   /* to be placed right before divbx_x* or rembx_xx */   \
        EMITW(0x53001C00 | MRM(Teax,    Teax,    0x00))                     \
                                     /* to prepare Reax for int-divide */

#define prebn_xx()   /* to be placed right before divbn_x* or rembn_xx */   \
        EMITW(0x13001C00 | MRM(Teax,    Teax,    0x00))                     \
                                     /* to prepare Reax for int-divide */


#define divbx_xr(RS)     /* Reax is in/out, Reax is in-zero-ext-(Reax) */   \
        EMITW(0x53001C00 | MRM(TIxx,    REG(RS), 0x00))                     \
        EMITW(0x1AC00800 | MRM(Teax,    Teax,    TIxx))

#define divbx_xm(MS, DS) /* Reax is in/out, Reax is in-zero-ext-(Reax) */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x38400000 | MDM(TMxx,    MOD(MS), VBL(DS), B1(DS), P1(DS)))  \
        EMITW(0x1AC00800 | MRM(Teax,    Teax,    TMxx))


#define divbn_xr(RS)     /* Reax is in/out, Reax is in-sign-ext-(Reax) */   \
        EMITW(0x13001C00 | MRM(TIxx,    REG(RS), 0x00))                     \
        EMITW(0x1AC00C00 | MRM(Teax,    Teax,    TIxx))

#define divbn_xm(MS, DS) /* Reax is in/out, Reax is in-sign-ext-(Reax) */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x38800000 | MDM(TMxx,    MOD(MS), VBL(DS), B1(DS), P1(DS)))  \
        EMITW(0x1AC00C00 | MRM(Teax,    Teax,    TMxx))

/* rem (G = G % S)
 * set-flags: undefined */

#define rembx_ri(RG, IS)       /* Redx cannot be used as first operand */   \
        EMITW(0x53001C00 | MRM(TMxx,    REG(RG), 0x00))                     \
        divbx_ri(W(RG), W(IS))                                              \
        EMITW(0x1B008000 | MRM(REG(RG), REG(RG), TIxx) | TMxx << 10)

#define rembx_rr(RG, RS)                /* RG no Redx, RS no Reax/Redx */   \
        EMITW(0x53001C00 | MRM(TMxx,    REG(RG), 0x00))                     \
        divbx_rr(W(RG), W(RS))                                              \
        EMITW(0x1B008000 | MRM(REG(RG), REG(RG), TIxx) | TMxx << 10)

#define rembx_ld(RG, MS, DS)            /* RG no Redx, MS no Oeax/Medx */   \
        EMITW(0x53001C00 | MRM(TIxx,    REG(RG), 0x00))                     \
        divbx_ld(W(RG), W(MS), W(DS))                                       \
        EMITW(0x1B008000 | MRM(REG(RG), REG(RG), TMxx) | TIxx << 10)


#define rembn_ri(RG, IS)       /* Redx cannot be used as first operand */   \
        EMITW(0x13001C00 | MRM(TMxx,    REG(RG), 0x00))                     \
        divbn_ri(W(RG), W(IS))                                              \
        EMITW(0x1B008000 | MRM(REG(RG), REG(RG), TIxx) | TMxx << 10)

#define rembn_rr(RG, RS)                /* RG no Redx, RS no Reax/Redx */   \
        EMITW(0x13001C00 | MRM(TMxx,    REG(RG), 0x00))                     \
        divbn_rr(W(RG), W(RS))                                              \
        EMITW(0x1B008000 | MRM(REG(RG), REG(RG), TIxx) | TMxx << 10)

#define rembn_ld(RG, MS, DS)            /* RG no Redx, MS no Oeax/Medx */   \
        EMITW(0x13001C00 | MRM(TIxx,    REG(RG), 0x00))                     \
        divbn_ld(W(RG), W(MS), W(DS))                                       \
        EMITW(0x1B008000 | MRM(REG(RG), REG(RG), TMxx) | TIxx << 10)


#define rembx_xx() /* to be placed before divbx_x*, but after prebx_xx */   \
        movbz_rr(Redx, Reax)         /* to prepare for rem calculation */

#define rembx_xr(RS)        /* to be placed immediately after divbx_xr */   \
        EMITW(0x1B008000 | MRM(Tedx,    Teax,    TIxx) | Tedx << 10)        \
                                                          /* Redx<-rem */

#define rembx_xm(MS, DS)    /* to be placed immediately after divbx_xm */   \
        EMITW(0x1B008000 | MRM(Tedx,    Teax,    TMxx) | Tedx << 10)        \
                                                          /* Redx<-rem */


#define rembn_xx() /* to be placed before divbn_x*, but after prebn_xx */   \
        movbn_rr(Redx, Reax)         /* to prepare for rem calculation */

#define rembn_xr(RS)        /* to be placed immediately after divbn_xr */   \
        EMITW(0x1B008000 | MRM(Tedx,    Teax,    TIxx) | Tedx << 10)        \
                                                          /* Redx<-rem */

#define rembn_xm(MS, DS)    /* to be placed immediately after divbn_xm */   \
        EMITW(0x1B008000 | MRM(Tedx,    Teax,    TMxx) | Tedx << 10)        \
                                                          /* Redx<-rem */

/* arj (G = G op S, if cc G then jump lb)
 * set-flags: undefined
 * refer to individual instruction descriptions
 * to stay within special register limitations */

     /* Definitions for arj's "op" and "cc" parameters
      * are provided in 32-bit rtarch_***.h files. */

#define arjbx_rx(RG, op, cc, lb)                                            \
        AR1(W(RG), op, b, Zrx)                                              \
        CMJ(cc, lb)

#define arjbx_mx(MG, DG, op, cc, lb)                                        \
        AR2(W(MG), W(DG), op, b, Zmx)                                       \
        CMJ(cc, lb)

#define arjbx_ri(RG, IS, op, cc, lb)                                        \
        AR2(W(RG), W(IS), op, b, Zri)                                       \
        CMJ(cc, lb)

#define arjbx_mi(MG, DG, IS, op, cc, lb)                                    \
        AR3(W(MG), W(DG), W(IS), op, b, Zmi)                                \
        CMJ(cc, lb)

#define arjbx_rr(RG, RS, op, cc, lb)                                        \
        AR2(W(RG), W(RS), op, b, Zrr)                                       \
        CMJ(cc, lb)

#define arjbx_ld(RG, MS, DS, op, cc, lb)                                    \
        AR3(W(RG), W(MS), W(DS), op, b, Zld)                                \
        CMJ(cc, lb)

#define arjbx_st(RS, MG, DG, op, cc, lb)                                    \
        AR3(W(RS), W(MG), W(DG), op, b, Zst)                                \
        CMJ(cc, lb)

#define arjbx_mr(MG, DG, RS, op, cc, lb)                                    \
        arjbx_st(W(RS), W(MG), W(DG), op, cc, lb)

/* cmj (flags = S ? T, if cc flags then jump lb)
 * set-flags: undefined */

     /* Definitions for cmj's "cc" parameter
      * are provided in 32-bit rtarch_***.h files. */

#define cmjbx_rz(RS, cc, lb)                                                \
        EMITW(0x13001C00 | MRM(TMxx,    REG(RS), 0x00))                     \
        EMITW(0x71000000 | MRM(TZxx,    TMxx,    0x00))                     \
        CMJ(cc, lb)

#define cmjbx_mz(MS, DS, cc, lb)                                            \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x38C00000 | MDM(TMxx,    MOD(MS), VBL(DS), B1(DS), P1(DS)))  \
        EMITW(0x71000000 | MRM(TZxx,    TMxx,    0x00))                     \
        CMJ(cc, lb)

#define cmjbx_ri(RS, IT, cc, lb)                                            \
        cmpbx_ri(W(RS), W(IT))                                              \
        CMJ(cc, lb)

#define cmjbx_mi(MS, DS, IT, cc, lb)                                        \
        cmpbx_mi(W(MS), W(DS), W(IT))                                       \
        CMJ(cc, lb)

#define cmjbx_rr(RS, RT, cc, lb)                                            \
        cmpbx_rr(W(RS), W(RT))                                              \
        CMJ(cc, lb)

#define cmjbx_rm(RS, MT, DT, cc, lb)                                        \
        cmpbx_rm(W(RS), W(MT), W(DT))                                       \
        CMJ(cc, lb)

#define cmjbx_mr(MS, DS, RT, cc, lb)                                        \
        cmpbx_mr(W(MS), W(DS), W(RT))                                       \
        CMJ(cc, lb)

/* cmp (flags = S ? T)
 * set-flags: yes */

#define cmpbx_ri(RS, IT)                                                    \
        AUW(EMPTY,    VAL(IT), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IT))   \
        EMITW(0x13001C00 | MRM(TIxx,    TIxx,    0x00))                     \
        EMITW(0x13001C00 | MRM(TMxx,    REG(RS), 0x00))                     \
        EMITW(0x6B000000 | MRM(TZxx,    TMxx,    TIxx))

#define cmpbx_mi(MS, DS, IT)                                                \
        AUW(SIB(MS),  VAL(IT), TIxx,    MOD(MS), VAL(DS), C1(DS), G3(IT))   \
        EMITW(0x38C00000 | MDM(TMxx,    MOD(MS), VBL(DS), B1(DS), P1(DS)))  \
        EMITW(0x13001C00 | MRM(TIxx,    TIxx,    0x00))                     \
        EMITW(0x6B000000 | MRM(TZxx,    TMxx,    TIxx))

#define cmpbx_rr(RS, RT)                                                    \
        EMITW(0x13001C00 | MRM(TIxx,    REG(RT), 0x00))                     \
        EMITW(0x13001C00 | MRM(TMxx,    REG(RS), 0x00))                     \
        EMITW(0x6B000000 | MRM(TZxx,    TMxx,    TIxx))

#define cmpbx_rm(RS, MT, DT)                                                \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x38C00000 | MDM(TMxx,    MOD(MT), VBL(DT), B1(DT), P1(DT)))  \
        EMITW(0x13001C00 | MRM(TIxx,    REG(RS), 0x00))                     \
        EMITW(0x6B000000 | MRM(TZxx,    TIxx,    TMxx))

#define cmpbx_mr(MS, DS, RT)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x38C00000 | MDM(TMxx,    MOD(MS), VBL(DS), B1(DS), P1(DS)))  \
        EMITW(0x13001C00 | MRM(TIxx,    REG(RT), 0x00))                     \
        EMITW(0x6B000000 | MRM(TZxx,    TMxx,    TIxx))

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

#endif /* RT_RTARCH_AHB_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
