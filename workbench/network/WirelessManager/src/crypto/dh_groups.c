/*
 * Diffie-Hellman groups
 * Copyright (c) 2007, Jouni Malinen <j@w1.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 */

#include "includes.h"

#include "common.h"
#include "crypto.h"
#include "dh_groups.h"


#ifdef ALL_DH_GROUPS

/* RFC 4306, B.1. Group 1 - 768 Bit MODP
 * Generator: 2
 * Prime: 2^768 - 2 ^704 - 1 + 2^64 * { [2^638 pi] + 149686 }
 */
static const u8 dh_group1_generator[1] = { 0x02 };
static const u8 dh_group1_prime[96] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xC9, 0x0F, 0xDA, 0xA2, 0x21, 0x68, 0xC2, 0x34,
	0xC4, 0xC6, 0x62, 0x8B, 0x80, 0xDC, 0x1C, 0xD1,
	0x29, 0x02, 0x4E, 0x08, 0x8A, 0x67, 0xCC, 0x74,
	0x02, 0x0B, 0xBE, 0xA6, 0x3B, 0x13, 0x9B, 0x22,
	0x51, 0x4A, 0x08, 0x79, 0x8E, 0x34, 0x04, 0xDD,
	0xEF, 0x95, 0x19, 0xB3, 0xCD, 0x3A, 0x43, 0x1B,
	0x30, 0x2B, 0x0A, 0x6D, 0xF2, 0x5F, 0x14, 0x37,
	0x4F, 0xE1, 0x35, 0x6D, 0x6D, 0x51, 0xC2, 0x45,
	0xE4, 0x85, 0xB5, 0x76, 0x62, 0x5E, 0x7E, 0xC6,
	0xF4, 0x4C, 0x42, 0xE9, 0xA6, 0x3A, 0x36, 0x20,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

/* RFC 4306, B.2. Group 2 - 1024 Bit MODP
 * Generator: 2
 * Prime: 2^1024 - 2^960 - 1 + 2^64 * { [2^894 pi] + 129093 }
 */
static const u8 dh_group2_generator[1] = { 0x02 };
static const u8 dh_group2_prime[128] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xC9, 0x0F, 0xDA, 0xA2, 0x21, 0x68, 0xC2, 0x34,
	0xC4, 0xC6, 0x62, 0x8B, 0x80, 0xDC, 0x1C, 0xD1,
	0x29, 0x02, 0x4E, 0x08, 0x8A, 0x67, 0xCC, 0x74,
	0x02, 0x0B, 0xBE, 0xA6, 0x3B, 0x13, 0x9B, 0x22,
	0x51, 0x4A, 0x08, 0x79, 0x8E, 0x34, 0x04, 0xDD,
	0xEF, 0x95, 0x19, 0xB3, 0xCD, 0x3A, 0x43, 0x1B,
	0x30, 0x2B, 0x0A, 0x6D, 0xF2, 0x5F, 0x14, 0x37,
	0x4F, 0xE1, 0x35, 0x6D, 0x6D, 0x51, 0xC2, 0x45,
	0xE4, 0x85, 0xB5, 0x76, 0x62, 0x5E, 0x7E, 0xC6,
	0xF4, 0x4C, 0x42, 0xE9, 0xA6, 0x37, 0xED, 0x6B,
	0x0B, 0xFF, 0x5C, 0xB6, 0xF4, 0x06, 0xB7, 0xED,
	0xEE, 0x38, 0x6B, 0xFB, 0x5A, 0x89, 0x9F, 0xA5,
	0xAE, 0x9F, 0x24, 0x11, 0x7C, 0x4B, 0x1F, 0xE6,
	0x49, 0x28, 0x66, 0x51, 0xEC, 0xE6, 0x53, 0x81,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

#endif /* ALL_DH_GROUPS */

/* RFC 3526, 2. Group 5 - 1536 Bit MODP
 * Generator: 2
 * Prime: 2^1536 - 2^1472 - 1 + 2^64 * { [2^1406 pi] + 741804 }
 */
static const u8 dh_group5_generator[1] = { 0x02 };
static const u8 dh_group5_prime[192] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xC9, 0x0F, 0xDA, 0xA2, 0x21, 0x68, 0xC2, 0x34,
	0xC4, 0xC6, 0x62, 0x8B, 0x80, 0xDC, 0x1C, 0xD1,
	0x29, 0x02, 0x4E, 0x08, 0x8A, 0x67, 0xCC, 0x74,
	0x02, 0x0B, 0xBE, 0xA6, 0x3B, 0x13, 0x9B, 0x22,
	0x51, 0x4A, 0x08, 0x79, 0x8E, 0x34, 0x04, 0xDD,
	0xEF, 0x95, 0x19, 0xB3, 0xCD, 0x3A, 0x43, 0x1B,
	0x30, 0x2B, 0x0A, 0x6D, 0xF2, 0x5F, 0x14, 0x37,
	0x4F, 0xE1, 0x35, 0x6D, 0x6D, 0x51, 0xC2, 0x45,
	0xE4, 0x85, 0xB5, 0x76, 0x62, 0x5E, 0x7E, 0xC6,
	0xF4, 0x4C, 0x42, 0xE9, 0xA6, 0x37, 0xED, 0x6B,
	0x0B, 0xFF, 0x5C, 0xB6, 0xF4, 0x06, 0xB7, 0xED,
	0xEE, 0x38, 0x6B, 0xFB, 0x5A, 0x89, 0x9F, 0xA5,
	0xAE, 0x9F, 0x24, 0x11, 0x7C, 0x4B, 0x1F, 0xE6,
	0x49, 0x28, 0x66, 0x51, 0xEC, 0xE4, 0x5B, 0x3D,
	0xC2, 0x00, 0x7C, 0xB8, 0xA1, 0x63, 0xBF, 0x05,
	0x98, 0xDA, 0x48, 0x36, 0x1C, 0x55, 0xD3, 0x9A,
	0x69, 0x16, 0x3F, 0xA8, 0xFD, 0x24, 0xCF, 0x5F,
	0x83, 0x65, 0x5D, 0x23, 0xDC, 0xA3, 0xAD, 0x96,
	0x1C, 0x62, 0xF3, 0x56, 0x20, 0x85, 0x52, 0xBB,
	0x9E, 0xD5, 0x29, 0x07, 0x70, 0x96, 0x96, 0x6D,
	0x67, 0x0C, 0x35, 0x4E, 0x4A, 0xBC, 0x98, 0x04,
	0xF1, 0x74, 0x6C, 0x08, 0xCA, 0x23, 0x73, 0x27,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

#ifdef ALL_DH_GROUPS

