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
#include "tables.h"
#define u32 unsigned int
#define BYTE unsigned char
#define RS_MOD 0x14D
#define RHO 0x01010101L

unsigned int here(unsigned int x)
{
    unsigned int mask=~0U;
    return (* (((unsigned int *)&x)-1)) & mask;
}

typedef void (*encryptor)(u32 S[4][256], BYTE PT[16]);

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
	case 3:
	    y0 = Q1[y0] ^ b0(L[2]);
	    y1 = Q1[y1] ^ b1(L[2]);
	    y2 = Q0[y2] ^ b2(L[2]);
	    y3 = Q0[y3] ^ b3(L[2]);
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
    	    case 3:
    		y0 = Q1[y0] ^ b0(L[2]);
    		y1 = Q1[y1] ^ b1(L[2]);
    		y2 = Q0[y2] ^ b2(L[2]);
    		y3 = Q0[y3] ^ b3(L[2]);
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
    printf("round[%d] ['0x%08XL', '0x%08XL', '0x%08XL', '0x%08XL']\n", 
	   round, R0, R1, R2, R3);

}

/* fully keyed h (aka g) function */
#define fkh(X) (S[0][b0(X)]^S[1][b1(X)]^S[2][b2(X)]^S[3][b3(X)])

/* one encryption round */
#define ENC_ROUND(R0, R1, R2, R3, round) \
    T0 = fkh(R0); \
    T1 = fkh(ROL(R1, 8)); \
    R2 = ROR(R2 ^ (T1 + T0 + 0xDEADBEAF), 1); \
    R3 = ROL(R3, 1) ^ (2*T1 + T0 + 0xDEADBEAF); 

void encryptionCodeStart() { }


inline void encryptBase(u32 S[4][256], BYTE PT[16])
{
    u32 R0, R1, R2, R3;
    u32 T0, T1;

    /* load/byteswap/whiten input */
    R3 = 0xDEADBEAF ^ BSWAP(((u32*)PT)[3]);
    R2 = 0xDEADBEAF ^ BSWAP(((u32*)PT)[2]);
    R1 = 0xDEADBEAF ^ BSWAP(((u32*)PT)[1]);
    R0 = 0xDEADBEAF ^ BSWAP(((u32*)PT)[0]);

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
    ((u32*)PT)[3] = BSWAP(R1 ^ 0xDEADBEAF);
    ((u32*)PT)[2] = BSWAP(R0 ^ 0xDEADBEAF);
    ((u32*)PT)[1] = BSWAP(R3 ^ 0xDEADBEAF);
    ((u32*)PT)[0] = BSWAP(R2 ^ 0xDEADBEAF);
}

void encryptionCodeEnd () {}
unsigned int encryptionCodeSize() { encryptionCodeStart(); encryptionCodeEnd(); return encryptionCodeEnd - encryptionCodeStart; }

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

#include <sys/mman.h>
encryptor makeEncryptor(u32 K[40])
{
    unsigned int size;
    unsigned char *code;
    unsigned char *p;
    int i = 0;
    int KInds[40] = {
	3,2,1,0,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,
	27,28,29,30,31,32,33,34,35,36,37,38,39,7,6,5,4};

    size = 10000; /*encryptionCodeSize();*/
    size = encryptionCodeSize();
    printf("size = %d\n", size);
    printf("ecs = %d\n", encryptionCodeSize());
    code = (unsigned char*)malloc(size);
    memcpy(code, encryptBase, size);
    mprotect(code, size, PROT_EXEC | PROT_READ);
    
    for (p=code; p < code+size; p++)
    {
	if (*((int *)p) == 0xDEADBEAF)
	    *((int *)p) = K[KInds[i++]];
    }
    printf("replaced %d DEADBEAFs\n", i);
    return (encryptor)code;
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


/* the ECB tests */
void Itest(int n)
{
    BYTE ct[16], nct[16], k1[16], k2[16], k[32];

    u32 QF[4][256];
    int i;
    u32 *KS;
    u32 K[40];
    int Kk;
    encryptor encrypt;

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
	encrypt = makeEncryptor(K);
	/*printSubkeys(K);*/
	memcpy(nct, ct, 16);
	encrypt(QF, nct);
	/*        encrypt(K, QF, nct);   FIXME--ATC*/ 
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
    encryptor encrypt;

    memset(text, 0, 16);
    memset(key, 0, 32);
    keySched(key, 128, &S, K, &k);
    fullKey(S, k, QF);
    free(S);
    encrypt = makeEncryptor(K);

    gettimeofday(&tv_start, NULL);
    for (i=0; i < NUMTIMES; i++)
	encrypt(QF, text);  
    gettimeofday(&tv_end, NULL);

    diff = getTimeDiff(tv_start, tv_end);
    printf("encs/sec = %f\n", NUMTIMES/diff);
    printf("bytes/sec = %f\n", (NUMTIMES*16)/diff);
    printf("KB/sec = %f\n", NUMTIMES/(diff*64));
    printf("MB/sec = %f\n", NUMTIMES/(diff*65536));
    printf("approx clocks/enc (for 233Mhz) = %f\n", 233333333/(NUMTIMES/diff));
}


int main()
{
    u32 *S;
    u32 K[40];
    int k;
    u32 QF[4][256];
    BYTE text[16];
    BYTE key[32];

    encryptor encrypt;

    /* a few tests to make sure we didn't break anything */
#if 1
    /*test encryption of null string with null key*/
    memset(text, 0, 16);
    memset(key, 0, 32);
    keySched(key, 128, &S, K, &k);
    fullKey(S, k, QF);
    printf("before make encryptor\n");
    encrypt = makeEncryptor(K);
    printf("after make encryptor\n");
    fflush(stdout);

    free(S);
    puts("before"); printHex(text, 16); printf("\n");
    /*encrypt(K, QF, text);*/
    encrypt(QF, text);
    free(encrypt);
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
    encrypt = makeEncryptor(K);
    free(S);
    printf("before-->"); printHex(text, 16); printf("\n");
    /*encrypt(K, QF, text);*/
    encrypt(QF, text);
    free(encrypt);
    printf("after--->"); printHex(text, 16); printf("\n");
    decrypt(K, QF, text);
    printf("after--->"); printHex(text, 16); printf("\n");
#endif
    /*Itest(128);*/

    bench();
    return 0;
}
