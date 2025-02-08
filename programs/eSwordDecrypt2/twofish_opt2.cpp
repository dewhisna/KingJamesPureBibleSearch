/****************************************************************************
**
** Copyright (C) 2024-2025 Donna Whisnant, a.k.a. Dewtronics.
** Contact: http://www.dewtronics.com/
**
** This file is part of the KJVCanOpener Application as originally written
** and developed for Bethel Church, Festus, MO.
**
** GNU General Public License Usage
** This file may be used under the terms of the GNU General Public License
** version 3.0 as published by the Free Software Foundation and appearing
** in the file gpl-3.0.txt included in the packaging of this file. Please
** review the following information to ensure the GNU General Public License
** version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and
** Dewtronics.
**
****************************************************************************/

/*
   compiler is gcc(egcs-2.91.66)
   flags are -O3 -fomit-frame-pointer -Wall
   Processor is 233Mhz Pentium II (Deschutes)
   OS is Linux 2.2.16

   Max encryption speed I've seen (in mulit-user mode even, although single
   user mode probably won't save more than a couple clocks):

   encs/sec = 506115.904591
   bytes/sec = 8097854.473457
   KB/sec = 7908.061009
   MB/sec = 7.722716
   approx clocks/enc (for 233Mhz) = 461.027466

   I easily beat the best C implementations (the best being MSC @ 600 clocks),
   so the target is the assembly implementations...

   according to twofish docs, fully tuned *assembly* (in clocks):
   compiled is 285          (shouldn't be able to do this)  (12.5 MB/sec)
   full keyed is 315        (if I get to 460, maybe this is possible but
							 I *really* doubt it)  (11.3 MB/sec)
   partially keyed is 460   (I'm *really* close) (7.7 MB/sec)
   minimal keying is 720    (I've beat this -their C did too) (4.9 MB/sec)

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tables.h"

typedef uint32_t u32;
typedef uint8_t BYTE;

#define RS_MOD 0x14D
#define RHO 0x01010101L

#include <zlib.h>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <assert.h>
#include "twofish_opt2.h"
#define CHUNK 16384
#define DEBUG_SQLITEPLUS 0			// Enable for debugging SQLitePlus Decrypt and Decompress operations

#ifndef UNUSED
#define UNUSED(x) ((void)(x))
#endif

/*
   gcc is smart enough to convert these to roll instructions.  If you want
   to see for yourself, either do gcc -O3 -S, or change the |'s to +'s and
   see how slow things get (you lose about 30-50 clocks) :).
*/
#define ROL(x,n) (((x) << ((n) & 0x1F)) | ((x) >> (32-((n) & 0x1F))))
#define ROR(x,n) (((x) >> ((n) & 0x1F)) | ((x) << (32-((n) & 0x1F))))

#if BIG_ENDIAN == 1
#define BSWAP(x) (((ROR(x,8) & 0xFF00FF00) | (ROL(x,8) & 0x00FF00FF)))
#else
#define BSWAP(x) (x)
#endif

#define _b(x, N) (((x) >> (N*8)) & 0xFF)

/* just casting to byte (instead of masking with 0xFF saves *tons* of clocks
   (around 50) */
#define b0(x) ((BYTE)(x))
/* this saved 10 clocks */
#define b1(x) ((BYTE)((x) >> 8))
/* use byte cast here saves around 10 clocks */
#define b2(x) (BYTE)((x) >> 16)
/* don't need to mask since all bits are in lower 8 - byte cast here saves
   nothing, but hey, what the hell, it doesn't hurt any */
#define b3(x) (BYTE)((x) >> 24)

#define BYTEARRAY_TO_U32(r) ((r[0] << 24) ^ (r[1] << 16) ^ (r[2] << 8) ^ r[3])
#define BYTES_TO_U32(r0, r1, r2, r3) ((r0 << 24) ^ (r1 << 16) ^ (r2 << 8) ^ r3)

void printSubkeys(u32 K[40])
{
	int i;
	printf("round subkeys\n");
	for (i=0;i<40;i+=2)
		printf("%08X %08X\n", K[i], K[i+1]);
}

