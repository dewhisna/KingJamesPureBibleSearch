# GF modulus polynomial for MDS matrix
GF_MOD = 2**8 + 2**6 + 2**5 + 2**3 + 1

# GF modulus polynomial for RS code
RS_MOD = 2**8 + 2**6 + 2**3 + 2**2 + 1

ROUNDS=16

import struct
import string

def to32Char(X):
    return list(struct.unpack('>BBBB', struct.pack('>I', X)))

def bytesTo32Bits(l):
    t = 0L
    for i in l:
        t = t << 8
        t = t + i
    return t

def ROR(x, n):
    #assumes 32 bit words
    #print 'n', n
    mask = (2L**n) - 1
    mask_bits = x & mask
    return (x >> n) | (mask_bits << (32 - n))

def ROL(x, n):
    return ROR(x, 32 - n)

def ROR4(x, n): #rotate 4 bit value
    mask = (2**n) - 1
    mask_bits = x & mask
    return (x >> n) | (mask_bits << (4 - n))

def polyMult(a, b):
    t = 0
    while a:
        #print "A=%s  B=%s  T=%s" % (hex(a), hex(b), hex(t))
        if a & 1:
            t = t ^ b
        b = b << 1
        a = a >> 1
    return t

def gfMod(t, modulus):
    modulus = modulus << 7
    for i in range(8):
        tt = t ^ modulus
        if tt < t:
            t = tt
        modulus = modulus >> 1
    return t

def gfMult(a, b, modulus):
    return gfMod(polyMult(a, b), modulus)

def matrixMultiply(md, sd, modulus):
    #uses GF(2**8)
    #can cheese since second arg is 1 dimensional
    r = []
    i = 0
    for j in range(len(md)):
        t = 0L
        for k in range(len(sd)):
            #print "t=%X" % t
            t = t ^ gfMult(md[j][k], sd[k], modulus)
        r.insert(0, t)
    return r

MDS = [
    [ 0x01, 0xEF, 0x5B, 0x5B ],
    [ 0x5B, 0xEF, 0xEF, 0x01 ],
    [ 0xEF, 0x5B, 0x01, 0xEF ],
    [ 0xEF, 0x01, 0xEF, 0x5B ],
    ]

RS = [
    [ 0x01, 0xA4, 0x55, 0x87, 0x5A, 0x58, 0xDB, 0x9E ], 
    [ 0xA4, 0x56, 0x82, 0xF3, 0x1E, 0xC6, 0x68, 0xE5 ], 
    [ 0x02, 0xA1, 0xFC, 0xC1, 0x47, 0xAE, 0x3D, 0x19 ], 
    [ 0xA4, 0x55, 0x87, 0x5A, 0x58, 0xDB, 0x9E, 0x03 ],
    ]

Q0 = [
    [ 0x8,0x1,0x7,0xD, 0x6,0xF,0x3,0x2, 0x0,0xB,0x5,0x9, 0xE,0xC,0xA,0x4 ], 
    [ 0xE,0xC,0xB,0x8, 0x1,0x2,0x3,0x5, 0xF,0x4,0xA,0x6, 0x7,0x0,0x9,0xD ], 
    [ 0xB,0xA,0x5,0xE, 0x6,0xD,0x9,0x0, 0xC,0x8,0xF,0x3, 0x2,0x4,0x7,0x1 ], 
    [ 0xD,0x7,0xF,0x4, 0x1,0x2,0x6,0xE, 0x9,0xB,0x3,0x0, 0x8,0x5,0xC,0xA ], 
    ]

Q1 = [
    [ 0x2,0x8,0xB,0xD, 0xF,0x7,0x6,0xE, 0x3,0x1,0x9,0x4, 0x0,0xA,0xC,0x5 ], 
    [ 0x1,0xE,0x2,0xB, 0x4,0xC,0x3,0x7, 0x6,0xD,0xA,0x5, 0xF,0x9,0x0,0x8 ], 
    [ 0x4,0xC,0x7,0x5, 0x1,0x6,0x9,0xA, 0x0,0xE,0xD,0x8, 0x2,0xB,0x3,0xF ], 
    [ 0xB,0x9,0x5,0x1, 0xC,0x3,0xD,0xE, 0x6,0x4,0x7,0xF, 0x2,0x0,0x8,0xA ], 
    ]

