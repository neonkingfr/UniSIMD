/******************************************************************************/
/* Copyright (c) 2013-2021 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_XHB_H
#define RT_RTARCH_XHB_H

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_xHB.h: Implementation of x86_64 half+byte BASE instructions.
 *
 * This file is a part of the unified SIMD assembler framework (rtarch.h)
 * designed to be compatible with different processor architectures,
 * while maintaining strictly defined common API.
 *
 * Recommended naming scheme for instructions:
 *
 * cmdhx_ri - applies [cmd] to [r]egister from [i]mmediate
 * cmdhx_mi - applies [cmd] to [m]emory   from [i]mmediate
 * cmdhx_rz - applies [cmd] to [r]egister from [z]ero-arg
 * cmdhx_mz - applies [cmd] to [m]emory   from [z]ero-arg
 *
 * cmdhx_rm - applies [cmd] to [r]egister from [m]emory
 * cmdhx_ld - applies [cmd] as above
 * cmdhx_mr - applies [cmd] to [m]emory   from [r]egister
 * cmdhx_st - applies [cmd] as above (arg list as cmdhx_ld)
 *
 * cmdhx_rr - applies [cmd] to [r]egister from [r]egister
 * cmdhx_mm - applies [cmd] to [m]emory   from [m]emory
 * cmdhx_rx - applies [cmd] to [r]egister (one-operand cmd)
 * cmdhx_mx - applies [cmd] to [m]emory   (one-operand cmd)
 *
 * cmdhx_rx - applies [cmd] to [r]egister from x-register
 * cmdhx_mx - applies [cmd] to [m]emory   from x-register
 * cmdhx_xr - applies [cmd] to x-register from [r]egister
 * cmdhx_xm - applies [cmd] to x-register from [m]emory
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
    ESC REX(0,       RXB(RD)) EMITB(0xC7)                                   \
        MRM(0x00,    MOD(RD), REG(RD))   /* truncate IC with TYP below */   \
        AUX(EMPTY,   EMPTY,   EMITH(VAL(IS) & ((TYP(IS) << 6) - 1)))

#define movhx_mi(MD, DD, IS)                                                \
ADR ESC REX(0,       RXB(MD)) EMITB(0xC7)                                   \
        MRM(0x00,    MOD(MD), REG(MD))   /* truncate IC with TYP below */   \
        AUX(SIB(MD), CMD(DD), EMITH(VAL(IS) & ((TYP(IS) << 6) - 1)))

#define movhx_rr(RD, RS)                                                    \
        REX(RXB(RD), RXB(RS)) EMITB(0x8B)                                   \
        MRM(REG(RD), MOD(RS), REG(RS))