/*
   multiply two polynomials represented as u32's, actually called with BYTES,
   but since I'm not really going to too much work to optimize key setup (since
   raw encryption speed is what I'm after), big deal.
*/
u32 polyMult(u32 a, u32 b)
{
	u32 t=0;
	while (a)
	{
		/*printf("A=%X  B=%X  T=%X\n", a, b, t);*/
		if (a&1) t^=b;
		b <<= 1;
		a >>= 1;
	}
	return t;
}

/* take the polynomial t and return the t % modulus in GF(256) */
u32 gfMod(u32 t, u32 modulus)
{
	int i;
	u32 tt;

	modulus <<= 7;
	for (i = 0; i < 8; i++)
	{
		tt = t ^ modulus;
		if (tt < t) t = tt;
		modulus >>= 1;
	}
	return t;
}

/*multiply a and b and return the modulus */
#define gfMult(a, b, modulus) gfMod(polyMult(a, b), modulus)

/* return a u32 containing the result of multiplying the RS Code matrix
   by the sd matrix
*/
u32 RSMatrixMultiply(BYTE sd[8])
{
	int j, k;
	BYTE t;
	BYTE result[4];

	for (j = 0; j < 4; j++)
	{
		t = 0;
		for (k = 0; k < 8; k++)
		{
			/*printf("t=%X  %X\n", t, gfMult(RS[j][k], sd[k], RS_MOD));*/
			t ^= gfMult(RS[j][k], sd[k], RS_MOD);
		}
		result[3-j] = t;
	}
	return BYTEARRAY_TO_U32(result);
}

/* the Zero-keyed h function (used by the key setup routine) */
u32 h(u32 X, u32 L[4], int k)
{
	BYTE y0, y1, y2, y3;
	BYTE z0, z1, z2, z3;
	y0 = b0(X);
	y1 = b1(X);
	y2 = b2(X);
	y3 = b3(X);

	switch(k)
	{
		case 4:
			y0 = Q1[y0] ^ b0(L[3]);
			y1 = Q0[y1] ^ b1(L[3]);
			y2 = Q0[y2] ^ b2(L[3]);
			y3 = Q1[y3] ^ b3(L[3]);
			[[fallthrough]];
		case 3:
			y0 = Q1[y0] ^ b0(L[2]);
			y1 = Q1[y1] ^ b1(L[2]);
			y2 = Q0[y2] ^ b2(L[2]);
			y3 = Q0[y3] ^ b3(L[2]);
			[[fallthrough]];
		case 2:
			y0 = Q1[  Q0 [ Q0[y0] ^ b0(L[1]) ] ^ b0(L[0]) ];
			y1 = Q0[  Q0 [ Q1[y1] ^ b1(L[1]) ] ^ b1(L[0]) ];
			y2 = Q1[  Q1 [ Q0[y2] ^ b2(L[1]) ] ^ b2(L[0]) ];
			y3 = Q0[  Q1 [ Q1[y3] ^ b3(L[1]) ] ^ b3(L[0]) ];
	}

	/* inline the MDS matrix multiply */
	z0 = multEF[y0] ^ y1 ^         multEF[y2] ^ mult5B[y3];
	z1 = multEF[y0] ^ mult5B[y1] ^ y2 ^         multEF[y3];
	z2 = mult5B[y0] ^ multEF[y1] ^ multEF[y2] ^ y3;
	z3 = y0 ^         multEF[y1] ^ mult5B[y2] ^ mult5B[y3];

	return BYTES_TO_U32(z0, z1, z2, z3);
}