/* RFC 3526, 3. Group 14 - 2048 Bit MODP
 * Generator: 2
 * Prime: 2^2048 - 2^1984 - 1 + 2^64 * { [2^1918 pi] + 124476 }
 */
static const u8 dh_group14_generator[1] = { 0x02 };
static const u8 dh_group14_prime[256] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xC9, 0x0F, 0xDA, 0xA2, 0x21, 0x68, 0xC2, 0x34,
	0xC4, 0xC6, 0x62, 0x8B, 0x80, 0xDC, 0x1C, 0xD1,
	0x29, 0x02, 0x4E, 0x08, 0x8A, 0x67, 0xCC, 0x74,
	0x02, 0x0B, 0xBE, 0xA6, 0x3B, 0x13, 0x9B, 0x22,
	0x51, 0x4A, 0x08, 0x79, 0x8E, 0x34, 0x04, 0xDD,
	0xEF, 0x95, 0x19, 0xB3, 0xCD, 0x3A, 0x43, 0x1B,
	0x30, 0x2B, 0x0A, 0x6D, 0xF2, 0x5F, 0x14, 0x37,
	0x4F, 0xE1, 0x35, 0x6D, 0x6D, 0x51, 0xC2, 0x45,
	0xE4, 0x85, 0xB5, 0x76, 0x62, 0x5E, 0x7E, 0xC6,
	0xF4, 0x4C, 0x42, 0xE9, 0xA6, 0x37, 0xED, 0x6B,
	0x0B, 0xFF, 0x5C, 0xB6, 0xF4, 0x06, 0xB7, 0xED,
	0xEE, 0x38, 0x6B, 0xFB, 0x5A, 0x89, 0x9F, 0xA5,
	0xAE, 0x9F, 0x24, 0x11, 0x7C, 0x4B, 0x1F, 0xE6,
	0x49, 0x28, 0x66, 0x51, 0xEC, 0xE4, 0x5B, 0x3D,
	0xC2, 0x00, 0x7C, 0xB8, 0xA1, 0x63, 0xBF, 0x05,
	0x98, 0xDA, 0x48, 0x36, 0x1C, 0x55, 0xD3, 0x9A,
	0x69, 0x16, 0x3F, 0xA8, 0xFD, 0x24, 0xCF, 0x5F,
	0x83, 0x65, 0x5D, 0x23, 0xDC, 0xA3, 0xAD, 0x96,
	0x1C, 0x62, 0xF3, 0x56, 0x20, 0x85, 0x52, 0xBB,
	0x9E, 0xD5, 0x29, 0x07, 0x70, 0x96, 0x96, 0x6D,
	0x67, 0x0C, 0x35, 0x4E, 0x4A, 0xBC, 0x98, 0x04,
	0xF1, 0x74, 0x6C, 0x08, 0xCA, 0x18, 0x21, 0x7C,
	0x32, 0x90, 0x5E, 0x46, 0x2E, 0x36, 0xCE, 0x3B,
	0xE3, 0x9E, 0x77, 0x2C, 0x18, 0x0E, 0x86, 0x03,
	0x9B, 0x27, 0x83, 0xA2, 0xEC, 0x07, 0xA2, 0x8F,
	0xB5, 0xC5, 0x5D, 0xF0, 0x6F, 0x4C, 0x52, 0xC9,
	0xDE, 0x2B, 0xCB, 0xF6, 0x95, 0x58, 0x17, 0x18,
	0x39, 0x95, 0x49, 0x7C, 0xEA, 0x95, 0x6A, 0xE5,
	0x15, 0xD2, 0x26, 0x18, 0x98, 0xFA, 0x05, 0x10,
	0x15, 0x72, 0x8E, 0x5A, 0x8A, 0xAC, 0xAA, 0x68,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

/* RFC 3526, 4. Group 15 - 3072 Bit MODP
 * Generator: 2
 * Prime: 2^3072 - 2^3008 - 1 + 2^64 * { [2^2942 pi] + 1690314 }
 */
static const u8 dh_group15_generator[1] = { 0x02 };
static const u8 dh_group15_prime[384] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xC9, 0x0F, 0xDA, 0xA2, 0x21, 0x68, 0xC2, 0x34,
	0xC4, 0xC6, 0x62, 0x8B, 0x80, 0xDC, 0x1C, 0xD1,
	0x29, 0x02, 0x4E, 0x08, 0x8A, 0x67, 0xCC, 0x74,
	0x02, 0x0B, 0xBE, 0xA6, 0x3B, 0x13, 0x9B, 0x22,
	0x51, 0x4A, 0x08, 0x79, 0x8E, 0x34, 0x04, 0xDD,
	0xEF, 0x95, 0x19, 0xB3, 0xCD, 0x3A, 0x43, 0x1B,
	0x30, 0x2B, 0x0A, 0x6D, 0xF2, 0x5F, 0x14, 0x37,
	0x4F, 0xE1, 0x35, 0x6D, 0x6D, 0x51, 0xC2, 0x45,
	0xE4, 0x85, 0xB5, 0x76, 0x62, 0x5E, 0x7E, 0xC6,
	0xF4, 0x4C, 0x42, 0xE9, 0xA6, 0x37, 0xED, 0x6B,
	0x0B, 0xFF, 0x5C, 0xB6, 0xF4, 0x06, 0xB7, 0xED,
	0xEE, 0x38, 0x6B, 0xFB, 0x5A, 0x89, 0x9F, 0xA5,
	0xAE, 0x9F, 0x24, 0x11, 0x7C, 0x4B, 0x1F, 0xE6,
	0x49, 0x28, 0x66, 0x51, 0xEC, 0xE4, 0x5B, 0x3D,
	0xC2, 0x00, 0x7C, 0xB8, 0xA1, 0x63, 0xBF, 0x05,
	0x98, 0xDA, 0x48, 0x36, 0x1C, 0x55, 0xD3, 0x9A,
	0x69, 0x16, 0x3F, 0xA8, 0xFD, 0x24, 0xCF, 0x5F,
	0x83, 0x65, 0x5D, 0x23, 0xDC, 0xA3, 0xAD, 0x96,
	0x1C, 0x62, 0xF3, 0x56, 0x20, 0x85, 0x52, 0xBB,
	0x9E, 0xD5, 0x29, 0x07, 0x70, 0x96, 0x96, 0x6D,
	0x67, 0x0C, 0x35, 0x4E, 0x4A, 0xBC, 0x98, 0x04,
	0xF1, 0x74, 0x6C, 0x08, 0xCA, 0x18, 0x21, 0x7C,
	0x32, 0x90, 0x5E, 0x46, 0x2E, 0x36, 0xCE, 0x3B,
	0xE3, 0x9E, 0x77, 0x2C, 0x18, 0x0E, 0x86, 0x03,
	0x9B, 0x27, 0x83, 0xA2, 0xEC, 0x07, 0xA2, 0x8F,
	0xB5, 0xC5, 0x5D, 0xF0, 0x6F, 0x4C, 0x52, 0xC9,
	0xDE, 0x2B, 0xCB, 0xF6, 0x95, 0x58, 0x17, 0x18,
	0x39, 0x95, 0x49, 0x7C, 0xEA, 0x95, 0x6A, 0xE5,
	0x15, 0xD2, 0x26, 0x18, 0x98, 0xFA, 0x05, 0x10,
	0x15, 0x72, 0x8E, 0x5A, 0x8A, 0xAA, 0xC4, 0x2D,
	0xAD, 0x33, 0x17, 0x0D, 0x04, 0x50, 0x7A, 0x33,
	0xA8, 0x55, 0x21, 0xAB, 0xDF, 0x1C, 0xBA, 0x64,
	0xEC, 0xFB, 0x85, 0x04, 0x58, 0xDB, 0xEF, 0x0A,
	0x8A, 0xEA, 0x71, 0x57, 0x5D, 0x06, 0x0C, 0x7D,
	0xB3, 0x97, 0x0F, 0x85, 0xA6, 0xE1, 0xE4, 0xC7,
	0xAB, 0xF5, 0xAE, 0x8C, 0xDB, 0x09, 0x33, 0xD7,
	0x1E, 0x8C, 0x94, 0xE0, 0x4A, 0x25, 0x61, 0x9D,
	0xCE, 0xE3, 0xD2, 0x26, 0x1A, 0xD2, 0xEE, 0x6B,
	0xF1, 0x2F, 0xFA, 0x06, 0xD9, 0x8A, 0x08, 0x64,
	0xD8, 0x76, 0x02, 0x73, 0x3E, 0xC8, 0x6A, 0x64,
	0x52, 0x1F, 0x2B, 0x18, 0x17, 0x7B, 0x20, 0x0C,
	0xBB, 0xE1, 0x17, 0x57, 0x7A, 0x61, 0x5D, 0x6C,
	0x77, 0x09, 0x88, 0xC0, 0xBA, 0xD9, 0x46, 0xE2,
	0x08, 0xE2, 0x4F, 0xA0, 0x74, 0xE5, 0xAB, 0x31,
	0x43, 0xDB, 0x5B, 0xFC, 0xE0, 0xFD, 0x10, 0x8E,
	0x4B, 0x82, 0xD1, 0x20, 0xA9, 0x3A, 0xD2, 0xCA,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