def printRoundKeys(K):
    for i in range(0, len(K), 2):
        print '%8s %8s' % (hex(K[i])[2:-1], hex(K[i+1])[2:-1])
        
def keySched(M, N): #M is key text in 32 bit words, N is bit width of M
    k = (N+63)/64

    Me = map(lambda x, M=M: M[x], range(0, (2*k-1), 2))
    Mo = map(lambda x, M=M: M[x], range(1, (2*k), 2))
    #print 'me=', map(hex, Me)
    #print 'mo=', map(hex, Mo)
    #print 'k=', k, len(M)
    
    S = []
    for i in range(0, k):
        x1 = to32Char(Me[i])#; x1.reverse()
        x2 = to32Char(Mo[i])#; x2.reverse()
        vector = x1 + x2 #x2 + x1
        #vector.reverse()
        #print "vector=", map(hex, vector)
        #print 'vector=', map(hex, vector)
        prod = matrixMultiply(RS, vector, RS_MOD)
        #print 'MM=%s' % map(hex, prod)
        prod.reverse()
        S.insert(0, bytesTo32Bits(prod))
        #print
    
    #reverse S
    #S.reverse()
    #print 'S=', map(hex, S)
    K = makeKey(Me, Mo, k)

    #printRoundKeys(K)

    return K, k, S

def makeKey(Me, Mo, k):
    K = []
    rho = 0x01010101L
    #print "LOOP is:", ROUNDS+4
    for i in range(ROUNDS + 4):
        A = h(2*i*rho, Me, k)
        B = h((2*i+1)*rho, Mo, k)
        #print 'A=', hex(A)
        #print 'B=', hex(B)
        #print

        B = ROL(B, 8)
        K.append( (A+B) & 0xFFFFFFFFL )
        K.append(ROL((A + 2*B) & 0xFFFFFFFFL, 9))
    return K

def Qpermute(x, Q):
    x=int(x)
    a0, b0 = x/16, x % 16
    a1 = a0 ^ b0
    b1 = (   a0 ^ ROR4(b0, 1) ^ (8*a0)  ) % 16
    a2, b2 = Q[0][a1], Q[1][b1]
    a3 = a2 ^ b2
    b3 = (   a2 ^ ROR4(b2, 1) ^ (8*a2)  ) % 16
    a4, b4 = Q[2][a3], Q[3][b3]

    return (16 * b4) + a4


def h(X, L, k):
    #X is 32 bit word
    #L is list (L[0] - L[K-1]) of 32 bit words  (Sbox keys)
    x = []
    l = []

    x = to32Char(X)
    x.reverse()
    l = map(to32Char, L)
    y = x[:]

    Qdones = [
        [Q1, Q0, Q1, Q0],
        [Q0, Q0, Q1, Q1],
        [Q0, Q1, Q0, Q1],
        [Q1, Q1, Q0, Q0],
        [Q1, Q0, Q0, Q1],
    ]

    #print 'pre Q', y, k, range(k-1, -1, -1)
    for i in range(k-1, -1, -1):
        for j in range(4):
            y[j] = Qpermute(y[j], Qdones[i+1][j]) ^ l[i][j]
            
    for j in range(4):
        y[j] = Qpermute(y[j], Qdones[0][j])
    #print 'PreMatrix=', y

    z = matrixMultiply(MDS, y, GF_MOD)
    #print 'PostMatrix= 0x%s' % string.join(map(lambda x:(x<16 and "0" or "") + hex(x)[2:-1], z), '')
    return bytesTo32Bits(z)
            
def g(X, S, k):
    return h(X, S, k)