/* given the Sbox keys, create the fully keyed QF */
void fullKey(u32 L[4], int k, u32 QF[4][256])
{
	BYTE y0, y1, y2, y3;

	int i;

	/* for all input values to the Q permutations */
	for (i=0; i<256; i++)
	{
		/* run the Q permutations */
		y0 = i; y1=i; y2=i; y3=i;
		switch(k)
		{
			case 4:
			y0 = Q1[y0] ^ b0(L[3]);
			y1 = Q0[y1] ^ b1(L[3]);
			y2 = Q0[y2] ^ b2(L[3]);
			y3 = Q1[y3] ^ b3(L[3]);
			[[fallthrough]];
			case 3:
			y0 = Q1[y0] ^ b0(L[2]);
			y1 = Q1[y1] ^ b1(L[2]);
			y2 = Q0[y2] ^ b2(L[2]);
			y3 = Q0[y3] ^ b3(L[2]);
			[[fallthrough]];
			case 2:
			y0 = Q1[  Q0 [ Q0[y0] ^ b0(L[1]) ] ^ b0(L[0]) ];
			y1 = Q0[  Q0 [ Q1[y1] ^ b1(L[1]) ] ^ b1(L[0]) ];
			y2 = Q1[  Q1 [ Q0[y2] ^ b2(L[1]) ] ^ b2(L[0]) ];
			y3 = Q0[  Q1 [ Q1[y3] ^ b3(L[1]) ] ^ b3(L[0]) ];
		}

		/* now do the partial MDS matrix multiplies */
		QF[0][i] = ((multEF[y0] << 24)
				| (multEF[y0] << 16)
				| (mult5B[y0] << 8)
				| y0);
		QF[1][i] = ((y1 << 24)
				| (mult5B[y1] << 16)
				| (multEF[y1] << 8)
				| multEF[y1]);
		QF[2][i] = ((multEF[y2] << 24)
				| (y2 << 16)
				| (multEF[y2] << 8)
				| mult5B[y2]);
		QF[3][i] = ((mult5B[y3] << 24)
				| (multEF[y3] << 16)
				| (y3 << 8)
				| mult5B[y3]);
	}
}

void printRound(int round, u32 R0, u32 R1, u32 R2, u32 R3, u32 K1, u32 K2)
{
	UNUSED(K1);
	UNUSED(K2);
	printf("round[%d] ['0x%08XL', '0x%08XL', '0x%08XL', '0x%08XL']\n",
	   round, R0, R1, R2, R3);
}

/* fully keyed h (aka g) function */
#define fkh(X) (S[0][b0(X)]^S[1][b1(X)]^S[2][b2(X)]^S[3][b3(X)])

/* one encryption round */
#define ENC_ROUND(R0, R1, R2, R3, round) \
	T0 = fkh(R0); \
	T1 = fkh(ROL(R1, 8)); \
	R2 = ROR(R2 ^ (T1 + T0 + K[2*round+8]), 1); \
	R3 = ROL(R3, 1) ^ (2*T1 + T0 + K[2*round+9]);

inline void encrypt(u32 K[40], u32 S[4][256], BYTE PT[16])
{
	u32 R0, R1, R2, R3;
	u32 T0, T1;

	/* load/byteswap/whiten input */
	R3 = K[3] ^ BSWAP(((u32*)PT)[3]);
	R2 = K[2] ^ BSWAP(((u32*)PT)[2]);
	R1 = K[1] ^ BSWAP(((u32*)PT)[1]);
	R0 = K[0] ^ BSWAP(((u32*)PT)[0]);

	ENC_ROUND(R0, R1, R2, R3, 0);
	ENC_ROUND(R2, R3, R0, R1, 1);
	ENC_ROUND(R0, R1, R2, R3, 2);
	ENC_ROUND(R2, R3, R0, R1, 3);
	ENC_ROUND(R0, R1, R2, R3, 4);
	ENC_ROUND(R2, R3, R0, R1, 5);
	ENC_ROUND(R0, R1, R2, R3, 6);
	ENC_ROUND(R2, R3, R0, R1, 7);
	ENC_ROUND(R0, R1, R2, R3, 8);
	ENC_ROUND(R2, R3, R0, R1, 9);
	ENC_ROUND(R0, R1, R2, R3, 10);
	ENC_ROUND(R2, R3, R0, R1, 11);
	ENC_ROUND(R0, R1, R2, R3, 12);
	ENC_ROUND(R2, R3, R0, R1, 13);
	ENC_ROUND(R0, R1, R2, R3, 14);
	ENC_ROUND(R2, R3, R0, R1, 15);

	/* load/byteswap/whiten output */
	((u32*)PT)[3] = BSWAP(R1 ^ K[7]);
	((u32*)PT)[2] = BSWAP(R0 ^ K[6]);
	((u32*)PT)[1] = BSWAP(R3 ^ K[5]);
	((u32*)PT)[0] = BSWAP(R2 ^ K[4]);
}