/* RFC 3526, 5. Group 16 - 4096 Bit MODP
 * Generator: 2
 * Prime: 2^4096 - 2^4032 - 1 + 2^64 * { [2^3966 pi] + 240904 }
 */
static const u8 dh_group16_generator[1] = { 0x02 };
static const u8 dh_group16_prime[512] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xC9, 0x0F, 0xDA, 0xA2, 0x21, 0x68, 0xC2, 0x34,
	0xC4, 0xC6, 0x62, 0x8B, 0x80, 0xDC, 0x1C, 0xD1,
	0x29, 0x02, 0x4E, 0x08, 0x8A, 0x67, 0xCC, 0x74,
	0x02, 0x0B, 0xBE, 0xA6, 0x3B, 0x13, 0x9B, 0x22,
	0x51, 0x4A, 0x08, 0x79, 0x8E, 0x34, 0x04, 0xDD,
	0xEF, 0x95, 0x19, 0xB3, 0xCD, 0x3A, 0x43, 0x1B,
	0x30, 0x2B, 0x0A, 0x6D, 0xF2, 0x5F, 0x14, 0x37,
	0x4F, 0xE1, 0x35, 0x6D, 0x6D, 0x51, 0xC2, 0x45,
	0xE4, 0x85, 0xB5, 0x76, 0x62, 0x5E, 0x7E, 0xC6,
	0xF4, 0x4C, 0x42, 0xE9, 0xA6, 0x37, 0xED, 0x6B,
	0x0B, 0xFF, 0x5C, 0xB6, 0xF4, 0x06, 0xB7, 0xED,
	0xEE, 0x38, 0x6B, 0xFB, 0x5A, 0x89, 0x9F, 0xA5,
	0xAE, 0x9F, 0x24, 0x11, 0x7C, 0x4B, 0x1F, 0xE6,
	0x49, 0x28, 0x66, 0x51, 0xEC, 0xE4, 0x5B, 0x3D,
	0xC2, 0x00, 0x7C, 0xB8, 0xA1, 0x63, 0xBF, 0x05,
	0x98, 0xDA, 0x48, 0x36, 0x1C, 0x55, 0xD3, 0x9A,
	0x69, 0x16, 0x3F, 0xA8, 0xFD, 0x24, 0xCF, 0x5F,
	0x83, 0x65, 0x5D, 0x23, 0xDC, 0xA3, 0xAD, 0x96,
	0x1C, 0x62, 0xF3, 0x56, 0x20, 0x85, 0x52, 0xBB,
	0x9E, 0xD5, 0x29, 0x07, 0x70, 0x96, 0x96, 0x6D,
	0x67, 0x0C, 0x35, 0x4E, 0x4A, 0xBC, 0x98, 0x04,
	0xF1, 0x74, 0x6C, 0x08, 0xCA, 0x18, 0x21, 0x7C,
	0x32, 0x90, 0x5E, 0x46, 0x2E, 0x36, 0xCE, 0x3B,
	0xE3, 0x9E, 0x77, 0x2C, 0x18, 0x0E, 0x86, 0x03,
	0x9B, 0x27, 0x83, 0xA2, 0xEC, 0x07, 0xA2, 0x8F,
	0xB5, 0xC5, 0x5D, 0xF0, 0x6F, 0x4C, 0x52, 0xC9,
	0xDE, 0x2B, 0xCB, 0xF6, 0x95, 0x58, 0x17, 0x18,
	0x39, 0x95, 0x49, 0x7C, 0xEA, 0x95, 0x6A, 0xE5,
	0x15, 0xD2, 0x26, 0x18, 0x98, 0xFA, 0x05, 0x10,
	0x15, 0x72, 0x8E, 0x5A, 0x8A, 0xAA, 0xC4, 0x2D,
	0xAD, 0x33, 0x17, 0x0D, 0x04, 0x50, 0x7A, 0x33,
	0xA8, 0x55, 0x21, 0xAB, 0xDF, 0x1C, 0xBA, 0x64,
	0xEC, 0xFB, 0x85, 0x04, 0x58, 0xDB, 0xEF, 0x0A,
	0x8A, 0xEA, 0x71, 0x57, 0x5D, 0x06, 0x0C, 0x7D,
	0xB3, 0x97, 0x0F, 0x85, 0xA6, 0xE1, 0xE4, 0xC7,
	0xAB, 0xF5, 0xAE, 0x8C, 0xDB, 0x09, 0x33, 0xD7,
	0x1E, 0x8C, 0x94, 0xE0, 0x4A, 0x25, 0x61, 0x9D,
	0xCE, 0xE3, 0xD2, 0x26, 0x1A, 0xD2, 0xEE, 0x6B,
	0xF1, 0x2F, 0xFA, 0x06, 0xD9, 0x8A, 0x08, 0x64,
	0xD8, 0x76, 0x02, 0x73, 0x3E, 0xC8, 0x6A, 0x64,
	0x52, 0x1F, 0x2B, 0x18, 0x17, 0x7B, 0x20, 0x0C,
	0xBB, 0xE1, 0x17, 0x57, 0x7A, 0x61, 0x5D, 0x6C,
	0x77, 0x09, 0x88, 0xC0, 0xBA, 0xD9, 0x46, 0xE2,
	0x08, 0xE2, 0x4F, 0xA0, 0x74, 0xE5, 0xAB, 0x31,
	0x43, 0xDB, 0x5B, 0xFC, 0xE0, 0xFD, 0x10, 0x8E,
	0x4B, 0x82, 0xD1, 0x20, 0xA9, 0x21, 0x08, 0x01,
	0x1A, 0x72, 0x3C, 0x12, 0xA7, 0x87, 0xE6, 0xD7,
	0x88, 0x71, 0x9A, 0x10, 0xBD, 0xBA, 0x5B, 0x26,
	0x99, 0xC3, 0x27, 0x18, 0x6A, 0xF4, 0xE2, 0x3C,
	0x1A, 0x94, 0x68, 0x34, 0xB6, 0x15, 0x0B, 0xDA,
	0x25, 0x83, 0xE9, 0xCA, 0x2A, 0xD4, 0x4C, 0xE8,
	0xDB, 0xBB, 0xC2, 0xDB, 0x04, 0xDE, 0x8E, 0xF9,
	0x2E, 0x8E, 0xFC, 0x14, 0x1F, 0xBE, 0xCA, 0xA6,
	0x28, 0x7C, 0x59, 0x47, 0x4E, 0x6B, 0xC0, 0x5D,
	0x99, 0xB2, 0x96, 0x4F, 0xA0, 0x90, 0xC3, 0xA2,
	0x23, 0x3B, 0xA1, 0x86, 0x51, 0x5B, 0xE7, 0xED,
	0x1F, 0x61, 0x29, 0x70, 0xCE, 0xE2, 0xD7, 0xAF,
	0xB8, 0x1B, 0xDD, 0x76, 0x21, 0x70, 0x48, 0x1C,
	0xD0, 0x06, 0x91, 0x27, 0xD5, 0xB0, 0x5A, 0xA9,
	0x93, 0xB4, 0xEA, 0x98, 0x8D, 0x8F, 0xDD, 0xC1,
	0x86, 0xFF, 0xB7, 0xDC, 0x90, 0xA6, 0xC0, 0x8F,
	0x4D, 0xF4, 0x35, 0xC9, 0x34, 0x06, 0x31, 0x99,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