def F(R0, R1, r, K, k, S):
    T0 = g(R0, S, k)
    T1 = g(ROL(R1, 8), S, k)
    F0 = (T0 + T1 + K[2*r+8]) & 0xFFFFFFFFL
    F1 = (T0 + 2*T1 + K[2*r+9]) & 0xFFFFFFFFL
    #print 'T0=', hex(T0)
    #print 'T1=', hex(T1)
    #print 'F0=', hex(F0)
    #print 'F1=', hex(F1)
    return F0, F1

def encrypt(K, k, S, PT): #pt is array of 4 32bit L's
    #BSWAP PT
    PT = map(lambda x:struct.unpack('>I', struct.pack('<I', x))[0], PT)
    #print 'round[-1]', map(hex, PT)
    #input whiten
    R = map(lambda i, PT=PT, K=K: PT[i] ^ K[i], range(4))

    #print 'round[0]', map(hex, R)
    for r in range(ROUNDS):
        NR = [0,0,0,0]
        FR0, FR1 = F(R[0], R[1], r, K, k, S)
        #print "ROL(R3, 1)", hex(ROL(R[3], 1)), hex(R[3])
        NR[2] = ROR(R[2] ^ FR0, 1)
        NR[3] = ROL(R[3], 1) ^ FR1
        NR[0] = R[0]
        NR[1] = R[1]
        R = NR
        if (r < ROUNDS - 1): #/* swap for next round */
            R[0], R[2] = R[2], R[0]
            R[1], R[3] = R[3], R[1]
        #print 'round[%s]' % (r+1), map(hex, R)#, hex(K[2*r+8]), hex(K[2*r+9])

    R = [R[2], R[3], R[0], R[1]]
    #print map(hex, K[4:9])
    R = map(lambda i, R=R, K=K: R[(i+2) % 4] ^ K[i+4], range(4))
    #BSWAP R
    R = map(lambda x:struct.unpack('>I', struct.pack('<I', x))[0], R)
    #print 'round[17]', map(hex, R)
    return R

def decrypt(K, k, S, PT): #pt is array of 4 32bit L's
    #BSWAP PT
    PT = map(lambda x:struct.unpack('>I', struct.pack('<I', x))[0], PT)
    #PT.reverse()
    #print 'round[-1]', map(hex, PT)
    #input whiten
    R = map(lambda i, PT=PT, K=K: PT[i] ^ K[i+4], range(4))

    #print 'round[17]', map(hex, R)
    for r in range(ROUNDS-1, -1, -1):
        NR = [0,0,0,0]
        FR0, FR1 = F(R[0], R[1], r, K, k, S)
        NR[2] = ROL(R[2], 1) ^ FR0
        NR[3] = ROR(R[3] ^ FR1, 1)
        NR[0] = R[0]
        NR[1] = R[1]
        R = NR
        if (r > 0): #/* swap for next round */
            R[0], R[2] = R[2], R[0]
            R[1], R[3] = R[3], R[1]
        print 'round[%s]' % (r+1), map(hex, R)

    R = [R[2], R[3], R[0], R[1]]
    R = map(lambda i, R=R, K=K: R[(i+2) % 4] ^ K[i], range(4))
    #BSWAP R
    R = map(lambda x:struct.unpack('>I', struct.pack('<I', x))[0], R)
    #print 'round[-1]', map(hex, R)
    return R


def testKey(K, k, S):
    print 'subkeys'
    printRoundKeys(K)
    #for i in range(0, len(k.subKeys), 2):
    #    print hex(k.subKeys[i]), hex(k.subKeys[i+1])
    
    ct = encrypt(K, k, S, [0L, 0L, 0L, 0L])
    print 'CT=', map(hex, ct)
    pt = decrypt(K, k, S, ct)
    
    print 'PT=', map(hex, pt)
    print ; print ; print

def dispLongList(v):
    return string.join(map(lambda x:string.zfill(hex(x)[2:-1], 8), v), '')