/* one decryption round */
#define DEC_ROUND(R0, R1, R2, R3, round) \
	T0 = fkh(R0); \
	T1 = fkh(ROL(R1, 8)); \
	R2 = ROL(R2, 1) ^ (T0 + T1 + K[2*round+8]); \
	R3 = ROR(R3 ^ (T0 + 2*T1 + K[2*round+9]), 1);

inline void decrypt(u32 K[40], u32 S[4][256], BYTE PT[16])
{
	u32 T0, T1;
	u32 R0, R1, R2, R3;

	/* load/byteswap/whiten input */
	R3 = K[7] ^ BSWAP(((u32*)PT)[3]);
	R2 = K[6] ^ BSWAP(((u32*)PT)[2]);
	R1 = K[5] ^ BSWAP(((u32*)PT)[1]);
	R0 = K[4] ^ BSWAP(((u32*)PT)[0]);

	DEC_ROUND(R0, R1, R2, R3, 15);
	DEC_ROUND(R2, R3, R0, R1, 14);
	DEC_ROUND(R0, R1, R2, R3, 13);
	DEC_ROUND(R2, R3, R0, R1, 12);
	DEC_ROUND(R0, R1, R2, R3, 11);
	DEC_ROUND(R2, R3, R0, R1, 10);
	DEC_ROUND(R0, R1, R2, R3, 9);
	DEC_ROUND(R2, R3, R0, R1, 8);
	DEC_ROUND(R0, R1, R2, R3, 7);
	DEC_ROUND(R2, R3, R0, R1, 6);
	DEC_ROUND(R0, R1, R2, R3, 5);
	DEC_ROUND(R2, R3, R0, R1, 4);
	DEC_ROUND(R0, R1, R2, R3, 3);
	DEC_ROUND(R2, R3, R0, R1, 2);
	DEC_ROUND(R0, R1, R2, R3, 1);
	DEC_ROUND(R2, R3, R0, R1, 0);

	/* load/byteswap/whiten output */
	((u32*)PT)[3] = BSWAP(R1 ^ K[3]);
	((u32*)PT)[2] = BSWAP(R0 ^ K[2]);
	((u32*)PT)[1] = BSWAP(R3 ^ K[1]);
	((u32*)PT)[0] = BSWAP(R2 ^ K[0]);
}

/* the key schedule routine */
void keySched(BYTE M[], int N, u32 **S, u32 K[40], int *k)
{
	u32 Mo[4], Me[4];
	int i, j;
	BYTE vector[8];
	u32 A, B;

	*k = (N + 63) / 64;
	*S = (u32*)malloc(sizeof(u32) * (*k));

	for (i = 0; i < *k; i++)
	{
		Me[i] = BSWAP(((u32*)M)[2*i]);
		Mo[i] = BSWAP(((u32*)M)[2*i+1]);
	}

	for (i = 0; i < *k; i++)
	{
		for (j = 0; j < 4; j++) vector[j] = _b(Me[i], j);
		for (j = 0; j < 4; j++) vector[j+4] = _b(Mo[i], j);
		(*S)[(*k)-i-1] = RSMatrixMultiply(vector);
	}
	for (i = 0; i < 20; i++)
	{
		A = h(2*i*RHO, Me, *k);
		B = ROL(h(2*i*RHO + RHO, Mo, *k), 8);
		K[2*i] = A+B;
		K[2*i+1] = ROL(A + 2*B, 9);
	}
}