/* RFC 3526, 6. Group 17 - 6144 Bit MODP
 * Generator: 2
 * Prime: 2^6144 - 2^6080 - 1 + 2^64 * { [2^6014 pi] + 929484 }
 */
static const u8 dh_group17_generator[1] = { 0x02 };
static const u8 dh_group17_prime[768] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xC9, 0x0F, 0xDA, 0xA2, 0x21, 0x68, 0xC2, 0x34,
	0xC4, 0xC6, 0x62, 0x8B, 0x80, 0xDC, 0x1C, 0xD1,
	0x29, 0x02, 0x4E, 0x08, 0x8A, 0x67, 0xCC, 0x74,
	0x02, 0x0B, 0xBE, 0xA6, 0x3B, 0x13, 0x9B, 0x22,
	0x51, 0x4A, 0x08, 0x79, 0x8E, 0x34, 0x04, 0xDD,
	0xEF, 0x95, 0x19, 0xB3, 0xCD, 0x3A, 0x43, 0x1B,
	0x30, 0x2B, 0x0A, 0x6D, 0xF2, 0x5F, 0x14, 0x37,
	0x4F, 0xE1, 0x35, 0x6D, 0x6D, 0x51, 0xC2, 0x45,
	0xE4, 0x85, 0xB5, 0x76, 0x62, 0x5E, 0x7E, 0xC6,
	0xF4, 0x4C, 0x42, 0xE9, 0xA6, 0x37, 0xED, 0x6B,
	0x0B, 0xFF, 0x5C, 0xB6, 0xF4, 0x06, 0xB7, 0xED,
	0xEE, 0x38, 0x6B, 0xFB, 0x5A, 0x89, 0x9F, 0xA5,
	0xAE, 0x9F, 0x24, 0x11, 0x7C, 0x4B, 0x1F, 0xE6,
	0x49, 0x28, 0x66, 0x51, 0xEC, 0xE4, 0x5B, 0x3D,
	0xC2, 0x00, 0x7C, 0xB8, 0xA1, 0x63, 0xBF, 0x05,
	0x98, 0xDA, 0x48, 0x36, 0x1C, 0x55, 0xD3, 0x9A,
	0x69, 0x16, 0x3F, 0xA8, 0xFD, 0x24, 0xCF, 0x5F,
	0x83, 0x65, 0x5D, 0x23, 0xDC, 0xA3, 0xAD, 0x96,
	0x1C, 0x62, 0xF3, 0x56, 0x20, 0x85, 0x52, 0xBB,
	0x9E, 0xD5, 0x29, 0x07, 0x70, 0x96, 0x96, 0x6D,
	0x67, 0x0C, 0x35, 0x4E, 0x4A, 0xBC, 0x98, 0x04,
	0xF1, 0x74, 0x6C, 0x08, 0xCA, 0x18, 0x21, 0x7C,
	0x32, 0x90, 0x5E, 0x46, 0x2E, 0x36, 0xCE, 0x3B,
	0xE3, 0x9E, 0x77, 0x2C, 0x18, 0x0E, 0x86, 0x03,
	0x9B, 0x27, 0x83, 0xA2, 0xEC, 0x07, 0xA2, 0x8F,
	0xB5, 0xC5, 0x5D, 0xF0, 0x6F, 0x4C, 0x52, 0xC9,
	0xDE, 0x2B, 0xCB, 0xF6, 0x95, 0x58, 0x17, 0x18,
	0x39, 0x95, 0x49, 0x7C, 0xEA, 0x95, 0x6A, 0xE5,
	0x15, 0xD2, 0x26, 0x18, 0x98, 0xFA, 0x05, 0x10,
	0x15, 0x72, 0x8E, 0x5A, 0x8A, 0xAA, 0xC4, 0x2D,
	0xAD, 0x33, 0x17, 0x0D, 0x04, 0x50, 0x7A, 0x33,
	0xA8, 0x55, 0x21, 0xAB, 0xDF, 0x1C, 0xBA, 0x64,
	0xEC, 0xFB, 0x85, 0x04, 0x58, 0xDB, 0xEF, 0x0A,
	0x8A, 0xEA, 0x71, 0x57, 0x5D, 0x06, 0x0C, 0x7D,
	0xB3, 0x97, 0x0F, 0x85, 0xA6, 0xE1, 0xE4, 0xC7,
	0xAB, 0xF5, 0xAE, 0x8C, 0xDB, 0x09, 0x33, 0xD7,
	0x1E, 0x8C, 0x94, 0xE0, 0x4A, 0x25, 0x61, 0x9D,
	0xCE, 0xE3, 0xD2, 0x26, 0x1A, 0xD2, 0xEE, 0x6B,
	0xF1, 0x2F, 0xFA, 0x06, 0xD9, 0x8A, 0x08, 0x64,
	0xD8, 0x76, 0x02, 0x73, 0x3E, 0xC8, 0x6A, 0x64,
	0x52, 0x1F, 0x2B, 0x18, 0x17, 0x7B, 0x20, 0x0C,
	0xBB, 0xE1, 0x17, 0x57, 0x7A, 0x61, 0x5D, 0x6C,
	0x77, 0x09, 0x88, 0xC0, 0xBA, 0xD9, 0x46, 0xE2,
	0x08, 0xE2, 0x4F, 0xA0, 0x74, 0xE5, 0xAB, 0x31,
	0x43, 0xDB, 0x5B, 0xFC, 0xE0, 0xFD, 0x10, 0x8E,
	0x4B, 0x82, 0xD1, 0x20, 0xA9, 0x21, 0x08, 0x01,
	0x1A, 0x72, 0x3C, 0x12, 0xA7, 0x87, 0xE6, 0xD7,
	0x88, 0x71, 0x9A, 0x10, 0xBD, 0xBA, 0x5B, 0x26,
	0x99, 0xC3, 0x27, 0x18, 0x6A, 0xF4, 0xE2, 0x3C,
	0x1A, 0x94, 0x68, 0x34, 0xB6, 0x15, 0x0B, 0xDA,
	0x25, 0x83, 0xE9, 0xCA, 0x2A, 0xD4, 0x4C, 0xE8,
	0xDB, 0xBB, 0xC2, 0xDB, 0x04, 0xDE, 0x8E, 0xF9,
	0x2E, 0x8E, 0xFC, 0x14, 0x1F, 0xBE, 0xCA, 0xA6,
	0x28, 0x7C, 0x59, 0x47, 0x4E, 0x6B, 0xC0, 0x5D,
	0x99, 0xB2, 0x96, 0x4F, 0xA0, 0x90, 0xC3, 0xA2,
	0x23, 0x3B, 0xA1, 0x86, 0x51, 0x5B, 0xE7, 0xED,
	0x1F, 0x61, 0x29, 0x70, 0xCE, 0xE2, 0xD7, 0xAF,
	0xB8, 0x1B, 0xDD, 0x76, 0x21, 0x70, 0x48, 0x1C,
	0xD0, 0x06, 0x91, 0x27, 0xD5, 0xB0, 0x5A, 0xA9,
	0x93, 0xB4, 0xEA, 0x98, 0x8D, 0x8F, 0xDD, 0xC1,
	0x86, 0xFF, 0xB7, 0xDC, 0x90, 0xA6, 0xC0, 0x8F,
	0x4D, 0xF4, 0x35, 0xC9, 0x34, 0x02, 0x84, 0x92,
	0x36, 0xC3, 0xFA, 0xB4, 0xD2, 0x7C, 0x70, 0x26,
	0xC1, 0xD4, 0xDC, 0xB2, 0x60, 0x26, 0x46, 0xDE,
	0xC9, 0x75, 0x1E, 0x76, 0x3D, 0xBA, 0x37, 0xBD,
	0xF8, 0xFF, 0x94, 0x06, 0xAD, 0x9E, 0x53, 0x0E,
	0xE5, 0xDB, 0x38, 0x2F, 0x41, 0x30, 0x01, 0xAE,
	0xB0, 0x6A, 0x53, 0xED, 0x90, 0x27, 0xD8, 0x31,
	0x17, 0x97, 0x27, 0xB0, 0x86, 0x5A, 0x89, 0x18,
	0xDA, 0x3E, 0xDB, 0xEB, 0xCF, 0x9B, 0x14, 0xED,
	0x44, 0xCE, 0x6C, 0xBA, 0xCE, 0xD4, 0xBB, 0x1B,
	0xDB, 0x7F, 0x14, 0x47, 0xE6, 0xCC, 0x25, 0x4B,
	0x33, 0x20, 0x51, 0x51, 0x2B, 0xD7, 0xAF, 0x42,
	0x6F, 0xB8, 0xF4, 0x01, 0x37, 0x8C, 0xD2, 0xBF,
	0x59, 0x83, 0xCA, 0x01, 0xC6, 0x4B, 0x92, 0xEC,
	0xF0, 0x32, 0xEA, 0x15, 0xD1, 0x72, 0x1D, 0x03,
	0xF4, 0x82, 0xD7, 0xCE, 0x6E, 0x74, 0xFE, 0xF6,
	0xD5, 0x5E, 0x70, 0x2F, 0x46, 0x98, 0x0C, 0x82,
	0xB5, 0xA8, 0x40, 0x31, 0x90, 0x0B, 0x1C, 0x9E,
	0x59, 0xE7, 0xC9, 0x7F, 0xBE, 0xC7, 0xE8, 0xF3,
	0x23, 0xA9, 0x7A, 0x7E, 0x36, 0xCC, 0x88, 0xBE,
	0x0F, 0x1D, 0x45, 0xB7, 0xFF, 0x58, 0x5A, 0xC5,
	0x4B, 0xD4, 0x07, 0xB2, 0x2B, 0x41, 0x54, 0xAA,
	0xCC, 0x8F, 0x6D, 0x7E, 0xBF, 0x48, 0xE1, 0xD8,
	0x14, 0xCC, 0x5E, 0xD2, 0x0F, 0x80, 0x37, 0xE0,
	0xA7, 0x97, 0x15, 0xEE, 0xF2, 0x9B, 0xE3, 0x28,
	0x06, 0xA1, 0xD5, 0x8B, 0xB7, 0xC5, 0xDA, 0x76,
	0xF5, 0x50, 0xAA, 0x3D, 0x8A, 0x1F, 0xBF, 0xF0,
	0xEB, 0x19, 0xCC, 0xB1, 0xA3, 0x13, 0xD5, 0x5C,
	0xDA, 0x56, 0xC9, 0xEC, 0x2E, 0xF2, 0x96, 0x32,
	0x38, 0x7F, 0xE8, 0xD7, 0x6E, 0x3C, 0x04, 0x68,
	0x04, 0x3E, 0x8F, 0x66, 0x3F, 0x48, 0x60, 0xEE,
	0x12, 0xBF, 0x2D, 0x5B, 0x0B, 0x74, 0x74, 0xD6,
	0xE6, 0x94, 0xF9, 0x1E, 0x6D, 0xCC, 0x40, 0x24,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