#define movhx_ld(RD, MS, DS)                                                \
    ADR REW(RXB(RD), RXB(MS)) EMITB(0x0F) EMITB(0xB7)                       \
        MRM(REG(RD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define movhn_ld(RD, MS, DS)                                                \
    ADR REW(RXB(RD), RXB(MS)) EMITB(0x0F) EMITB(0xBF)                       \
        MRM(REG(RD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define movhx_st(RS, MD, DD)                                                \
ADR ESC REX(RXB(RS), RXB(MD)) EMITB(0x89)                                   \
        MRM(REG(RS), MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)

/* and (G = G & S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define andhx_ri(RG, IS)                                                    \
        andhxZri(W(RG), W(IS))

#define andhx_mi(MG, DG, IS)                                                \
        andhxZmi(W(MG), W(DG), W(IS))

#define andhx_rr(RG, RS)                                                    \
        andhxZrr(W(RG), W(RS))

#define andhx_ld(RG, MS, DS)                                                \
        andhxZld(W(RG), W(MS), W(DS))

#define andhx_st(RS, MG, DG)                                                \
        andhxZst(W(RS), W(MG), W(DG))

#define andhx_mr(MG, DG, RS)                                                \
        andhx_st(W(RS), W(MG), W(DG))


#define andhxZri(RG, IS)                                                    \
    ESC REX(0,       RXB(RG)) EMITB(0x81)                                   \
        MRM(0x04,    MOD(RG), REG(RG))   /* truncate IC with TYP below */   \
        AUX(EMPTY,   EMPTY,   EMITH(VAL(IS) & ((TYP(IS) << 6) - 1)))

#define andhxZmi(MG, DG, IS)                                                \
ADR ESC REX(0,       RXB(MG)) EMITB(0x81)                                   \
        MRM(0x04,    MOD(MG), REG(MG))   /* truncate IC with TYP below */   \
        AUX(SIB(MG), CMD(DG), EMITH(VAL(IS) & ((TYP(IS) << 6) - 1)))

#define andhxZrr(RG, RS)                                                    \
        REX(RXB(RG), RXB(RS)) EMITB(0x23)                                   \
        MRM(REG(RG), MOD(RS), REG(RS))

#define andhxZld(RG, MS, DS)                                                \
ADR ESC REX(RXB(RG), RXB(MS)) EMITB(0x23)                                   \
        MRM(REG(RG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define andhxZst(RS, MG, DG)                                                \
ADR ESC REX(RXB(RS), RXB(MG)) EMITB(0x21)                                   \
        MRM(REG(RS), MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

#define andhxZmr(MG, DG, RS)                                                \
        andhxZst(W(RS), W(MG), W(DG))

/* ann (G = ~G & S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define annhx_ri(RG, IS)                                                    \
        annhxZri(W(RG), W(IS))

#define annhx_mi(MG, DG, IS)                                                \
        annhxZmi(W(MG), W(DG), W(IS))

#define annhx_rr(RG, RS)                                                    \
        annhxZrr(W(RG), W(RS))

#define annhx_ld(RG, MS, DS)                                                \
        annhxZld(W(RG), W(MS), W(DS))

#define annhx_st(RS, MG, DG)                                                \
        annhxZst(W(RS), W(MG), W(DG))

#define annhx_mr(MG, DG, RS)                                                \
        annhx_st(W(RS), W(MG), W(DG))


#define annhxZri(RG, IS)                                                    \
        nothx_rx(W(RG))                                                     \
        andhxZri(W(RG), W(IS))

#define annhxZmi(MG, DG, IS)                                                \
        nothx_mx(W(MG), W(DG))                                              \
        andhxZmi(W(MG), W(DG), W(IS))

#if RT_BASE_COMPAT_BMI < 2 /* 0 - generic, 1 - 3-op-VEX, 2 - BMI1+BMI2 */

#define annhxZrr(RG, RS)                                                    \
        nothx_rx(W(RG))                                                     \
        andhxZrr(W(RG), W(RS))

#else /* RT_BASE_COMPAT_BMI >= 2 */

#define annhxZrr(RG, RS)                                                    \
        VEX(RXB(RG), RXB(RS), REN(RG), 0, 0, 2) EMITB(0xF2)                 \
        MRM(REG(RG), MOD(RS), REG(RS))

#endif /* RT_BASE_COMPAT_BMI >= 2 */

#define annhxZld(RG, MS, DS)                                                \
        nothx_rx(W(RG))                                                     \
        andhxZld(W(RG), W(MS), W(DS))

#define annhxZst(RS, MG, DG)                                                \
        nothx_mx(W(MG), W(DG))                                              \
        andhxZst(W(RS), W(MG), W(DG))

#define annhxZmr(MG, DG, RS)                                                \
        annhxZst(W(RS), W(MG), W(DG))

/* orr (G = G | S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define orrhx_ri(RG, IS)                                                    \
        orrhxZri(W(RG), W(IS))

#define orrhx_mi(MG, DG, IS)                                                \
        orrhxZmi(W(MG), W(DG), W(IS))

#define orrhx_rr(RG, RS)                                                    \
        orrhxZrr(W(RG), W(RS))

#define orrhx_ld(RG, MS, DS)                                                \
        orrhxZld(W(RG), W(MS), W(DS))

#define orrhx_st(RS, MG, DG)                                                \
        orrhxZst(W(RS), W(MG), W(DG))

#define orrhx_mr(MG, DG, RS)                                                \
        orrhx_st(W(RS), W(MG), W(DG))


#define orrhxZri(RG, IS)                                                    \
    ESC REX(0,       RXB(RG)) EMITB(0x81)                                   \
        MRM(0x01,    MOD(RG), REG(RG))   /* truncate IC with TYP below */   \
        AUX(EMPTY,   EMPTY,   EMITH(VAL(IS) & ((TYP(IS) << 6) - 1)))

#define orrhxZmi(MG, DG, IS)                                                \
ADR ESC REX(0,       RXB(MG)) EMITB(0x81)                                   \
        MRM(0x01,    MOD(MG), REG(MG))   /* truncate IC with TYP below */   \
        AUX(SIB(MG), CMD(DG), EMITH(VAL(IS) & ((TYP(IS) << 6) - 1)))

#define orrhxZrr(RG, RS)                                                    \
        REX(RXB(RG), RXB(RS)) EMITB(0x0B)                                   \
        MRM(REG(RG), MOD(RS), REG(RS))

#define orrhxZld(RG, MS, DS)                                                \
ADR ESC REX(RXB(RG), RXB(MS)) EMITB(0x0B)                                   \
        MRM(REG(RG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define orrhxZst(RS, MG, DG)                                                \
ADR ESC REX(RXB(RS), RXB(MG)) EMITB(0x09)                                   \
        MRM(REG(RS), MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

#define orrhxZmr(MG, DG, RS)                                                \
        orrhxZst(W(RS), W(MG), W(DG))

/* orn (G = ~G | S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define ornhx_ri(RG, IS)                                                    \
        ornhxZri(W(RG), W(IS))

#define ornhx_mi(MG, DG, IS)                                                \
        ornhxZmi(W(MG), W(DG), W(IS))

#define ornhx_rr(RG, RS)                                                    \
        ornhxZrr(W(RG), W(RS))

#define ornhx_ld(RG, MS, DS)                                                \
        ornhxZld(W(RG), W(MS), W(DS))

#define ornhx_st(RS, MG, DG)                                                \
        ornhxZst(W(RS), W(MG), W(DG))

#define ornhx_mr(MG, DG, RS)                                                \
        ornhx_st(W(RS), W(MG), W(DG))


#define ornhxZri(RG, IS)                                                    \
        nothx_rx(W(RG))                                                     \
        orrhxZri(W(RG), W(IS))

#define ornhxZmi(MG, DG, IS)                                                \
        nothx_mx(W(MG), W(DG))                                              \
        orrhxZmi(W(MG), W(DG), W(IS))

#define ornhxZrr(RG, RS)                                                    \
        nothx_rx(W(RG))                                                     \
        orrhxZrr(W(RG), W(RS))

#define ornhxZld(RG, MS, DS)                                                \
        nothx_rx(W(RG))                                                     \
        orrhxZld(W(RG), W(MS), W(DS))

#define ornhxZst(RS, MG, DG)                                                \
        nothx_mx(W(MG), W(DG))                                              \
        orrhxZst(W(RS), W(MG), W(DG))

#define ornhxZmr(MG, DG, RS)                                                \
        ornhxZst(W(RS), W(MG), W(DG))

/* xor (G = G ^ S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define xorhx_ri(RG, IS)                                                    \
        xorhxZri(W(RG), W(IS))

#define xorhx_mi(MG, DG, IS)                                                \
        xorhxZmi(W(MG), W(DG), W(IS))

#define xorhx_rr(RG, RS)                                                    \
        xorhxZrr(W(RG), W(RS))

#define xorhx_ld(RG, MS, DS)                                                \
        xorhxZld(W(RG), W(MS), W(DS))

#define xorhx_st(RS, MG, DG)                                                \
        xorhxZst(W(RS), W(MG), W(DG))

#define xorhx_mr(MG, DG, RS)                                                \
        xorhx_st(W(RS), W(MG), W(DG))


#define xorhxZri(RG, IS)                                                    \
    ESC REX(0,       RXB(RG)) EMITB(0x81)                                   \
        MRM(0x06,    MOD(RG), REG(RG))   /* truncate IC with TYP below */   \
        AUX(EMPTY,   EMPTY,   EMITH(VAL(IS) & ((TYP(IS) << 6) - 1)))

#define xorhxZmi(MG, DG, IS)                                                \
ADR ESC REX(0,       RXB(MG)) EMITB(0x81)                                   \
        MRM(0x06,    MOD(MG), REG(MG))   /* truncate IC with TYP below */   \
        AUX(SIB(MG), CMD(DG), EMITH(VAL(IS) & ((TYP(IS) << 6) - 1)))

#define xorhxZrr(RG, RS)                                                    \
        REX(RXB(RG), RXB(RS)) EMITB(0x33)                                   \
        MRM(REG(RG), MOD(RS), REG(RS))

#define xorhxZld(RG, MS, DS)                                                \
ADR ESC REX(RXB(RG), RXB(MS)) EMITB(0x33)                                   \
        MRM(REG(RG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define xorhxZst(RS, MG, DG)                                                \
ADR ESC REX(RXB(RS), RXB(MG)) EMITB(0x31)                                   \
        MRM(REG(RS), MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

#define xorhxZmr(MG, DG, RS)                                                \
        xorhxZst(W(RS), W(MG), W(DG))

/* not (G = ~G)
 * set-flags: no */

#define nothx_rx(RG)                                                        \
        REX(0,       RXB(RG)) EMITB(0xF7)                                   \
        MRM(0x02,    MOD(RG), REG(RG))

#define nothx_mx(MG, DG)                                                    \
ADR ESC REX(0,       RXB(MG)) EMITB(0xF7)                                   \
        MRM(0x02,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

/* neg (G = -G)
 * set-flags: undefined (*_*), yes (*Z*) */

#define neghx_rx(RG)                                                        \
        neghxZrx(W(RG))

#define neghx_mx(MG, DG)                                                    \
        neghxZmx(W(MG), W(DG))


#define neghxZrx(RG)                                                        \
        REX(0,       RXB(RG)) EMITB(0xF7)                                   \
        MRM(0x03,    MOD(RG), REG(RG))

#define neghxZmx(MG, DG)                                                    \
ADR ESC REX(0,       RXB(MG)) EMITB(0xF7)                                   \
        MRM(0x03,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

/* add (G = G + S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define addhx_ri(RG, IS)                                                    \
        addhxZri(W(RG), W(IS))

#define addhx_mi(MG, DG, IS)                                                \
        addhxZmi(W(MG), W(DG), W(IS))

#define addhx_rr(RG, RS)                                                    \
        addhxZrr(W(RG), W(RS))

#define addhx_ld(RG, MS, DS)                                                \
        addhxZld(W(RG), W(MS), W(DS))

#define addhn_ld(RG, MS, DS)                                                \
        addhnZld(W(RG), W(MS), W(DS))

#define addhx_st(RS, MG, DG)                                                \
        addhxZst(W(RS), W(MG), W(DG))

#define addhx_mr(MG, DG, RS)                                                \
        addhx_st(W(RS), W(MG), W(DG))


#define addhxZri(RG, IS)                                                    \
    ESC REX(0,       RXB(RG)) EMITB(0x81)                                   \
        MRM(0x00,    MOD(RG), REG(RG))   /* truncate IC with TYP below */   \
        AUX(EMPTY,   EMPTY,   EMITH(VAL(IS) & ((TYP(IS) << 6) - 1)))

#define addhxZmi(MG, DG, IS)                                                \
ADR ESC REX(0,       RXB(MG)) EMITB(0x81)                                   \
        MRM(0x00,    MOD(MG), REG(MG))   /* truncate IC with TYP below */   \
        AUX(SIB(MG), CMD(DG), EMITH(VAL(IS) & ((TYP(IS) << 6) - 1)))

#define addhxZrr(RG, RS)                                                    \
        REX(RXB(RG), RXB(RS)) EMITB(0x03)                                   \
        MRM(REG(RG), MOD(RS), REG(RS))

#define addhxZld(RG, MS, DS)                                                \
ADR ESC REX(RXB(RG), RXB(MS)) EMITB(0x03)                                   \
        MRM(REG(RG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define addhnZld(RG, MS, DS)                                                \
ADR ESC REX(RXB(RG), RXB(MS)) EMITB(0x03)                                   \
        MRM(REG(RG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define addhxZst(RS, MG, DG)                                                \
ADR ESC REX(RXB(RS), RXB(MG)) EMITB(0x01)                                   \
        MRM(REG(RS), MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

#define addhxZmr(MG, DG, RS)                                                \
        addhxZst(W(RS), W(MG), W(DG))

/* sub (G = G - S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define subhx_ri(RG, IS)                                                    \
        subhxZri(W(RG), W(IS))

#define subhx_mi(MG, DG, IS)                                                \
        subhxZmi(W(MG), W(DG), W(IS))

#define subhx_rr(RG, RS)                                                    \
        subhxZrr(W(RG), W(RS))

#define subhx_ld(RG, MS, DS)                                                \
        subhxZld(W(RG), W(MS), W(DS))

#define subhn_ld(RG, MS, DS)                                                \
        subhnZld(W(RG), W(MS), W(DS))

#define subhx_st(RS, MG, DG)                                                \
        subhxZst(W(RS), W(MG), W(DG))

#define subhx_mr(MG, DG, RS)                                                \
        subhx_st(W(RS), W(MG), W(DG))


#define subhxZri(RG, IS)                                                    \
    ESC REX(0,       RXB(RG)) EMITB(0x81)                                   \
        MRM(0x05,    MOD(RG), REG(RG))   /* truncate IC with TYP below */   \
        AUX(EMPTY,   EMPTY,   EMITH(VAL(IS) & ((TYP(IS) << 6) - 1)))

#define subhxZmi(MG, DG, IS)                                                \
ADR ESC REX(0,       RXB(MG)) EMITB(0x81)                                   \
        MRM(0x05,    MOD(MG), REG(MG))   /* truncate IC with TYP below */   \
        AUX(SIB(MG), CMD(DG), EMITH(VAL(IS) & ((TYP(IS) << 6) - 1)))

#define subhxZrr(RG, RS)                                                    \
        REX(RXB(RG), RXB(RS)) EMITB(0x2B)                                   \
        MRM(REG(RG), MOD(RS), REG(RS))

#define subhxZld(RG, MS, DS)                                                \
ADR ESC REX(RXB(RG), RXB(MS)) EMITB(0x2B)                                   \
        MRM(REG(RG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define subhnZld(RG, MS, DS)                                                \
ADR ESC REX(RXB(RG), RXB(MS)) EMITB(0x2B)                                   \
        MRM(REG(RG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define subhxZst(RS, MG, DG)                                                \
ADR ESC REX(RXB(RS), RXB(MG)) EMITB(0x29)                                   \
        MRM(REG(RS), MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

#define subhxZmr(MG, DG, RS)                                                \
        subhxZst(W(RS), W(MG), W(DG))

/* shl (G = G << S)
 * set-flags: undefined (*_*), yes (*Z*)
 * for maximum compatibility: shift count must be modulo elem-size */

#if RT_BASE_COMPAT_BMI < 2 /* 0 - generic, 1 - 3-op-VEX, 2 - BMI1+BMI2 */

#define shlhx_rx(RG)                     /* reads Recx for shift count */   \
        shlhxZrx(W(RG))

#else /* RT_BASE_COMPAT_BMI >= 2 */

#define shlhx_rx(RG)                     /* reads Recx for shift count */   \
        VEX(RXB(RG), RXB(RG),    0x01, 0, 1, 2) EMITB(0xF7)                 \
        MRM(REG(RG), MOD(RG), REG(RG))

#endif /* RT_BASE_COMPAT_BMI >= 2 */

#define shlhx_mx(MG, DG)                 /* reads Recx for shift count */   \
        shlhxZmx(W(MG), W(DG))

#define shlhx_ri(RG, IS)                                                    \
        shlhxZri(W(RG), W(IS))

#define shlhx_mi(MG, DG, IS)                                                \
        shlhxZmi(W(MG), W(DG), W(IS))

#if RT_BASE_COMPAT_BMI < 2 /* 0 - generic, 1 - 3-op-VEX, 2 - BMI1+BMI2 */

#define shlhx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        shlhxZrr(W(RG), W(RS))

#else /* RT_BASE_COMPAT_BMI >= 2 */

#define shlhx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        VEX(RXB(RG), RXB(RG), REN(RS), 0, 1, 2) EMITB(0xF7)                 \
        MRM(REG(RG), MOD(RG), REG(RG))

#endif /* RT_BASE_COMPAT_BMI >= 2 */

#define shlhx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        shlhxZld(W(RG), W(MS), W(DS))

#define shlhx_st(RS, MG, DG)                                                \
        shlhxZst(W(RS), W(MG), W(DG))

#define shlhx_mr(MG, DG, RS)                                                \
        shlhx_st(W(RS), W(MG), W(DG))


#define shlhxZrx(RG)                     /* reads Recx for shift count */   \
        REX(0,       RXB(RG)) EMITB(0xD3)                                   \
        MRM(0x04,    MOD(RG), REG(RG))                                      \

#define shlhxZmx(MG, DG)                 /* reads Recx for shift count */   \
ADR ESC REX(0,       RXB(MG)) EMITB(0xD3)                                   \
        MRM(0x04,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

#define shlhxZri(RG, IS)                                                    \
        REX(0,       RXB(RG)) EMITB(0xC1)                                   \
        MRM(0x04,    MOD(RG), REG(RG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS)))

#define shlhxZmi(MG, DG, IS)                                                \
ADR ESC REX(0,       RXB(MG)) EMITB(0xC1)                                   \
        MRM(0x04,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMITB(VAL(IS)))

#define shlhxZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movhx_rr(Recx, W(RS))                                               \
        shlhxZrx(W(RG))                                                     \
        stack_ld(Recx)

#define shlhxZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movhx_ld(Recx, W(MS), W(DS))                                        \
        shlhxZrx(W(RG))                                                     \
        stack_ld(Recx)

#define shlhxZst(RS, MG, DG)                                                \
        stack_st(Recx)                                                      \
        movhx_rr(Recx, W(RS))                                               \
        shlhxZmx(W(MG), W(DG))                                              \
        stack_ld(Recx)

#define shlhxZmr(MG, DG, RS)                                                \
        shlhxZst(W(RS), W(MG), W(DG))

/* shr (G = G >> S)
 * set-flags: undefined (*_*), yes (*Z*)
 * for maximum compatibility: shift count must be modulo elem-size */

#if RT_BASE_COMPAT_BMI < 2 /* 0 - generic, 1 - 3-op-VEX, 2 - BMI1+BMI2 */

#define shrhx_rx(RG)                     /* reads Recx for shift count */   \
        shrhxZrx(W(RG))

#else /* RT_BASE_COMPAT_BMI >= 2 */

#define shrhx_rx(RG)                     /* reads Recx for shift count */   \
        VEX(RXB(RG), RXB(RG),    0x01, 0, 3, 2) EMITB(0xF7)                 \
        MRM(REG(RG), MOD(RG), REG(RG))

#endif /* RT_BASE_COMPAT_BMI >= 2 */

#define shrhx_mx(MG, DG)                 /* reads Recx for shift count */   \
        shrhxZmx(W(MG), W(DG))

#define shrhx_ri(RG, IS)                                                    \
        shrhxZri(W(RG), W(IS))

#define shrhx_mi(MG, DG, IS)                                                \
        shrhxZmi(W(MG), W(DG), W(IS))

#if RT_BASE_COMPAT_BMI < 2 /* 0 - generic, 1 - 3-op-VEX, 2 - BMI1+BMI2 */

#define shrhx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        shrhxZrr(W(RG), W(RS))

#else /* RT_BASE_COMPAT_BMI >= 2 */

#define shrhx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        VEX(RXB(RG), RXB(RG), REN(RS), 0, 3, 2) EMITB(0xF7)                 \
        MRM(REG(RG), MOD(RG), REG(RG))

#endif /* RT_BASE_COMPAT_BMI >= 2 */

#define shrhx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        shrhxZld(W(RG), W(MS), W(DS))

#define shrhx_st(RS, MG, DG)                                                \
        shrhxZst(W(RS), W(MG), W(DG))

#define shrhx_mr(MG, DG, RS)                                                \
        shrhx_st(W(RS), W(MG), W(DG))


#define shrhxZrx(RG)                     /* reads Recx for shift count */   \
        REX(0,       RXB(RG)) EMITB(0xD3)                                   \
        MRM(0x05,    MOD(RG), REG(RG))                                      \

#define shrhxZmx(MG, DG)                 /* reads Recx for shift count */   \
ADR ESC REX(0,       RXB(MG)) EMITB(0xD3)                                   \
        MRM(0x05,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

#define shrhxZri(RG, IS)                                                    \
        REX(0,       RXB(RG)) EMITB(0xC1)                                   \
        MRM(0x05,    MOD(RG), REG(RG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS)))

#define shrhxZmi(MG, DG, IS)                                                \
ADR ESC REX(0,       RXB(MG)) EMITB(0xC1)                                   \
        MRM(0x05,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMITB(VAL(IS)))

#define shrhxZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movhx_rr(Recx, W(RS))                                               \
        shrhxZrx(W(RG))                                                     \
        stack_ld(Recx)

#define shrhxZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movhx_ld(Recx, W(MS), W(DS))                                        \
        shrhxZrx(W(RG))                                                     \
        stack_ld(Recx)

#define shrhxZst(RS, MG, DG)                                                \
        stack_st(Recx)                                                      \
        movhx_rr(Recx, W(RS))                                               \
        shrhxZmx(W(MG), W(DG))                                              \
        stack_ld(Recx)

#define shrhxZmr(MG, DG, RS)                                                \
        shrhxZst(W(RS), W(MG), W(DG))


#define shrhn_rx(RG)                     /* reads Recx for shift count */   \
    ESC REX(0,       RXB(RG)) EMITB(0xD3)                                   \
        MRM(0x07,    MOD(RG), REG(RG))                                      \

#define shrhn_mx(MG, DG)                 /* reads Recx for shift count */   \
ADR ESC REX(0,       RXB(MG)) EMITB(0xD3)                                   \
        MRM(0x07,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

#define shrhn_ri(RG, IS)                                                    \
    ESC REX(0,       RXB(RG)) EMITB(0xC1)                                   \
        MRM(0x07,    MOD(RG), REG(RG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS)))

#define shrhn_mi(MG, DG, IS)                                                \
ADR ESC REX(0,       RXB(MG)) EMITB(0xC1)                                   \
        MRM(0x07,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMITB(VAL(IS)))

#define shrhn_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movhx_rr(Recx, W(RS))                                               \
        shrhn_rx(W(RG))                                                     \
        stack_ld(Recx)

#define shrhn_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movhx_ld(Recx, W(MS), W(DS))                                        \
        shrhn_rx(W(RG))                                                     \
        stack_ld(Recx)

#define shrhn_st(RS, MG, DG)                                                \
        stack_st(Recx)                                                      \
        movhx_rr(Recx, W(RS))                                               \
        shrhn_mx(W(MG), W(DG))                                              \
        stack_ld(Recx)

#define shrhn_mr(MG, DG, RS)                                                \
        shrhn_st(W(RS), W(MG), W(DG))


#define shrhnZrx(RG)                     /* reads Recx for shift count */   \
    ESC REX(0,       RXB(RG)) EMITB(0xD3)                                   \
        MRM(0x07,    MOD(RG), REG(RG))                                      \

#define shrhnZmx(MG, DG)                 /* reads Recx for shift count */   \
ADR ESC REX(0,       RXB(MG)) EMITB(0xD3)                                   \
        MRM(0x07,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

#define shrhnZri(RG, IS)                                                    \
    ESC REX(0,       RXB(RG)) EMITB(0xC1)                                   \
        MRM(0x07,    MOD(RG), REG(RG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS)))

#define shrhnZmi(MG, DG, IS)                                                \
ADR ESC REX(0,       RXB(MG)) EMITB(0xC1)                                   \
        MRM(0x07,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMITB(VAL(IS)))

#define shrhnZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movhx_rr(Recx, W(RS))                                               \
        shrhnZrx(W(RG))                                                     \
        stack_ld(Recx)

#define shrhnZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movhx_ld(Recx, W(MS), W(DS))                                        \
        shrhnZrx(W(RG))                                                     \
        stack_ld(Recx)

#define shrhnZst(RS, MG, DG)                                                \
        stack_st(Recx)                                                      \
        movhx_rr(Recx, W(RS))                                               \
        shrhnZmx(W(MG), W(DG))                                              \
        stack_ld(Recx)

#define shrhnZmr(MG, DG, RS)                                                \
        shrhnZst(W(RS), W(MG), W(DG))

/* mul (G = G * S)
 * set-flags: undefined */

#define mulhx_ri(RG, IS)                                                    \
    ESC REX(RXB(RG), RXB(RG)) EMITB(0x69)                                   \
        MRM(REG(RG), MOD(RG), REG(RG))   /* truncate IC with TYP below */   \
        AUX(EMPTY,   EMPTY,   EMITH(VAL(IS) & ((TYP(IS) << 6) - 1)))

#define mulhx_rr(RG, RS)                                                    \
        REX(RXB(RG), RXB(RS)) EMITB(0x0F) EMITB(0xAF)                       \
        MRM(REG(RG), MOD(RS), REG(RS))

#define mulhx_ld(RG, MS, DS)                                                \
ADR ESC REX(RXB(RG), RXB(MS)) EMITB(0x0F) EMITB(0xAF)                       \
        MRM(REG(RG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define mulhn_ld(RG, MS, DS)                                                \
        mulhx_ld(W(RG), W(MS), W(DS))


#define mulhx_xr(RS)     /* Reax is in/out, Redx is out(high)-zero-ext */   \
    ESC REX(0,       RXB(RS)) EMITB(0xF7)                                   \
        MRM(0x04,    MOD(RS), REG(RS))                                      \

#define mulhx_xm(MS, DS) /* Reax is in/out, Redx is out(high)-zero-ext */   \
ADR ESC REX(0,       RXB(MS)) EMITB(0xF7)                                   \
        MRM(0x04,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)


#define mulhn_xr(RS)     /* Reax is in/out, Redx is out(high)-sign-ext */   \
    ESC REX(0,       RXB(RS)) EMITB(0xF7)                                   \
        MRM(0x05,    MOD(RS), REG(RS))                                      \

#define mulhn_xm(MS, DS) /* Reax is in/out, Redx is out(high)-sign-ext */   \
ADR ESC REX(0,       RXB(MS)) EMITB(0xF7)                                   \
        MRM(0x05,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)


#define mulhp_xr(RS)     /* Reax is in/out, prepares Redx for divhn_x* */   \
        mulhn_xr(W(RS))       /* product must not exceed operands size */

#define mulhp_xm(MS, DS) /* Reax is in/out, prepares Redx for divhn_x* */   \
        mulhn_xm(W(MS), W(DS))/* product must not exceed operands size */

/* div (G = G / S)
 * set-flags: undefined */

#define divhx_ri(RG, IS)       /* Reax cannot be used as first operand */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movhx_mi(Mebp, inf_SCR01(0), W(IS))                                 \
        movhx_rr(Reax, W(RG))                                               \
        prehx_xx()                                                          \
        divhx_xm(Mebp, inf_SCR01(0))                                        \
        stack_ld(Redx)                                                      \
        movhx_rr(W(RG), Reax)                                               \
        stack_ld(Reax)

#define divhx_rr(RG, RS)                /* RG no Reax, RS no Reax/Redx */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movhx_rr(Reax, W(RG))                                               \
        prehx_xx()                                                          \
        divhx_xr(W(RS))                                                     \
        stack_ld(Redx)                                                      \
        movhx_rr(W(RG), Reax)                                               \
        stack_ld(Reax)

#define divhx_ld(RG, MS, DS)            /* RG no Reax, MS no Oeax/Medx */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movhx_rr(Reax, W(RG))                                               \
        prehx_xx()                                                          \
        divhx_xm(W(MS), W(DS))                                              \
        stack_ld(Redx)                                                      \
        movhx_rr(W(RG), Reax)                                               \
        stack_ld(Reax)


#define divhn_ri(RG, IS)       /* Reax cannot be used as first operand */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movhx_mi(Mebp, inf_SCR01(0), W(IS))                                 \
        movhx_rr(Reax, W(RG))                                               \
        prehn_xx()                                                          \
        divhn_xm(Mebp, inf_SCR01(0))                                        \
        stack_ld(Redx)                                                      \
        movhx_rr(W(RG), Reax)                                               \
        stack_ld(Reax)

#define divhn_rr(RG, RS)                /* RG no Reax, RS no Reax/Redx */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movhx_rr(Reax, W(RG))                                               \
        prehn_xx()                                                          \
        divhn_xr(W(RS))                                                     \
        stack_ld(Redx)                                                      \
        movhx_rr(W(RG), Reax)                                               \
        stack_ld(Reax)

#define divhn_ld(RG, MS, DS)            /* RG no Reax, MS no Oeax/Medx */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movhx_rr(Reax, W(RG))                                               \
        prehn_xx()                                                          \
        divhn_xm(W(MS), W(DS))                                              \
        stack_ld(Redx)                                                      \
        movhx_rr(W(RG), Reax)                                               \
        stack_ld(Reax)


#define prehx_xx()          /* to be placed immediately prior divhx_x* */   \
        movhx_ri(Redx, IC(0))        /* to prepare Redx for int-divide */

#define prehn_xx()          /* to be placed immediately prior divhn_x* */   \
        movhx_rr(Redx, Reax)         /* to prepare Redx for int-divide */   \
        shrhn_ri(Redx, IC(31))


#define divhx_xr(RS)     /* Reax is in/out, Redx is in(zero)/out(junk) */   \
    ESC REX(0,       RXB(RS)) EMITB(0xF7)                                   \
        MRM(0x06,    MOD(RS), REG(RS))                                      \
        AUX(EMPTY,   EMPTY,   EMPTY)

#define divhx_xm(MS, DS) /* Reax is in/out, Redx is in(zero)/out(junk) */   \
ADR ESC REX(0,       RXB(MS)) EMITB(0xF7)                                   \
        MRM(0x06,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)


#define divhn_xr(RS)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
    ESC REX(0,       RXB(RS)) EMITB(0xF7)                                   \
        MRM(0x07,    MOD(RS), REG(RS))                                      \
        AUX(EMPTY,   EMPTY,   EMPTY)

#define divhn_xm(MS, DS) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
ADR ESC REX(0,       RXB(MS)) EMITB(0xF7)                                   \
        MRM(0x07,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)


#define divhp_xr(RS)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divhn_xr(W(RS))              /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

#define divhp_xm(MS, DS) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divhn_xm(W(MS), W(DS))       /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

/* rem (G = G % S)
 * set-flags: undefined */

#define remhx_ri(RG, IS)       /* Redx cannot be used as first operand */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movhx_mi(Mebp, inf_SCR01(0), W(IS))                                 \
        movhx_rr(Reax, W(RG))                                               \
        prehx_xx()                                                          \
        divhx_xm(Mebp, inf_SCR01(0))                                        \
        stack_ld(Reax)                                                      \
        movhx_rr(W(RG), Redx)                                               \
        stack_ld(Redx)

#define remhx_rr(RG, RS)                /* RG no Redx, RS no Reax/Redx */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movhx_rr(Reax, W(RG))                                               \
        prehx_xx()                                                          \
        divhx_xr(W(RS))                                                     \
        stack_ld(Reax)                                                      \
        movhx_rr(W(RG), Redx)                                               \
        stack_ld(Redx)

#define remhx_ld(RG, MS, DS)            /* RG no Redx, MS no Oeax/Medx */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movhx_rr(Reax, W(RG))                                               \
        prehx_xx()                                                          \
        divhx_xm(W(MS), W(DS))                                              \
        stack_ld(Reax)                                                      \
        movhx_rr(W(RG), Redx)                                               \
        stack_ld(Redx)


#define remhn_ri(RG, IS)       /* Redx cannot be used as first operand */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movhx_mi(Mebp, inf_SCR01(0), W(IS))                                 \
        movhx_rr(Reax, W(RG))                                               \
        prehn_xx()                                                          \
        divhn_xm(Mebp, inf_SCR01(0))                                        \
        stack_ld(Reax)                                                      \
        movhx_rr(W(RG), Redx)                                               \
        stack_ld(Redx)

#define remhn_rr(RG, RS)                /* RG no Redx, RS no Reax/Redx */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movhx_rr(Reax, W(RG))                                               \
        prehn_xx()                                                          \
        divhn_xr(W(RS))                                                     \
        stack_ld(Reax)                                                      \
        movhx_rr(W(RG), Redx)                                               \
        stack_ld(Redx)

#define remhn_ld(RG, MS, DS)            /* RG no Redx, MS no Oeax/Medx */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movhx_rr(Reax, W(RG))                                               \
        prehn_xx()                                                          \
        divhn_xm(W(MS), W(DS))                                              \
        stack_ld(Reax)                                                      \
        movhx_rr(W(RG), Redx)                                               \
        stack_ld(Redx)


#define remhx_xx()          /* to be placed immediately prior divhx_x* */   \
                                     /* to prepare for rem calculation */

#define remhx_xr(RS)        /* to be placed immediately after divhx_xr */   \
                                     /* to produce remainder Redx<-rem */

#define remhx_xm(MS, DS)    /* to be placed immediately after divhx_xm */   \
                                     /* to produce remainder Redx<-rem */


#define remhn_xx()          /* to be placed immediately prior divhn_x* */   \
                                     /* to prepare for rem calculation */

#define remhn_xr(RS)        /* to be placed immediately after divhn_xr */   \
                                     /* to produce remainder Redx<-rem */

#define remhn_xm(MS, DS)    /* to be placed immediately after divhn_xm */   \
                                     /* to produce remainder Redx<-rem */

/* arj (G = G op S, if cc G then jump lb)
 * set-flags: undefined
 * refer to individual instruction descriptions
 * to stay within special register limitations */

#define add_n   AM3
#define sub_n   AM4

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
        cmjhx_ri(W(RS), IC(0), cc, lb)

#define cmjhx_mz(MS, DS, cc, lb)                                            \
        cmjhx_mi(W(MS), W(DS), IC(0), cc, lb)

#define cmjhn_mz(MS, DS, cc, lb)                                            \
        cmjhn_mi(W(MS), W(DS), IC(0), cc, lb)

#define cmjhx_ri(RS, IT, cc, lb)                                            \
        cmphx_ri(W(RS), W(IT))                                              \
        CMJ(cc, lb)

#define cmjhx_mi(MS, DS, IT, cc, lb)                                        \
        cmphx_mi(W(MS), W(DS), W(IT))                                       \
        CMJ(cc, lb)

#define cmjhn_mi(MS, DS, IT, cc, lb)                                        \
        cmphn_mi(W(MS), W(DS), W(IT))                                       \
        CMJ(cc, lb)

#define cmjhx_rr(RS, RT, cc, lb)                                            \
        cmphx_rr(W(RS), W(RT))                                              \
        CMJ(cc, lb)

#define cmjhx_rm(RS, MT, DT, cc, lb)                                        \
        cmphx_rm(W(RS), W(MT), W(DT))                                       \
        CMJ(cc, lb)

#define cmjhn_rm(RS, MT, DT, cc, lb)                                        \
        cmphn_rm(W(RS), W(MT), W(DT))                                       \
        CMJ(cc, lb)

#define cmjhx_mr(MS, DS, RT, cc, lb)                                        \
        cmphx_mr(W(MS), W(DS), W(RT))                                       \
        CMJ(cc, lb)

#define cmjhn_mr(MS, DS, RT, cc, lb)                                        \
        cmphn_mr(W(MS), W(DS), W(RT))                                       \
        CMJ(cc, lb)

/* cmp (flags = S ? T)
 * set-flags: yes */

#define cmphx_ri(RS, IT)                                                    \
    ESC REX(0,       RXB(RS)) EMITB(0x81)                                   \
        MRM(0x07,    MOD(RS), REG(RS))   /* truncate IC with TYP below */   \
        AUX(EMPTY,   EMPTY,   EMITH(VAL(IT) & ((TYP(IT) << 6) - 1)))

#define cmphx_mi(MS, DS, IT)                                                \
ADR ESC REX(0,       RXB(MS)) EMITB(0x81)                                   \
        MRM(0x07,    MOD(MS), REG(MS))   /* truncate IC with TYP below */   \
        AUX(SIB(MS), CMD(DS), EMITH(VAL(IT) & ((TYP(IT) << 6) - 1)))

#define cmphn_mi(MS, DS, IT)                                                \
ADR ESC REX(0,       RXB(MS)) EMITB(0x81)                                   \
        MRM(0x07,    MOD(MS), REG(MS))   /* truncate IC with TYP below */   \
        AUX(SIB(MS), CMD(DS), EMITH(VAL(IT) & ((TYP(IT) << 6) - 1)))

#define cmphx_rr(RS, RT)                                                    \
    ESC REX(RXB(RS), RXB(RT)) EMITB(0x3B)                                   \
        MRM(REG(RS), MOD(RT), REG(RT))

#define cmphx_rm(RS, MT, DT)                                                \
ADR ESC REX(RXB(RS), RXB(MT)) EMITB(0x3B)                                   \
        MRM(REG(RS), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

#define cmphn_rm(RS, MT, DT)                                                \
ADR ESC REX(RXB(RS), RXB(MT)) EMITB(0x3B)                                   \
        MRM(REG(RS), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

#define cmphx_mr(MS, DS, RT)                                                \
ADR ESC REX(RXB(RT), RXB(MS)) EMITB(0x39)                                   \
        MRM(REG(RT), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define cmphn_mr(MS, DS, RT)                                                \
ADR ESC REX(RXB(RT), RXB(MS)) EMITB(0x39)                                   \
        MRM(REG(RT), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/************************** 8-bit subset instructions *************************/

/* mov (D = S)
 * set-flags: no */

#define movbx_ri(RD, IS)                                                    \
        REX(0,       RXB(RD)) EMITB(0xC6)                                   \
        MRM(0x00,    MOD(RD), REG(RD))   /* truncate IC with TYP below */   \
        AUX(EMPTY,   EMPTY,   EMITH(VAL(IS) & ((TYP(IS) << 6) - 1)))

#define movbx_mi(MD, DD, IS)                                                \
    ADR REX(0,       RXB(MD)) EMITB(0xC6)                                   \
        MRM(0x00,    MOD(MD), REG(MD))   /* truncate IC with TYP below */   \
        AUX(SIB(MD), CMD(DD), EMITH(VAL(IS) & ((TYP(IS) << 6) - 1)))

#define movbx_rr(RD, RS)                                                    \
        REX(RXB(RD), RXB(RS)) EMITB(0x8B)                                   \
        MRM(REG(RD), MOD(RS), REG(RS))

#define movbx_ld(RD, MS, DS)                                                \
    ADR REW(RXB(RD), RXB(MS)) EMITB(0x0F) EMITB(0xB6)                       \
        MRM(REG(RD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define movbn_ld(RD, MS, DS)                                                \
    ADR REW(RXB(RD), RXB(MS)) EMITB(0x0F) EMITB(0xBE)                       \
        MRM(REG(RD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define movbx_st(RS, MD, DD)                                                \
    ADR REX(RXB(RS), RXB(MD)) EMITB(0x88)                                   \
        MRM(REG(RS), MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)

/* and (G = G & S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define andbx_ri(RG, IS)                                                    \
        andbxZri(W(RG), W(IS))

#define andbx_mi(MG, DG, IS)                                                \
        andbxZmi(W(MG), W(DG), W(IS))

#define andbx_rr(RG, RS)                                                    \
        andbxZrr(W(RG), W(RS))

#define andbx_ld(RG, MS, DS)                                                \
        andbxZld(W(RG), W(MS), W(DS))

#define andbx_st(RS, MG, DG)                                                \
        andbxZst(W(RS), W(MG), W(DG))

#define andbx_mr(MG, DG, RS)                                                \
        andbx_st(W(RS), W(MG), W(DG))


#define andbxZri(RG, IS)                                                    \
        REX(0,       RXB(RG)) EMITB(0x80)                                   \
        MRM(0x04,    MOD(RG), REG(RG))   /* truncate IC with TYP below */   \
        AUX(EMPTY,   EMPTY,   EMITH(VAL(IS) & ((TYP(IS) << 6) - 1)))

#define andbxZmi(MG, DG, IS)                                                \
    ADR REX(0,       RXB(MG)) EMITB(0x80)                                   \
        MRM(0x04,    MOD(MG), REG(MG))   /* truncate IC with TYP below */   \
        AUX(SIB(MG), CMD(DG), EMITH(VAL(IS) & ((TYP(IS) << 6) - 1)))

#define andbxZrr(RG, RS)                                                    \
        REX(RXB(RG), RXB(RS)) EMITB(0x22)                                   \
        MRM(REG(RG), MOD(RS), REG(RS))

#define andbxZld(RG, MS, DS)                                                \
    ADR REX(RXB(RG), RXB(MS)) EMITB(0x22)                                   \
        MRM(REG(RG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define andbxZst(RS, MG, DG)                                                \
    ADR REX(RXB(RS), RXB(MG)) EMITB(0x20)                                   \
        MRM(REG(RS), MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

#define andbxZmr(MG, DG, RS)                                                \
        andbxZst(W(RS), W(MG), W(DG))

/* ann (G = ~G & S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define annbx_ri(RG, IS)                                                    \
        annbxZri(W(RG), W(IS))

#define annbx_mi(MG, DG, IS)                                                \
        annbxZmi(W(MG), W(DG), W(IS))

#define annbx_rr(RG, RS)                                                    \
        annbxZrr(W(RG), W(RS))

#define annbx_ld(RG, MS, DS)                                                \
        annbxZld(W(RG), W(MS), W(DS))

#define annbx_st(RS, MG, DG)                                                \
        annbxZst(W(RS), W(MG), W(DG))

#define annbx_mr(MG, DG, RS)                                                \
        annbx_st(W(RS), W(MG), W(DG))


#define annbxZri(RG, IS)                                                    \
        notbx_rx(W(RG))                                                     \
        andbxZri(W(RG), W(IS))

#define annbxZmi(MG, DG, IS)                                                \
        notbx_mx(W(MG), W(DG))                                              \
        andbxZmi(W(MG), W(DG), W(IS))

#if RT_BASE_COMPAT_BMI < 2 /* 0 - generic, 1 - 3-op-VEX, 2 - BMI1+BMI2 */

#define annbxZrr(RG, RS)                                                    \
        notbx_rx(W(RG))                                                     \
        andbxZrr(W(RG), W(RS))

#else /* RT_BASE_COMPAT_BMI >= 2 */

#define annbxZrr(RG, RS)                                                    \
        VEX(RXB(RG), RXB(RS), REN(RG), 0, 0, 2) EMITB(0xF2)                 \
        MRM(REG(RG), MOD(RS), REG(RS))

#endif /* RT_BASE_COMPAT_BMI >= 2 */

#define annbxZld(RG, MS, DS)                                                \
        notbx_rx(W(RG))                                                     \
        andbxZld(W(RG), W(MS), W(DS))

#define annbxZst(RS, MG, DG)                                                \
        notbx_mx(W(MG), W(DG))                                              \
        andbxZst(W(RS), W(MG), W(DG))

#define annbxZmr(MG, DG, RS)                                                \
        annbxZst(W(RS), W(MG), W(DG))

/* orr (G = G | S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define orrbx_ri(RG, IS)                                                    \
        orrbxZri(W(RG), W(IS))

#define orrbx_mi(MG, DG, IS)                                                \
        orrbxZmi(W(MG), W(DG), W(IS))

#define orrbx_rr(RG, RS)                                                    \
        orrbxZrr(W(RG), W(RS))

#define orrbx_ld(RG, MS, DS)                                                \
        orrbxZld(W(RG), W(MS), W(DS))

#define orrbx_st(RS, MG, DG)                                                \
        orrbxZst(W(RS), W(MG), W(DG))

#define orrbx_mr(MG, DG, RS)                                                \
        orrbx_st(W(RS), W(MG), W(DG))


#define orrbxZri(RG, IS)                                                    \
        REX(0,       RXB(RG)) EMITB(0x80)                                   \
        MRM(0x01,    MOD(RG), REG(RG))   /* truncate IC with TYP below */   \
        AUX(EMPTY,   EMPTY,   EMITH(VAL(IS) & ((TYP(IS) << 6) - 1)))

#define orrbxZmi(MG, DG, IS)                                                \
    ADR REX(0,       RXB(MG)) EMITB(0x80)                                   \
        MRM(0x01,    MOD(MG), REG(MG))   /* truncate IC with TYP below */   \
        AUX(SIB(MG), CMD(DG), EMITH(VAL(IS) & ((TYP(IS) << 6) - 1)))

#define orrbxZrr(RG, RS)                                                    \
        REX(RXB(RG), RXB(RS)) EMITB(0x0A)                                   \
        MRM(REG(RG), MOD(RS), REG(RS))

#define orrbxZld(RG, MS, DS)                                                \
    ADR REX(RXB(RG), RXB(MS)) EMITB(0x0A)                                   \
        MRM(REG(RG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define orrbxZst(RS, MG, DG)                                                \
    ADR REX(RXB(RS), RXB(MG)) EMITB(0x08)                                   \
        MRM(REG(RS), MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

#define orrbxZmr(MG, DG, RS)                                                \
        orrbxZst(W(RS), W(MG), W(DG))

/* orn (G = ~G | S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define ornbx_ri(RG, IS)                                                    \
        ornbxZri(W(RG), W(IS))

#define ornbx_mi(MG, DG, IS)                                                \
        ornbxZmi(W(MG), W(DG), W(IS))

#define ornbx_rr(RG, RS)                                                    \
        ornbxZrr(W(RG), W(RS))

#define ornbx_ld(RG, MS, DS)                                                \
        ornbxZld(W(RG), W(MS), W(DS))

#define ornbx_st(RS, MG, DG)                                                \
        ornbxZst(W(RS), W(MG), W(DG))

#define ornbx_mr(MG, DG, RS)                                                \
        ornbx_st(W(RS), W(MG), W(DG))


#define ornbxZri(RG, IS)                                                    \
        notbx_rx(W(RG))                                                     \
        orrbxZri(W(RG), W(IS))

#define ornbxZmi(MG, DG, IS)                                                \
        notbx_mx(W(MG), W(DG))                                              \
        orrbxZmi(W(MG), W(DG), W(IS))

#define ornbxZrr(RG, RS)                                                    \
        notbx_rx(W(RG))                                                     \
        orrbxZrr(W(RG), W(RS))

#define ornbxZld(RG, MS, DS)                                                \
        notbx_rx(W(RG))                                                     \
        orrbxZld(W(RG), W(MS), W(DS))

#define ornbxZst(RS, MG, DG)                                                \
        notbx_mx(W(MG), W(DG))                                              \
        orrbxZst(W(RS), W(MG), W(DG))

#define ornbxZmr(MG, DG, RS)                                                \
        ornbxZst(W(RS), W(MG), W(DG))

/* xor (G = G ^ S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define xorbx_ri(RG, IS)                                                    \
        xorbxZri(W(RG), W(IS))

#define xorbx_mi(MG, DG, IS)                                                \
        xorbxZmi(W(MG), W(DG), W(IS))

#define xorbx_rr(RG, RS)                                                    \
        xorbxZrr(W(RG), W(RS))

#define xorbx_ld(RG, MS, DS)                                                \
        xorbxZld(W(RG), W(MS), W(DS))

#define xorbx_st(RS, MG, DG)                                                \
        xorbxZst(W(RS), W(MG), W(DG))

#define xorbx_mr(MG, DG, RS)                                                \
        xorbx_st(W(RS), W(MG), W(DG))


#define xorbxZri(RG, IS)                                                    \
        REX(0,       RXB(RG)) EMITB(0x80)                                   \
        MRM(0x06,    MOD(RG), REG(RG))   /* truncate IC with TYP below */   \
        AUX(EMPTY,   EMPTY,   EMITH(VAL(IS) & ((TYP(IS) << 6) - 1)))

#define xorbxZmi(MG, DG, IS)                                                \
    ADR REX(0,       RXB(MG)) EMITB(0x80)                                   \
        MRM(0x06,    MOD(MG), REG(MG))   /* truncate IC with TYP below */   \
        AUX(SIB(MG), CMD(DG), EMITH(VAL(IS) & ((TYP(IS) << 6) - 1)))

#define xorbxZrr(RG, RS)                                                    \
        REX(RXB(RG), RXB(RS)) EMITB(0x32)                                   \
        MRM(REG(RG), MOD(RS), REG(RS))

#define xorbxZld(RG, MS, DS)                                                \
    ADR REX(RXB(RG), RXB(MS)) EMITB(0x32)                                   \
        MRM(REG(RG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define xorbxZst(RS, MG, DG)                                                \
    ADR REX(RXB(RS), RXB(MG)) EMITB(0x30)                                   \
        MRM(REG(RS), MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

#define xorbxZmr(MG, DG, RS)                                                \
        xorbxZst(W(RS), W(MG), W(DG))

/* not (G = ~G)
 * set-flags: no */

#define notbx_rx(RG)                                                        \
        REX(0,       RXB(RG)) EMITB(0xF6)                                   \
        MRM(0x02,    MOD(RG), REG(RG))

#define notbx_mx(MG, DG)                                                    \
ADR ESC REX(0,       RXB(MG)) EMITB(0xF6)                                   \
        MRM(0x02,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

/* neg (G = -G)
 * set-flags: undefined (*_*), yes (*Z*) */

#define negbx_rx(RG)                                                        \
        negbxZrx(W(RG))

#define negbx_mx(MG, DG)                                                    \
        negbxZmx(W(MG), W(DG))


#define negbxZrx(RG)                                                        \
        REX(0,       RXB(RG)) EMITB(0xF6)                                   \
        MRM(0x03,    MOD(RG), REG(RG))

#define negbxZmx(MG, DG)                                                    \
ADR ESC REX(0,       RXB(MG)) EMITB(0xF6)                                   \
        MRM(0x03,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

/* add (G = G + S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define addbx_ri(RG, IS)                                                    \
        addbxZri(W(RG), W(IS))

#define addbx_mi(MG, DG, IS)                                                \
        addbxZmi(W(MG), W(DG), W(IS))

#define addbx_rr(RG, RS)                                                    \
        addbxZrr(W(RG), W(RS))

#define addbx_ld(RG, MS, DS)                                                \
        addbxZld(W(RG), W(MS), W(DS))

#define addbn_ld(RG, MS, DS)                                                \
        addbnZld(W(RG), W(MS), W(DS))

#define addbx_st(RS, MG, DG)                                                \
        addbxZst(W(RS), W(MG), W(DG))

#define addbx_mr(MG, DG, RS)                                                \
        addbx_st(W(RS), W(MG), W(DG))


#define addbxZri(RG, IS)                                                    \
        REX(0,       RXB(RG)) EMITB(0x80)                                   \
        MRM(0x00,    MOD(RG), REG(RG))   /* truncate IC with TYP below */   \
        AUX(EMPTY,   EMPTY,   EMITH(VAL(IS) & ((TYP(IS) << 6) - 1)))

#define addbxZmi(MG, DG, IS)                                                \
    ADR REX(0,       RXB(MG)) EMITB(0x80)                                   \
        MRM(0x00,    MOD(MG), REG(MG))   /* truncate IC with TYP below */   \
        AUX(SIB(MG), CMD(DG), EMITH(VAL(IS) & ((TYP(IS) << 6) - 1)))

#define addbxZrr(RG, RS)                                                    \
        REX(RXB(RG), RXB(RS)) EMITB(0x02)                                   \
        MRM(REG(RG), MOD(RS), REG(RS))

#define addbxZld(RG, MS, DS)                                                \
    ADR REX(RXB(RG), RXB(MS)) EMITB(0x02)                                   \
        MRM(REG(RG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define addbnZld(RG, MS, DS)                                                \
    ADR REX(RXB(RG), RXB(MS)) EMITB(0x02)                                   \
        MRM(REG(RG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define addbxZst(RS, MG, DG)                                                \
    ADR REX(RXB(RS), RXB(MG)) EMITB(0x00)                                   \
        MRM(REG(RS), MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

#define addbxZmr(MG, DG, RS)                                                \
        addbxZst(W(RS), W(MG), W(DG))

/* sub (G = G - S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define subbx_ri(RG, IS)                                                    \
        subbxZri(W(RG), W(IS))

#define subbx_mi(MG, DG, IS)                                                \
        subbxZmi(W(MG), W(DG), W(IS))

#define subbx_rr(RG, RS)                                                    \
        subbxZrr(W(RG), W(RS))

#define subbx_ld(RG, MS, DS)                                                \
        subbxZld(W(RG), W(MS), W(DS))

#define subbn_ld(RG, MS, DS)                                                \
        subbnZld(W(RG), W(MS), W(DS))

#define subbx_st(RS, MG, DG)                                                \
        subbxZst(W(RS), W(MG), W(DG))

#define subbx_mr(MG, DG, RS)                                                \
        subbx_st(W(RS), W(MG), W(DG))


#define subbxZri(RG, IS)                                                    \
        REX(0,       RXB(RG)) EMITB(0x80)                                   \
        MRM(0x05,    MOD(RG), REG(RG))   /* truncate IC with TYP below */   \
        AUX(EMPTY,   EMPTY,   EMITH(VAL(IS) & ((TYP(IS) << 6) - 1)))

#define subbxZmi(MG, DG, IS)                                                \
    ADR REX(0,       RXB(MG)) EMITB(0x80)                                   \
        MRM(0x05,    MOD(MG), REG(MG))   /* truncate IC with TYP below */   \
        AUX(SIB(MG), CMD(DG), EMITH(VAL(IS) & ((TYP(IS) << 6) - 1)))

#define subbxZrr(RG, RS)                                                    \
        REX(RXB(RG), RXB(RS)) EMITB(0x2A)                                   \
        MRM(REG(RG), MOD(RS), REG(RS))

#define subbxZld(RG, MS, DS)                                                \
    ADR REX(RXB(RG), RXB(MS)) EMITB(0x2A)                                   \
        MRM(REG(RG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define subbnZld(RG, MS, DS)                                                \
    ADR REX(RXB(RG), RXB(MS)) EMITB(0x2A)                                   \
        MRM(REG(RG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define subbxZst(RS, MG, DG)                                                \
    ADR REX(RXB(RS), RXB(MG)) EMITB(0x28)                                   \
        MRM(REG(RS), MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

#define subbxZmr(MG, DG, RS)                                                \
        subbxZst(W(RS), W(MG), W(DG))

/* shl (G = G << S)
 * set-flags: undefined (*_*), yes (*Z*)
 * for maximum compatibility: shift count must be modulo elem-size */

#if RT_BASE_COMPAT_BMI < 2 /* 0 - generic, 1 - 3-op-VEX, 2 - BMI1+BMI2 */

#define shlbx_rx(RG)                     /* reads Recx for shift count */   \
        shlbxZrx(W(RG))

#else /* RT_BASE_COMPAT_BMI >= 2 */

#define shlbx_rx(RG)                     /* reads Recx for shift count */   \
        VEX(RXB(RG), RXB(RG),    0x01, 0, 1, 2) EMITB(0xF7)                 \
        MRM(REG(RG), MOD(RG), REG(RG))

#endif /* RT_BASE_COMPAT_BMI >= 2 */

#define shlbx_mx(MG, DG)                 /* reads Recx for shift count */   \
        shlbxZmx(W(MG), W(DG))

#define shlbx_ri(RG, IS)                                                    \
        shlbxZri(W(RG), W(IS))

#define shlbx_mi(MG, DG, IS)                                                \
        shlbxZmi(W(MG), W(DG), W(IS))

#if RT_BASE_COMPAT_BMI < 2 /* 0 - generic, 1 - 3-op-VEX, 2 - BMI1+BMI2 */

#define shlbx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        shlbxZrr(W(RG), W(RS))

#else /* RT_BASE_COMPAT_BMI >= 2 */

#define shlbx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        VEX(RXB(RG), RXB(RG), REN(RS), 0, 1, 2) EMITB(0xF7)                 \
        MRM(REG(RG), MOD(RG), REG(RG))

#endif /* RT_BASE_COMPAT_BMI >= 2 */

#define shlbx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        shlbxZld(W(RG), W(MS), W(DS))

#define shlbx_st(RS, MG, DG)                                                \
        shlbxZst(W(RS), W(MG), W(DG))

#define shlbx_mr(MG, DG, RS)                                                \
        shlbx_st(W(RS), W(MG), W(DG))


#define shlbxZrx(RG)                     /* reads Recx for shift count */   \
        REX(0,       RXB(RG)) EMITB(0xD2)                                   \
        MRM(0x04,    MOD(RG), REG(RG))                                      \

#define shlbxZmx(MG, DG)                 /* reads Recx for shift count */   \
    ADR REX(0,       RXB(MG)) EMITB(0xD2)                                   \
        MRM(0x04,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

#define shlbxZri(RG, IS)                                                    \
        REX(0,       RXB(RG)) EMITB(0xC0)                                   \
        MRM(0x04,    MOD(RG), REG(RG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS)))

#define shlbxZmi(MG, DG, IS)                                                \
    ADR REX(0,       RXB(MG)) EMITB(0xC0)                                   \
        MRM(0x04,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMITB(VAL(IS)))

#define shlbxZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movbx_rr(Recx, W(RS))                                               \
        shlbxZrx(W(RG))                                                     \
        stack_ld(Recx)

#define shlbxZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movbx_ld(Recx, W(MS), W(DS))                                        \
        shlbxZrx(W(RG))                                                     \
        stack_ld(Recx)

#define shlbxZst(RS, MG, DG)                                                \
        stack_st(Recx)                                                      \
        movbx_rr(Recx, W(RS))                                               \
        shlbxZmx(W(MG), W(DG))                                              \
        stack_ld(Recx)

#define shlbxZmr(MG, DG, RS)                                                \
        shlbxZst(W(RS), W(MG), W(DG))

/* shr (G = G >> S)
 * set-flags: undefined (*_*), yes (*Z*)

 * for maximum compatibility: shift count must be modulo elem-size */

#if RT_BASE_COMPAT_BMI < 2 /* 0 - generic, 1 - 3-op-VEX, 2 - BMI1+BMI2 */

#define shrbx_rx(RG)                     /* reads Recx for shift count */   \
        shrbxZrx(W(RG))

#else /* RT_BASE_COMPAT_BMI >= 2 */

#define shrbx_rx(RG)                     /* reads Recx for shift count */   \
        VEX(RXB(RG), RXB(RG),    0x01, 0, 3, 2) EMITB(0xF7)                 \
        MRM(REG(RG), MOD(RG), REG(RG))

#endif /* RT_BASE_COMPAT_BMI >= 2 */

#define shrbx_mx(MG, DG)                 /* reads Recx for shift count */   \
        shrbxZmx(W(MG), W(DG))

#define shrbx_ri(RG, IS)                                                    \
        shrbxZri(W(RG), W(IS))

#define shrbx_mi(MG, DG, IS)                                                \
        shrbxZmi(W(MG), W(DG), W(IS))

#if RT_BASE_COMPAT_BMI < 2 /* 0 - generic, 1 - 3-op-VEX, 2 - BMI1+BMI2 */

#define shrbx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        shrbxZrr(W(RG), W(RS))

#else /* RT_BASE_COMPAT_BMI >= 2 */

#define shrbx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        VEX(RXB(RG), RXB(RG), REN(RS), 0, 3, 2) EMITB(0xF7)                 \
        MRM(REG(RG), MOD(RG), REG(RG))

#endif /* RT_BASE_COMPAT_BMI >= 2 */

#define shrbx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        shrbxZld(W(RG), W(MS), W(DS))

#define shrbx_st(RS, MG, DG)                                                \
        shrbxZst(W(RS), W(MG), W(DG))

#define shrbx_mr(MG, DG, RS)                                                \
        shrbx_st(W(RS), W(MG), W(DG))


#define shrbxZrx(RG)                     /* reads Recx for shift count */   \
        REX(0,       RXB(RG)) EMITB(0xD2)                                   \
        MRM(0x05,    MOD(RG), REG(RG))                                      \

#define shrbxZmx(MG, DG)                 /* reads Recx for shift count */   \
    ADR REX(0,       RXB(MG)) EMITB(0xD2)                                   \
        MRM(0x05,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

#define shrbxZri(RG, IS)                                                    \
        REX(0,       RXB(RG)) EMITB(0xC0)                                   \
        MRM(0x05,    MOD(RG), REG(RG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS)))

#define shrbxZmi(MG, DG, IS)                                                \
    ADR REX(0,       RXB(MG)) EMITB(0xC0)                                   \
        MRM(0x05,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMITB(VAL(IS)))

#define shrbxZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movbx_rr(Recx, W(RS))                                               \
        shrbxZrx(W(RG))                                                     \
        stack_ld(Recx)

#define shrbxZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movbx_ld(Recx, W(MS), W(DS))                                        \
        shrbxZrx(W(RG))                                                     \
        stack_ld(Recx)

#define shrbxZst(RS, MG, DG)                                                \
        stack_st(Recx)                                                      \
        movbx_rr(Recx, W(RS))                                               \
        shrbxZmx(W(MG), W(DG))                                              \
        stack_ld(Recx)

#define shrbxZmr(MG, DG, RS)                                                \
        shrbxZst(W(RS), W(MG), W(DG))


#define shrbn_rx(RG)                     /* reads Recx for shift count */   \
        REX(0,       RXB(RG)) EMITB(0xD2)                                   \
        MRM(0x07,    MOD(RG), REG(RG))                                      \

#define shrbn_mx(MG, DG)                 /* reads Recx for shift count */   \
    ADR REX(0,       RXB(MG)) EMITB(0xD2)                                   \
        MRM(0x07,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

#define shrbn_ri(RG, IS)                                                    \
        REX(0,       RXB(RG)) EMITB(0xC0)                                   \
        MRM(0x07,    MOD(RG), REG(RG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS)))

#define shrbn_mi(MG, DG, IS)                                                \
    ADR REX(0,       RXB(MG)) EMITB(0xC0)                                   \
        MRM(0x07,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMITB(VAL(IS)))

#define shrbn_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movbx_rr(Recx, W(RS))                                               \
        shrbn_rx(W(RG))                                                     \
        stack_ld(Recx)

#define shrbn_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movbx_ld(Recx, W(MS), W(DS))                                        \
        shrbn_rx(W(RG))                                                     \
        stack_ld(Recx)

#define shrbn_st(RS, MG, DG)                                                \
        stack_st(Recx)                                                      \
        movbx_rr(Recx, W(RS))                                               \
        shrbn_mx(W(MG), W(DG))                                              \
        stack_ld(Recx)

#define shrbn_mr(MG, DG, RS)                                                \
        shrbn_st(W(RS), W(MG), W(DG))


#define shrbnZrx(RG)                     /* reads Recx for shift count */   \
        REX(0,       RXB(RG)) EMITB(0xD2)                                   \
        MRM(0x07,    MOD(RG), REG(RG))                                      \

#define shrbnZmx(MG, DG)                 /* reads Recx for shift count */   \
    ADR REX(0,       RXB(MG)) EMITB(0xD2)                                   \
        MRM(0x07,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

#define shrbnZri(RG, IS)                                                    \
        REX(0,       RXB(RG)) EMITB(0xC0)                                   \
        MRM(0x07,    MOD(RG), REG(RG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS)))

#define shrbnZmi(MG, DG, IS)                                                \
    ADR REX(0,       RXB(MG)) EMITB(0xC0)                                   \
        MRM(0x07,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMITB(VAL(IS)))

#define shrbnZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movbx_rr(Recx, W(RS))                                               \
        shrbnZrx(W(RG))                                                     \
        stack_ld(Recx)

#define shrbnZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movbx_ld(Recx, W(MS), W(DS))                                        \
        shrbnZrx(W(RG))                                                     \
        stack_ld(Recx)

#define shrbnZst(RS, MG, DG)                                                \
        stack_st(Recx)                                                      \
        movbx_rr(Recx, W(RS))                                               \
        shrbnZmx(W(MG), W(DG))                                              \
        stack_ld(Recx)

#define shrbnZmr(MG, DG, RS)                                                \
        shrbnZst(W(RS), W(MG), W(DG))

/* mul (G = G * S)
 * set-flags: undefined */

#define mulbx_ri(RG, IS)                                                    \
    ESC REX(RXB(RG), RXB(RG)) EMITB(0x69)                                   \
        MRM(REG(RG), MOD(RG), REG(RG))   /* truncate IC with TYP below */   \
        AUX(EMPTY,   EMPTY,   EMITH(VAL(IS) & ((TYP(IS) << 6) - 1)))

#define mulbx_rr(RG, RS)                                                    \
        REX(RXB(RG), RXB(RS)) EMITB(0x0F) EMITB(0xAF)                       \
        MRM(REG(RG), MOD(RS), REG(RS))

#define mulbx_ld(RG, MS, DS)                                                \
        stack_st(Reax)                                                      \
        movbx_rr(Reax, W(RG))                                               \
        mulbx_xm(W(MS), W(DS))                                              \
        movbx_rr(W(RG), Reax)                                               \
        stack_ld(Reax)

#define mulbn_ld(RG, MS, DS)                                                \
        mulbx_ld(W(RG), W(MS), W(DS))


#define mulbx_xr(RS)     /* Reax is in/out, Redx is out(high)-zero-ext */   \
        REX(0,       RXB(RS)) EMITB(0xF6)                                   \
        MRM(0x04,    MOD(RS), REG(RS))                                      \

#define mulbx_xm(MS, DS) /* Reax is in/out, Redx is out(high)-zero-ext */   \
    ADR REX(0,       RXB(MS)) EMITB(0xF6)                                   \
        MRM(0x04,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)


#define mulbn_xr(RS)     /* Reax is in/out, Redx is out(high)-sign-ext */   \
        REX(0,       RXB(RS)) EMITB(0xF6)                                   \
        MRM(0x05,    MOD(RS), REG(RS))                                      \

#define mulbn_xm(MS, DS) /* Reax is in/out, Redx is out(high)-sign-ext */   \
    ADR REX(0,       RXB(MS)) EMITB(0xF6)                                   \
        MRM(0x05,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)


#define mulbp_xr(RS)     /* Reax is in/out, prepares Redx for divbn_x* */   \
        mulbn_xr(W(RS))       /* product must not exceed operands size */

#define mulbp_xm(MS, DS) /* Reax is in/out, prepares Redx for divbn_x* */   \
        mulbn_xm(W(MS), W(DS))/* product must not exceed operands size */

/* div (G = G / S)
 * set-flags: undefined */

#define divbx_ri(RG, IS)       /* Reax cannot be used as first operand */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movbx_mi(Mebp, inf_SCR01(0), W(IS))                                 \
        movbx_rr(Reax, W(RG))                                               \
        prebx_xx()                                                          \
        divbx_xm(Mebp, inf_SCR01(0))                                        \
        stack_ld(Redx)                                                      \
        movbx_rr(W(RG), Reax)                                               \
        stack_ld(Reax)

#define divbx_rr(RG, RS)                /* RG no Reax, RS no Reax/Redx */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movbx_rr(Reax, W(RG))                                               \
        prebx_xx()                                                          \
        divbx_xr(W(RS))                                                     \
        stack_ld(Redx)                                                      \
        movbx_rr(W(RG), Reax)                                               \
        stack_ld(Reax)

#define divbx_ld(RG, MS, DS)            /* RG no Reax, MS no Oeax/Medx */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movbx_rr(Reax, W(RG))                                               \
        prebx_xx()                                                          \
        divbx_xm(W(MS), W(DS))                                              \
        stack_ld(Redx)                                                      \
        movbx_rr(W(RG), Reax)                                               \
        stack_ld(Reax)


#define divbn_ri(RG, IS)       /* Reax cannot be used as first operand */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movbx_mi(Mebp, inf_SCR01(0), W(IS))                                 \
        movbx_rr(Reax, W(RG))                                               \
        prebn_xx()                                                          \
        divbn_xm(Mebp, inf_SCR01(0))                                        \
        stack_ld(Redx)                                                      \
        movbx_rr(W(RG), Reax)                                               \
        stack_ld(Reax)

#define divbn_rr(RG, RS)                /* RG no Reax, RS no Reax/Redx */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movbx_rr(Reax, W(RG))                                               \
        prebn_xx()                                                          \
        divbn_xr(W(RS))                                                     \
        stack_ld(Redx)                                                      \
        movbx_rr(W(RG), Reax)                                               \
        stack_ld(Reax)

#define divbn_ld(RG, MS, DS)            /* RG no Reax, MS no Oeax/Medx */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movbx_rr(Reax, W(RG))                                               \
        prebn_xx()                                                          \
        divbn_xm(W(MS), W(DS))                                              \
        stack_ld(Redx)                                                      \
        movbx_rr(W(RG), Reax)                                               \
        stack_ld(Reax)


#define prebx_xx()          /* to be placed immediately prior divbx_x* */

#define prebn_xx()          /* to be placed immediately prior divbn_x* */


#define divbx_xr(RS)     /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        REX(0,       RXB(RS)) EMITB(0xF6)                                   \
        MRM(0x06,    MOD(RS), REG(RS))                                      \
        AUX(EMPTY,   EMPTY,   EMPTY)                                        \
        EMITB(0x8A)                                                         \
        MRM(0x02,    0x03,    0x04)

#define divbx_xm(MS, DS) /* Reax is in/out, Redx is in(zero)/out(junk) */   \
    ADR REX(0,       RXB(MS)) EMITB(0xF6)                                   \
        MRM(0x06,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)                                        \
        EMITB(0x8A)                                                         \
        MRM(0x02,    0x03,    0x04)


#define divbn_xr(RS)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        REX(0,       RXB(RS)) EMITB(0xF6)                                   \
        MRM(0x07,    MOD(RS), REG(RS))                                      \
        AUX(EMPTY,   EMPTY,   EMPTY)                                        \
        EMITB(0x8A)                                                         \
        MRM(0x02,    0x03,    0x04)

#define divbn_xm(MS, DS) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
    ADR REX(0,       RXB(MS)) EMITB(0xF6)                                   \
        MRM(0x07,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)                                        \
        EMITB(0x8A)                                                         \
        MRM(0x02,    0x03,    0x04)


#define divbp_xr(RS)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divbn_xr(W(RS))              /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

#define divbp_xm(MS, DS) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divbn_xm(W(MS), W(DS))       /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

/* rem (G = G % S)
 * set-flags: undefined */

#define rembx_ri(RG, IS)       /* Redx cannot be used as first operand */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movbx_mi(Mebp, inf_SCR01(0), W(IS))                                 \
        movbx_rr(Reax, W(RG))                                               \
        prebx_xx()                                                          \
        divbx_xm(Mebp, inf_SCR01(0))                                        \
        stack_ld(Reax)                                                      \
        movbx_rr(W(RG), Redx)                                               \
        stack_ld(Redx)

#define rembx_rr(RG, RS)                /* RG no Redx, RS no Reax/Redx */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movbx_rr(Reax, W(RG))                                               \
        prebx_xx()                                                          \
        divbx_xr(W(RS))                                                     \
        stack_ld(Reax)                                                      \
        movbx_rr(W(RG), Redx)                                               \
        stack_ld(Redx)

#define rembx_ld(RG, MS, DS)            /* RG no Redx, MS no Oeax/Medx */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movbx_rr(Reax, W(RG))                                               \
        prebx_xx()                                                          \
        divbx_xm(W(MS), W(DS))                                              \
        stack_ld(Reax)                                                      \
        movbx_rr(W(RG), Redx)                                               \
        stack_ld(Redx)


#define rembn_ri(RG, IS)       /* Redx cannot be used as first operand */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movbx_mi(Mebp, inf_SCR01(0), W(IS))                                 \
        movbx_rr(Reax, W(RG))                                               \
        prebn_xx()                                                          \
        divbn_xm(Mebp, inf_SCR01(0))                                        \
        stack_ld(Reax)                                                      \
        movbx_rr(W(RG), Redx)                                               \
        stack_ld(Redx)

#define rembn_rr(RG, RS)                /* RG no Redx, RS no Reax/Redx */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movbx_rr(Reax, W(RG))                                               \
        prebn_xx()                                                          \
        divbn_xr(W(RS))                                                     \
        stack_ld(Reax)                                                      \
        movbx_rr(W(RG), Redx)                                               \
        stack_ld(Redx)

#define rembn_ld(RG, MS, DS)            /* RG no Redx, MS no Oeax/Medx */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movbx_rr(Reax, W(RG))                                               \
        prebn_xx()                                                          \
        divbn_xm(W(MS), W(DS))                                              \
        stack_ld(Reax)                                                      \
        movbx_rr(W(RG), Redx)                                               \
        stack_ld(Redx)


#define rembx_xx()          /* to be placed immediately prior divbx_x* */   \
                                     /* to prepare for rem calculation */

#define rembx_xr(RS)        /* to be placed immediately after divbx_xr */   \
                                     /* to produce remainder Redx<-rem */

#define rembx_xm(MS, DS)    /* to be placed immediately after divbx_xm */   \
                                     /* to produce remainder Redx<-rem */


#define rembn_xx()          /* to be placed immediately prior divbn_x* */   \
                                     /* to prepare for rem calculation */

#define rembn_xr(RS)        /* to be placed immediately after divbn_xr */   \
                                     /* to produce remainder Redx<-rem */

#define rembn_xm(MS, DS)    /* to be placed immediately after divbn_xm */   \
                                     /* to produce remainder Redx<-rem */

/* arj (G = G op S, if cc G then jump lb)
 * set-flags: undefined
 * refer to individual instruction descriptions
 * to stay within special register limitations */

#define add_n   AM3
#define sub_n   AM4

     /* Definitions for arj's "op" and "cc" parameters
      * are provided in 32-bit rtarch_***.h files. */

#define arjbx_rx(RG, op, cc, lb)                                            \
        AR1(W(RG), op, h, Zrx)                                              \
        CMJ(cc, lb)

#define arjbx_mx(MG, DG, op, cc, lb)                                        \
        AR2(W(MG), W(DG), op, h, Zmx)                                       \
        CMJ(cc, lb)

#define arjbx_ri(RG, IS, op, cc, lb)                                        \
        AR2(W(RG), W(IS), op, h, Zri)                                       \
        CMJ(cc, lb)

#define arjbx_mi(MG, DG, IS, op, cc, lb)                                    \
        AR3(W(MG), W(DG), W(IS), op, h, Zmi)                                \
        CMJ(cc, lb)

#define arjbx_rr(RG, RS, op, cc, lb)                                        \
        AR2(W(RG), W(RS), op, h, Zrr)                                       \
        CMJ(cc, lb)

#define arjbx_ld(RG, MS, DS, op, cc, lb)                                    \
        AR3(W(RG), W(MS), W(DS), op, h, Zld)                                \
        CMJ(cc, lb)

#define arjbx_st(RS, MG, DG, op, cc, lb)                                    \
        AR3(W(RS), W(MG), W(DG), op, h, Zst)                                \
        CMJ(cc, lb)

#define arjbx_mr(MG, DG, RS, op, cc, lb)                                    \
        arjbx_st(W(RS), W(MG), W(DG), op, cc, lb)

/* cmj (flags = S ? T, if cc flags then jump lb)
 * set-flags: undefined */

     /* Definitions for cmj's "cc" parameter
      * are provided in 32-bit rtarch_***.h files. */

#define cmjbx_rz(RS, cc, lb)                                                \
        cmjbx_ri(W(RS), IC(0), cc, lb)

#define cmjbx_mz(MS, DS, cc, lb)                                            \
        cmjbx_mi(W(MS), W(DS), IC(0), cc, lb)

#define cmjbn_mz(MS, DS, cc, lb)                                            \
        cmjbn_mi(W(MS), W(DS), IC(0), cc, lb)

#define cmjbx_ri(RS, IT, cc, lb)                                            \
        cmpbx_ri(W(RS), W(IT))                                              \
        CMJ(cc, lb)

#define cmjbx_mi(MS, DS, IT, cc, lb)                                        \
        cmpbx_mi(W(MS), W(DS), W(IT))                                       \
        CMJ(cc, lb)

#define cmjbn_mi(MS, DS, IT, cc, lb)                                        \
        cmpbn_mi(W(MS), W(DS), W(IT))                                       \
        CMJ(cc, lb)

#define cmjbx_rr(RS, RT, cc, lb)                                            \
        cmpbx_rr(W(RS), W(RT))                                              \
        CMJ(cc, lb)

#define cmjbx_rm(RS, MT, DT, cc, lb)                                        \
        cmpbx_rm(W(RS), W(MT), W(DT))                                       \
        CMJ(cc, lb)

#define cmjbn_rm(RS, MT, DT, cc, lb)                                        \
        cmpbn_rm(W(RS), W(MT), W(DT))                                       \
        CMJ(cc, lb)

#define cmjbx_mr(MS, DS, RT, cc, lb)                                        \
        cmpbx_mr(W(MS), W(DS), W(RT))                                       \
        CMJ(cc, lb)

#define cmjbn_mr(MS, DS, RT, cc, lb)                                        \
        cmpbn_mr(W(MS), W(DS), W(RT))                                       \
        CMJ(cc, lb)

/* cmp (flags = S ? T)
 * set-flags: yes */

#define cmpbx_ri(RS, IT)                                                    \
        REX(0,       RXB(RS)) EMITB(0x80)                                   \
        MRM(0x07,    MOD(RS), REG(RS))   /* truncate IC with TYP below */   \
        AUX(EMPTY,   EMPTY,   EMITH(VAL(IT) & ((TYP(IT) << 6) - 1)))

#define cmpbx_mi(MS, DS, IT)                                                \
    ADR REX(0,       RXB(MS)) EMITB(0x80)                                   \
        MRM(0x07,    MOD(MS), REG(MS))   /* truncate IC with TYP below */   \
        AUX(SIB(MS), CMD(DS), EMITH(VAL(IT) & ((TYP(IT) << 6) - 1)))

#define cmpbn_mi(MS, DS, IT)                                                \
    ADR REX(0,       RXB(MS)) EMITB(0x80)                                   \
        MRM(0x07,    MOD(MS), REG(MS))   /* truncate IC with TYP below */   \
        AUX(SIB(MS), CMD(DS), EMITH(VAL(IT) & ((TYP(IT) << 6) - 1)))

#define cmpbx_rr(RS, RT)                                                    \
        REX(RXB(RS), RXB(RT)) EMITB(0x3A)                                   \
        MRM(REG(RS), MOD(RT), REG(RT))

#define cmpbx_rm(RS, MT, DT)                                                \
    ADR REX(RXB(RS), RXB(MT)) EMITB(0x3A)                                   \
        MRM(REG(RS), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

#define cmpbn_rm(RS, MT, DT)                                                \
    ADR REX(RXB(RS), RXB(MT)) EMITB(0x3A)                                   \
        MRM(REG(RS), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

#define cmpbx_mr(MS, DS, RT)                                                \
    ADR REX(RXB(RT), RXB(MS)) EMITB(0x38)                                   \
        MRM(REG(RT), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define cmpbn_mr(MS, DS, RT)                                                \
    ADR REX(RXB(RT), RXB(MS)) EMITB(0x38)                                   \
        MRM(REG(RT), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

#endif /* RT_RTARCH_XHB_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