/***********************************************************************
  TESTING FUNCTIONS AND STUFF STARTS HERE
***********************************************************************/
void printHex(BYTE b[], int lim)
{
	int i;
	for (i=0; i<lim;i++)
		printf("%02X", (u32)b[i]);
}

std::string getHex(BYTE b[], int lim)
{
	char buf[10];
	std::string strHex;
	int i;
	for (i=0; i<lim;i++) {
		sprintf(buf, "%02X", (u32)b[i]);
		strHex += buf;
	}
	return strHex;
}

/* the ECB tests */
void Itest(int n)
{
	BYTE ct[16], nct[16], k1[16], k2[16], k[32];

	u32 QF[4][256];
	int i;
	u32 *KS;
	u32 K[40];
	int Kk;

	memset(ct, 0, 16);
	memset(nct, 0, 16);
	memset(k1, 0, 16);
	memset(k2, 0, 16);

	for (i=0; i<49; i++)
	{
		memcpy(k, k1, 16);
		memcpy(k+16, k2, 16);

		keySched(k, n, &KS, K, &Kk);
		fullKey(KS, Kk, QF);
		free(KS);
		/*printSubkeys(K);*/
		memcpy(nct, ct, 16);
		encrypt(K, QF, nct);
		printf("\nI=%d\n", i+1);
		printf("KEY=");
		printHex(k, n/8);
		printf("\n");
		printf("PT="); printHex(ct, 16); printf("\n");
		printf("CT="); printHex(nct, 16); printf("\n");
		memcpy(k2, k1, 16);
		memcpy(k1, ct, 16);
		memcpy(ct, nct, 16);
	}
}

#if defined(__linux__) || defined(__APPLE__) || defined(__unix__)
#include <sys/time.h>
#include <unistd.h>
#include <time.h>

double getTimeDiff(struct timeval t1, struct timeval t2)
{
	long us1;
	long us2;
	us1 = t2.tv_sec - t1.tv_sec;
	us2 = t2.tv_usec - t1.tv_usec;
	if (us2 < 0)
	{
		us1--;
		us2 += 1000000;
	}
	return us1 + (us2 / 1000000.0);
}

/* a million encryptions should give us a good feel for how we're doing */
#define NUMTIMES 1000000
void bench()
{
	u32 *S;
	u32 K[40];
	int k;
	int i;
	struct timeval tv_start, tv_end;
	double diff;
	u32 QF[4][256];
	BYTE text[16];
	BYTE key[32];

	memset(text, 0, 16);
	memset(key, 0, 32);
	keySched(key, 128, &S, K, &k);
	fullKey(S, k, QF);
	free(S);

	gettimeofday(&tv_start, NULL);
	for (i=0; i < NUMTIMES; i++)
		encrypt(K, QF, text);
	gettimeofday(&tv_end, NULL);

	diff = getTimeDiff(tv_start, tv_end);
	printf("encs/sec = %f\n", NUMTIMES/diff);
	printf("bytes/sec = %f\n", (NUMTIMES*16)/diff);
	printf("KB/sec = %f\n", NUMTIMES/(diff*64));
	printf("MB/sec = %f\n", NUMTIMES/(diff*65536));
	printf("approx clocks/enc (for 233Mhz) = %f\n", 233333333/(NUMTIMES/diff));
}
#endif