/* RFC 3526, 7. Group 18 - 8192 Bit MODP
 * Generator: 2
 * Prime: 2^8192 - 2^8128 - 1 + 2^64 * { [2^8062 pi] + 4743158 }
 */
static const u8 dh_group18_generator[1] = { 0x02 };
static const u8 dh_group18_prime[1024] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xC9, 0x0F, 0xDA, 0xA2, 0x21, 0x68, 0xC2, 0x34,
	0xC4, 0xC6, 0x62, 0x8B, 0x80, 0xDC, 0x1C, 0xD1,
	0x29, 0x02, 0x4E, 0x08, 0x8A, 0x67, 0xCC, 0x74,
	0x02, 0x0B, 0xBE, 0xA6, 0x3B, 0x13, 0x9B, 0x22,
	0x51, 0x4A, 0x08, 0x79, 0x8E, 0x34, 0x04, 0xDD,
	0xEF, 0x95, 0x19, 0xB3, 0xCD, 0x3A, 0x43, 0x1B,
	0x30, 0x2B, 0x0A, 0x6D, 0xF2, 0x5F, 0x14, 0x37,
	0x4F, 0xE1, 0x35, 0x6D, 0x6D, 0x51, 0xC2, 0x45,
	0xE4, 0x85, 0xB5, 0x76, 0x62, 0x5E, 0x7E, 0xC6,
	0xF4, 0x4C, 0x42, 0xE9, 0xA6, 0x37, 0xED, 0x6B,
	0x0B, 0xFF, 0x5C, 0xB6, 0xF4, 0x06, 0xB7, 0xED,
	0xEE, 0x38, 0x6B, 0xFB, 0x5A, 0x89, 0x9F, 0xA5,
	0xAE, 0x9F, 0x24, 0x11, 0x7C, 0x4B, 0x1F, 0xE6,
	0x49, 0x28, 0x66, 0x51, 0xEC, 0xE4, 0x5B, 0x3D,
	0xC2, 0x00, 0x7C, 0xB8, 0xA1, 0x63, 0xBF, 0x05,
	0x98, 0xDA, 0x48, 0x36, 0x1C, 0x55, 0xD3, 0x9A,
	0x69, 0x16, 0x3F, 0xA8, 0xFD, 0x24, 0xCF, 0x5F,
	0x83, 0x65, 0x5D, 0x23, 0xDC, 0xA3, 0xAD, 0x96,
	0x1C, 0x62, 0xF3, 0x56, 0x20, 0x85, 0x52, 0xBB,
	0x9E, 0xD5, 0x29, 0x07, 0x70, 0x96, 0x96, 0x6D,
	0x67, 0x0C, 0x35, 0x4E, 0x4A, 0xBC, 0x98, 0x04,
	0xF1, 0x74, 0x6C, 0x08, 0xCA, 0x18, 0x21, 0x7C,
	0x32, 0x90, 0x5E, 0x46, 0x2E, 0x36, 0xCE, 0x3B,
	0xE3, 0x9E, 0x77, 0x2C, 0x18, 0x0E, 0x86, 0x03,
	0x9B, 0x27, 0x83, 0xA2, 0xEC, 0x07, 0xA2, 0x8F,
	0xB5, 0xC5, 0x5D, 0xF0, 0x6F, 0x4C, 0x52, 0xC9,
	0xDE, 0x2B, 0xCB, 0xF6, 0x95, 0x58, 0x17, 0x18,
	0x39, 0x95, 0x49, 0x7C, 0xEA, 0x95, 0x6A, 0xE5,
	0x15, 0xD2, 0x26, 0x18, 0x98, 0xFA, 0x05, 0x10,
	0x15, 0x72, 0x8E, 0x5A, 0x8A, 0xAA, 0xC4, 0x2D,
	0xAD, 0x33, 0x17, 0x0D, 0x04, 0x50, 0x7A, 0x33,
	0xA8, 0x55, 0x21, 0xAB, 0xDF, 0x1C, 0xBA, 0x64,
	0xEC, 0xFB, 0x85, 0x04, 0x58, 0xDB, 0xEF, 0x0A,
	0x8A, 0xEA, 0x71, 0x57, 0x5D, 0x06, 0x0C, 0x7D,
	0xB3, 0x97, 0x0F, 0x85, 0xA6, 0xE1, 0xE4, 0xC7,
	0xAB, 0xF5, 0xAE, 0x8C, 0xDB, 0x09, 0x33, 0xD7,
	0x1E, 0x8C, 0x94, 0xE0, 0x4A, 0x25, 0x61, 0x9D,
	0xCE, 0xE3, 0xD2, 0x26, 0x1A, 0xD2, 0xEE, 0x6B,
	0xF1, 0x2F, 0xFA, 0x06, 0xD9, 0x8A, 0x08, 0x64,
	0xD8, 0x76, 0x02, 0x73, 0x3E, 0xC8, 0x6A, 0x64,
	0x52, 0x1F, 0x2B, 0x18, 0x17, 0x7B, 0x20, 0x0C,
	0xBB, 0xE1, 0x17, 0x57, 0x7A, 0x61, 0x5D, 0x6C,
	0x77, 0x09, 0x88, 0xC0, 0xBA, 0xD9, 0x46, 0xE2,
	0x08, 0xE2, 0x4F, 0xA0, 0x74, 0xE5, 0xAB, 0x31,
	0x43, 0xDB, 0x5B, 0xFC, 0xE0, 0xFD, 0x10, 0x8E,
	0x4B, 0x82, 0xD1, 0x20, 0xA9, 0x21, 0x08, 0x01,
	0x1A, 0x72, 0x3C, 0x12, 0xA7, 0x87, 0xE6, 0xD7,
	0x88, 0x71, 0x9A, 0x10, 0xBD, 0xBA, 0x5B, 0x26,
	0x99, 0xC3, 0x27, 0x18, 0x6A, 0xF4, 0xE2, 0x3C,
	0x1A, 0x94, 0x68, 0x34, 0xB6, 0x15, 0x0B, 0xDA,
	0x25, 0x83, 0xE9, 0xCA, 0x2A, 0xD4, 0x4C, 0xE8,
	0xDB, 0xBB, 0xC2, 0xDB, 0x04, 0xDE, 0x8E, 0xF9,
	0x2E, 0x8E, 0xFC, 0x14, 0x1F, 0xBE, 0xCA, 0xA6,
	0x28, 0x7C, 0x59, 0x47, 0x4E, 0x6B, 0xC0, 0x5D,
	0x99, 0xB2, 0x96, 0x4F, 0xA0, 0x90, 0xC3, 0xA2,
	0x23, 0x3B, 0xA1, 0x86, 0x51, 0x5B, 0xE7, 0xED,
	0x1F, 0x61, 0x29, 0x70, 0xCE, 0xE2, 0xD7, 0xAF,
	0xB8, 0x1B, 0xDD, 0x76, 0x21, 0x70, 0x48, 0x1C,
	0xD0, 0x06, 0x91, 0x27, 0xD5, 0xB0, 0x5A, 0xA9,
	0x93, 0xB4, 0xEA, 0x98, 0x8D, 0x8F, 0xDD, 0xC1,
	0x86, 0xFF, 0xB7, 0xDC, 0x90, 0xA6, 0xC0, 0x8F,
	0x4D, 0xF4, 0x35, 0xC9, 0x34, 0x02, 0x84, 0x92,
	0x36, 0xC3, 0xFA, 0xB4, 0xD2, 0x7C, 0x70, 0x26,
	0xC1, 0xD4, 0xDC, 0xB2, 0x60, 0x26, 0x46, 0xDE,
	0xC9, 0x75, 0x1E, 0x76, 0x3D, 0xBA, 0x37, 0xBD,
	0xF8, 0xFF, 0x94, 0x06, 0xAD, 0x9E, 0x53, 0x0E,
	0xE5, 0xDB, 0x38, 0x2F, 0x41, 0x30, 0x01, 0xAE,
	0xB0, 0x6A, 0x53, 0xED, 0x90, 0x27, 0xD8, 0x31,
	0x17, 0x97, 0x27, 0xB0, 0x86, 0x5A, 0x89, 0x18,
	0xDA, 0x3E, 0xDB, 0xEB, 0xCF, 0x9B, 0x14, 0xED,
	0x44, 0xCE, 0x6C, 0xBA, 0xCE, 0xD4, 0xBB, 0x1B,
	0xDB, 0x7F, 0x14, 0x47, 0xE6, 0xCC, 0x25, 0x4B,
	0x33, 0x20, 0x51, 0x51, 0x2B, 0xD7, 0xAF, 0x42,
	0x6F, 0xB8, 0xF4, 0x01, 0x37, 0x8C, 0xD2, 0xBF,
	0x59, 0x83, 0xCA, 0x01, 0xC6, 0x4B, 0x92, 0xEC,
	0xF0, 0x32, 0xEA, 0x15, 0xD1, 0x72, 0x1D, 0x03,
	0xF4, 0x82, 0xD7, 0xCE, 0x6E, 0x74, 0xFE, 0xF6,
	0xD5, 0x5E, 0x70, 0x2F, 0x46, 0x98, 0x0C, 0x82,
	0xB5, 0xA8, 0x40, 0x31, 0x90, 0x0B, 0x1C, 0x9E,
	0x59, 0xE7, 0xC9, 0x7F, 0xBE, 0xC7, 0xE8, 0xF3,
	0x23, 0xA9, 0x7A, 0x7E, 0x36, 0xCC, 0x88, 0xBE,
	0x0F, 0x1D, 0x45, 0xB7, 0xFF, 0x58, 0x5A, 0xC5,
	0x4B, 0xD4, 0x07, 0xB2, 0x2B, 0x41, 0x54, 0xAA,
	0xCC, 0x8F, 0x6D, 0x7E, 0xBF, 0x48, 0xE1, 0xD8,
	0x14, 0xCC, 0x5E, 0xD2, 0x0F, 0x80, 0x37, 0xE0,
	0xA7, 0x97, 0x15, 0xEE, 0xF2, 0x9B, 0xE3, 0x28,
	0x06, 0xA1, 0xD5, 0x8B, 0xB7, 0xC5, 0xDA, 0x76,
	0xF5, 0x50, 0xAA, 0x3D, 0x8A, 0x1F, 0xBF, 0xF0,
	0xEB, 0x19, 0xCC, 0xB1, 0xA3, 0x13, 0xD5, 0x5C,
	0xDA, 0x56, 0xC9, 0xEC, 0x2E, 0xF2, 0x96, 0x32,
	0x38, 0x7F, 0xE8, 0xD7, 0x6E, 0x3C, 0x04, 0x68,
	0x04, 0x3E, 0x8F, 0x66, 0x3F, 0x48, 0x60, 0xEE,
	0x12, 0xBF, 0x2D, 0x5B, 0x0B, 0x74, 0x74, 0xD6,
	0xE6, 0x94, 0xF9, 0x1E, 0x6D, 0xBE, 0x11, 0x59,
	0x74, 0xA3, 0x92, 0x6F, 0x12, 0xFE, 0xE5, 0xE4,
	0x38, 0x77, 0x7C, 0xB6, 0xA9, 0x32, 0xDF, 0x8C,
	0xD8, 0xBE, 0xC4, 0xD0, 0x73, 0xB9, 0x31, 0xBA,
	0x3B, 0xC8, 0x32, 0xB6, 0x8D, 0x9D, 0xD3, 0x00,
	0x74, 0x1F, 0xA7, 0xBF, 0x8A, 0xFC, 0x47, 0xED,
	0x25, 0x76, 0xF6, 0x93, 0x6B, 0xA4, 0x24, 0x66,
	0x3A, 0xAB, 0x63, 0x9C, 0x5A, 0xE4, 0xF5, 0x68,
	0x34, 0x23, 0xB4, 0x74, 0x2B, 0xF1, 0xC9, 0x78,
	0x23, 0x8F, 0x16, 0xCB, 0xE3, 0x9D, 0x65, 0x2D,
	0xE3, 0xFD, 0xB8, 0xBE, 0xFC, 0x84, 0x8A, 0xD9,
	0x22, 0x22, 0x2E, 0x04, 0xA4, 0x03, 0x7C, 0x07,
	0x13, 0xEB, 0x57, 0xA8, 0x1A, 0x23, 0xF0, 0xC7,
	0x34, 0x73, 0xFC, 0x64, 0x6C, 0xEA, 0x30, 0x6B,
	0x4B, 0xCB, 0xC8, 0x86, 0x2F, 0x83, 0x85, 0xDD,
	0xFA, 0x9D, 0x4B, 0x7F, 0xA2, 0xC0, 0x87, 0xE8,
	0x79, 0x68, 0x33, 0x03, 0xED, 0x5B, 0xDD, 0x3A,
	0x06, 0x2B, 0x3C, 0xF5, 0xB3, 0xA2, 0x78, 0xA6,
	0x6D, 0x2A, 0x13, 0xF8, 0x3F, 0x44, 0xF8, 0x2D,
	0xDF, 0x31, 0x0E, 0xE0, 0x74, 0xAB, 0x6A, 0x36,
	0x45, 0x97, 0xE8, 0x99, 0xA0, 0x25, 0x5D, 0xC1,
	0x64, 0xF3, 0x1C, 0xC5, 0x08, 0x46, 0x85, 0x1D,
	0xF9, 0xAB, 0x48, 0x19, 0x5D, 0xED, 0x7E, 0xA1,
	0xB1, 0xD5, 0x10, 0xBD, 0x7E, 0xE7, 0x4D, 0x73,
	0xFA, 0xF3, 0x6B, 0xC3, 0x1E, 0xCF, 0xA2, 0x68,
	0x35, 0x90, 0x46, 0xF4, 0xEB, 0x87, 0x9F, 0x92,
	0x40, 0x09, 0x43, 0x8B, 0x48, 0x1C, 0x6C, 0xD7,
	0x88, 0x9A, 0x00, 0x2E, 0xD5, 0xEE, 0x38, 0x2B,
	0xC9, 0x19, 0x0D, 0xA6, 0xFC, 0x02, 0x6E, 0x47,
	0x95, 0x58, 0xE4, 0x47, 0x56, 0x77, 0xE9, 0xAA,
	0x9E, 0x30, 0x50, 0xE2, 0x76, 0x56, 0x94, 0xDF,
	0xC8, 0x1F, 0x56, 0xE8, 0x80, 0xB9, 0x6E, 0x71,
	0x60, 0xC9, 0x80, 0xDD, 0x98, 0xED, 0xD3, 0xDF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