def Itest128():
    ct = [0L, 0L, 0L, 0L]
    k = [0L, 0L, 0L, 0L]

    for i in range(49):
        K, Kk, KS = keySched(k, 128)
        #printRoundKeys(K)
        nct = encrypt(K, Kk, KS, ct)
        print
        print 'I=%d' % (i+1)
        print 'KEY=%s' % dispLongList(k)
        print 'PT=%s' % dispLongList(ct)
        print 'CT=%s' % dispLongList(nct)
        k = ct
        ct = nct
        
def Itest256():
    ct = [0L, 0L, 0L, 0L]
    k1 = [0L, 0L, 0L, 0L]
    k2 = [0L, 0L, 0L, 0L]
    
    for i in range(49):
        K, Kk, KS = keySched(k1 + k2, 256)
        nct = encrypt(K, Kk, KS, ct)
        print
        print 'I=%d' % (i+1)
        print 'KEY=%s' % (dispLongList(k1)+dispLongList(k2))
        print 'PT=%s' % dispLongList(ct)
        print 'CT=%s' % dispLongList(nct)

        k2 = k1
        k1 = ct
        ct = nct

def Itest192():
    ct = [0L, 0L, 0L, 0L]
    k1 = [0L, 0L, 0L, 0L]
    k2 = [0L, 0L, 0L, 0L]
    
    for i in range(49):
        K, Kk, KS = keySched(k1 + k2, 192)
        nct = encrypt(K, Kk, KS, ct)
        print
        print 'I=%d' % (i+1)
        print 'KEY=%s' % (dispLongList(k1)+dispLongList(k2[:2]))
        print 'PT=%s' % dispLongList(ct)
        print 'CT=%s' % dispLongList(nct)

        k2 = k1
        k1 = ct
        ct = nct

def bench():
    import time
    ENCS = 50
    k = [0L, 0L, 0L, 0L]
    pt = [0L, 0L, 0L, 0L]
    K, Kk, KS = keySched(k, 128)
    a = range(ENCS)
    b = time.time()
    for i in a:
        encrypt(K, Kk, KS, pt)
    e = time.time()
    print 'time for %d encryptions: %f' % (ENCS, e-b)
    print 'e/s:', (ENCS / (e-b))
    print 'b/s:', (16 * (ENCS / (e-b)))

    b = time.time()
    for i in a:
        keySched(k, 128)
    e = time.time()
    print 'time for %d key setups: %f' % (ENCS, e-b)
    print 'ks/s:', (ENCS / (e-b))

if __name__ == '__main__':
    Itest256()
    #print hex(polyMult(0x5b, 3))
    #Itest192()
    #Itest256()
    
    #K, k, S = keySched([0l, 0l, 0l, 0l], 128)
    #testKey(K, k, S)
    #K, k, S = keySched([0x01234567L, 0x89abcdefL, 0xfedcba98l, 0x76543210L,
    #                    0x00112233L, 0x44556677L], 192)
    #testKey(K, k, S)

    #K, k, S = keySched([0x01234567L, 0x89abcdefL, 0xfedcba98l, 0x76543210L,
    #                    0x00112233L, 0x44556677L, 0x8899aabbl, 0xccddeeffl],
    #                   256)
    #testKey(K, k, S)

    #K, k, S = keySched([0xefcdab89l, 0x67452301l,
    #                    0x10325476l, 0x98badcfel,
    #                    0x77665544l, 0x33221100], 192)
    #PT= [0xDEADBEAFL, 0xCAFEBABEL, 0x86753090L, 0x04554013L]
    #CT = encrypt(K, k, S, PT)
    #print dispLongList(CT)
    #PT = decrypt(K, k, S, CT)
    #print dispLongList(PT)
    #bench()
    K,k,S = keySched([0x9F589F5C, 0xF6122C32, 0xB6BFEC2F, 0x2AE8C35A], 128)
    CT = encrypt(K, k, S,
                 [ 0xD491DB16L, 0xE7B1C39EL, 0x86CB086BL, 0x789F5419L ])
    print dispLongList(CT)