int testTwoFish()
{
	u32 *S;
	u32 K[40];
	int k;
	u32 QF[4][256];
	BYTE text[16];
	BYTE key[32];

	/* a few tests to make sure we didn't break anything */

	/*test encryption of null string with null key*/
	memset(text, 0, 16);
	memset(key, 0, 32);
	keySched(key, 128, &S, K, &k);
	fullKey(S, k, QF);
	free(S);
	puts("before"); printHex(text, 16); printf("\n");
	encrypt(K, QF, text);
	puts("after"); printHex(text, 16); printf("\n");

	/*
	   I=3 encryption from ECB test, again to make sure we didn't
	   break anything
	*/
	memcpy(key,  "\x9F\x58\x9F\x5C\xF6\x12\x2C\x32"
			 "\xB6\xBF\xEC\x2F\x2A\xE8\xC3\x5A", 16);
	memcpy(text, "\xD4\x91\xDB\x16\xE7\xB1\xC3\x9E"
			 "\x86\xCB\x08\x6B\x78\x9F\x54\x19", 16);
	keySched(key, 128, &S, K, &k);
	fullKey(S, k, QF);
	free(S);
	printf("before-->"); printHex(text, 16); printf("\n");
	encrypt(K, QF, text);
	printf("after--->"); printHex(text, 16); printf("\n");
	decrypt(K, QF, text);
	printf("after--->"); printHex(text, 16); printf("\n");

	/*Itest(128);*/

#if defined(__linux__) || defined(__APPLE__) || defined(__unix__)
	bench();
#endif
	return 0;
}

// ----------------------------------------------------------------------------

//
// From zlib zpipe.c example, but modified for stringstream:
//
/* Decompress from file source to file dest until stream ends or EOF.
   inf() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_DATA_ERROR if the deflate data is
   invalid or incomplete, Z_VERSION_ERROR if the version of zlib.h and
   the version of the library linked do not match, or Z_ERRNO if there
   is an error reading or writing the files. */