#endif /* ALL_DH_GROUPS */


#define DH_GROUP(id) \
{ id, dh_group ## id ## _generator, sizeof(dh_group ## id ## _generator), \
dh_group ## id ## _prime, sizeof(dh_group ## id ## _prime) }
		

static struct dh_group dh_groups[] = {
	DH_GROUP(5),
#ifdef ALL_DH_GROUPS
	DH_GROUP(1),
	DH_GROUP(2),
	DH_GROUP(14),
	DH_GROUP(15),
	DH_GROUP(16),
	DH_GROUP(17),
	DH_GROUP(18)
#endif /* ALL_DH_GROUPS */
};

#define NUM_DH_GROUPS (sizeof(dh_groups) / sizeof(dh_groups[0]))


const struct dh_group * dh_groups_get(int id)
{
	size_t i;

	for (i = 0; i < NUM_DH_GROUPS; i++) {
		if (dh_groups[i].id == id)
			return &dh_groups[i];
	}
	return NULL;
}


/**
 * dh_init - Initialize Diffie-Hellman handshake
 * @dh: Selected Diffie-Hellman group
 * @priv: Pointer for returning Diffie-Hellman private key
 * Returns: Diffie-Hellman public value
 */
struct wpabuf * dh_init(const struct dh_group *dh, struct wpabuf **priv)
{
	struct wpabuf *pv;
	size_t pv_len;

	if (dh == NULL)
		return NULL;

	wpabuf_free(*priv);
	*priv = wpabuf_alloc(dh->prime_len);
	if (*priv == NULL)
		return NULL;

	if (os_get_random(wpabuf_put(*priv, dh->prime_len), dh->prime_len)) {
		wpabuf_free(*priv);
		*priv = NULL;
		return NULL;
	}

	if (os_memcmp(wpabuf_head(*priv), dh->prime, dh->prime_len) > 0) {
		/* Make sure private value is smaller than prime */
		*(wpabuf_mhead_u8(*priv)) = 0;
	}
	wpa_hexdump_buf_key(MSG_DEBUG, "DH: private value", *priv);

	pv_len = dh->prime_len;
	pv = wpabuf_alloc(pv_len);
	if (pv == NULL)
		return NULL;
	if (crypto_mod_exp(dh->generator, dh->generator_len,
			   wpabuf_head(*priv), wpabuf_len(*priv),
			   dh->prime, dh->prime_len, wpabuf_mhead(pv),
			   &pv_len) < 0) {
		wpabuf_free(pv);
		wpa_printf(MSG_INFO, "DH: crypto_mod_exp failed");
		return NULL;
	}
	wpabuf_put(pv, pv_len);
	wpa_hexdump_buf(MSG_DEBUG, "DH: public value", pv);

	return pv;
}


/**
 * dh_derive_shared - Derive shared Diffie-Hellman key
 * @peer_public: Diffie-Hellman public value from peer
 * @own_private: Diffie-Hellman private key from dh_init()
 * @dh: Selected Diffie-Hellman group
 * Returns: Diffie-Hellman shared key
 */
struct wpabuf * dh_derive_shared(const struct wpabuf *peer_public,
				 const struct wpabuf *own_private,
				 const struct dh_group *dh)
{
	struct wpabuf *shared;
	size_t shared_len;

	if (dh == NULL || peer_public == NULL || own_private == NULL)
		return NULL;

	shared_len = dh->prime_len;
	shared = wpabuf_alloc(shared_len);
	if (shared == NULL)
		return NULL;
	if (crypto_mod_exp(wpabuf_head(peer_public), wpabuf_len(peer_public),
			   wpabuf_head(own_private), wpabuf_len(own_private),
			   dh->prime, dh->prime_len,
			   wpabuf_mhead(shared), &shared_len) < 0) {
		wpabuf_free(shared);
		wpa_printf(MSG_INFO, "DH: crypto_mod_exp failed");
		return NULL;
	}
	wpabuf_put(shared, shared_len);
	wpa_hexdump_buf_key(MSG_DEBUG, "DH: shared key", shared);

	return shared;
}