int inf(std::stringstream &source, std::stringstream &dest)
{
	int ret;
	unsigned have;
	z_stream strm;
	unsigned char in[CHUNK];
	unsigned char out[CHUNK];

	/* allocate inflate state */
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = 0;
	strm.next_in = Z_NULL;
	ret = inflateInit(&strm);
	if (ret != Z_OK)
		return ret;

	/* decompress until deflate stream ends or end of file */
	do {
		strm.avail_in = (z_uInt)source.readsome((char*)in, CHUNK);
//        strm.avail_in = fread(in, 1, CHUNK, source);
//        if (ferror(source)) {
//        if (source.fail()) {
//            (void)inflateEnd(&strm);
//            return Z_ERRNO;
//        }
		if (strm.avail_in == 0) {
			ret = Z_STREAM_END;
			break;
		}
		strm.next_in = in;

		/* run inflate() on input until output buffer not full */
		do {
			strm.avail_out = CHUNK;
			strm.next_out = out;
			ret = inflate(&strm, Z_SYNC_FLUSH);
			assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
			switch (ret) {
				case Z_NEED_DICT:
					ret = Z_DATA_ERROR;     /* and fall through */
					[[fallthrough]];
				case Z_DATA_ERROR:
				case Z_MEM_ERROR:
					(void)inflateEnd(&strm);
					return ret;
			}
			have = CHUNK - strm.avail_out;
			dest.write((char*)out, have);
//            if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
//                (void)inflateEnd(&strm);
//                return Z_ERRNO;
//            }
		} while (strm.avail_out == 0);

	/* done when inflate() says it's done */
	} while (ret != Z_STREAM_END);

	/* clean up and return */
	(void)inflateEnd(&strm);
	return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

// ----------------------------------------------------------------------------

int decompressSQLitePlus(std::stringstream &ssDecrypted, std::string &strPlain)
{
	std::stringstream ssPlain;

	// The first byte seems to be some sort of bitfield or checksum.  Most
	//	have a value of 0x8D or 0x9D, but there's other values too, like
	//	0xBC, etc.  Not sure how it's calculated, so just ignoring it...
	ssDecrypted.ignore(1);

	int nCompressedSize = ssDecrypted.get() + (ssDecrypted.get() << 8);
#if DEBUG_SQLITEPLUS
	std::cerr << "Compressed Size: " << nCompressedSize << std::endl;
#endif

	if ((ssDecrypted.get() != 0x00) || (ssDecrypted.get() != 0x1F)) {
		std::cerr << "*** Decryption Failed -- missing 0x1F00 header\n";
		return Z_DATA_ERROR;
	}
	std::size_t nExpectedDecompressedSize = ssDecrypted.get() + (ssDecrypted.get() << 8) + (ssDecrypted.get() << 16) + (ssDecrypted.get() << 24);
#if DEBUG_SQLITEPLUS
	std::cerr << "Expected Decompressed Size: " << nExpectedDecompressedSize << std::endl;
#endif

	nCompressedSize -= 4;		// Subtract expected decompressed size from total
	std::stringstream ssCompressed;
	while (nCompressedSize--) {
		ssCompressed << (BYTE)ssDecrypted.get();
	}
	nCompressedSize = (int)ssCompressed.str().size();

	int nInfRes = inf(ssCompressed, ssPlain);
	if (nInfRes != Z_OK) {
		std::cerr << "*** Inflate returned: " << nInfRes << ", " << zError(nInfRes) << std::endl;
		return nInfRes;
	}

#if DEBUG_SQLITEPLUS
	std::cerr << "Decompressed Size: " << ssPlain.str().size() << std::endl;
#endif
	if (nExpectedDecompressedSize != ssPlain.str().size()) {
		std::cerr << "*** Decompressed Size Mismatch!\n";
		std::cerr << "Expected Decompressed Size: " << (int)nExpectedDecompressedSize << "\n"
				  << "Actual Decompressed Size: " << (int)ssPlain.str().size() << "\n"
				  << "Compressed Size: " << nCompressedSize << "\n"
				  << "Plain text: \"" << ssPlain.str() << "\"\n";
		strPlain = ssPlain.str();
		return Z_STREAM_END;			// Return Z_STREAM_END (1) if we were short of data
	}

	strPlain = ssPlain.str();

	return Z_OK;
}

// ----------------------------------------------------------------------------

CSQLitePlusDecryptor::CSQLitePlusDecryptor(const std::string &strCryptKey)
{
	uint32_t *S;
	int k;
	uint8_t key[32];

	memset(key, 0, sizeof(key));
	memcpy(key, strCryptKey.c_str(), std::min(strCryptKey.size(), sizeof(key)));
	keySched(key, 128, &S, K, &k);
	fullKey(S, k, QF);
	free(S);
}

int CSQLitePlusDecryptor::decrypt(std::stringstream &ssSource, std::string &strPlain)
{
	BYTE text[16];
	std::stringstream ssDecrypted;

	std::stringstream ssSourceDump;		// For debugging and error tracking
	std::stringstream ssDecryptDump;

	bool bDone = false;
	while (!bDone) {
		memset(text, 0, sizeof(text));
		if (!ssSource.readsome((char *)text, sizeof(text))) bDone = true;
		if (!bDone) {
			ssSourceDump << getHex(text, sizeof(text)) << "\n";
			::decrypt(K, QF, text);
			ssDecryptDump << getHex(text, sizeof(text)) << "\n";
			ssDecrypted.write((char*)text, sizeof(text));
#if DEBUG_SQLITEPLUS
			printHex(text, sizeof(text)); printf("\n");
#endif
		}
	}

	int nRetVal = decompressSQLitePlus(ssDecrypted, strPlain);

	if ((nRetVal != Z_OK) && (nRetVal != Z_STREAM_END)) {
		std::cerr << "\nSource Data:\n";
		std::cerr << ssSourceDump.str() << "\n";
		std::cerr << "Decrypt Data:\n";
		std::cerr << ssDecryptDump.str() << "\n";
	}

	return nRetVal;
}

bool CSQLitePlusDecryptor::test()
{
	std::stringstream ssSource;
	std::string strPlain;

	//59766fe51cd9802e702cd2c80d8c936c
	//c22f06e4e1bfe07c401626ba59ccf67b
	//0ab79206587bd65a8a15584eca9d25bc
	//20edfd9b04af63581cc888aaaaca16da
	//6fbe68adda4bd4a45cfe439406877f9f

	ssSource.write("\x59\x76\x6f\xe5\x1c\xd9\x80\x2e\x70\x2c\xd2\xc8\x0d\x8c\x93\x6c", 16);
	ssSource.write("\xc2\x2f\x06\xe4\xe1\xbf\xe0\x7c\x40\x16\x26\xba\x59\xcc\xf6\x7b", 16);
	ssSource.write("\x0a\xb7\x92\x06\x58\x7b\xd6\x5a\x8a\x15\x58\x4e\xca\x9d\x25\xbc", 16);
	ssSource.write("\x20\xed\xfd\x9b\x04\xaf\x63\x58\x1c\xc8\x88\xaa\xaa\xca\x16\xda", 16);
	ssSource.write("\x6f\xbe\x68\xad\xda\x4b\xd4\xa4\x5c\xfe\x43\x94\x06\x87\x7f\x9f", 16);
	if (decrypt(ssSource, strPlain) != Z_OK) return false;
	std::cout << "\"" << strPlain << "\"" << std::endl;

	// ------------------------------------------------------------------------

	ssSource.str("");
	strPlain.clear();

	//d08296dd8d5224621b39cd24b433f1a1
	//7affd148977da6e346a7500c527720cd
	//684d3815f84e13ad17f8e710edda85ec
	//20669a7461d4c10cc01e8993cb71175f
	//0aed47e1f17f97713883d75514c9e5be
	//8431c28a666d1ccb92a181f38c265101
	//4558d22fa02348b6655dd5290e097de6
	//87244b39af357aab6d043755a81b0efa

	ssSource.write("\xd0\x82\x96\xdd\x8d\x52\x24\x62\x1b\x39\xcd\x24\xb4\x33\xf1\xa1", 16);
	ssSource.write("\x7a\xff\xd1\x48\x97\x7d\xa6\xe3\x46\xa7\x50\x0c\x52\x77\x20\xcd", 16);
	ssSource.write("\x68\x4d\x38\x15\xf8\x4e\x13\xad\x17\xf8\xe7\x10\xed\xda\x85\xec", 16);
	ssSource.write("\x20\x66\x9a\x74\x61\xd4\xc1\x0c\xc0\x1e\x89\x93\xcb\x71\x17\x5f", 16);
	ssSource.write("\x0a\xed\x47\xe1\xf1\x7f\x97\x71\x38\x83\xd7\x55\x14\xc9\xe5\xbe", 16);
	ssSource.write("\x84\x31\xc2\x8a\x66\x6d\x1c\xcb\x92\xa1\x81\xf3\x8c\x26\x51\x01", 16);
	ssSource.write("\x45\x58\xd2\x2f\xa0\x23\x48\xb6\x65\x5d\xd5\x29\x0e\x09\x7d\xe6", 16);
	ssSource.write("\x87\x24\x4b\x39\xaf\x35\x7a\xab\x6d\x04\x37\x55\xa8\x1b\x0e\xfa", 16);
	if (decrypt(ssSource, strPlain) != Z_OK) return false;
	std::cout << "\"" << strPlain << "\"" << std::endl;

	// ------------------------------------------------------------------------

	ssSource.str("");
	strPlain.clear();

	//8239de1b26b175223874c3b94f7eabcd
	//e63b3b4ff2764c7f9a8dd34796309f25
	//fde8e30011470a56c3057e309c8e0ef6
	//2cdcd4408f936da0cd42766b22eac1de

	ssSource.write("\x82\x39\xde\x1b\x26\xb1\x75\x22\x38\x74\xc3\xb9\x4f\x7e\xab\xcd", 16);
	ssSource.write("\xe6\x3b\x3b\x4f\xf2\x76\x4c\x7f\x9a\x8d\xd3\x47\x96\x30\x9f\x25", 16);
	ssSource.write("\xfd\xe8\xe3\x00\x11\x47\x0a\x56\xc3\x05\x7e\x30\x9c\x8e\x0e\xf6", 16);
	ssSource.write("\x2c\xdc\xd4\x40\x8f\x93\x6d\xa0\xcd\x42\x76\x6b\x22\xea\xc1\xde", 16);
	if (decrypt(ssSource, strPlain) != Z_OK) return false;
	std::cout << "\"" << strPlain << "\"" << std::endl;

	// ------------------------------------------------------------------------

	return true;
}

// ============================================================================

