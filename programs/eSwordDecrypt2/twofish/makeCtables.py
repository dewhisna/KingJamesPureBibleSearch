import myref
print '#define u8 unsigned char'
print 'u8 RS[4][8] = {'
for i in myref.RS:
    print '    {',
    for j in i:
        print "0x%02X," % j,
    print '},'
print '};'
print

print 'u8 Q0[] = {'
print '   ',
for i in range(256):
    print "0x%02X," % myref.Qpermute(i, myref.Q0),
    if not ((i+1) % 8):
        print '\n   ',
print '};'
print

print 'u8 Q1[] = {'
print '   ',
for i in range(256):
    print "0x%02X," % myref.Qpermute(i, myref.Q1),
    if not ((i+1) % 8):
        print '\n   ',
print '};'
print

print 'u8 mult5B[] = {'
print '   ',
for i in range(256):
    print "0x%02X," % myref.gfMult(0x5B, i, myref.GF_MOD),
    if not ((i+1) % 8):
        print '\n   ',
print '};'
print

print 'u8 multEF[] = {'
print '   ',
for i in range(256):
    print "0x%02X," % myref.gfMult(0xEF, i, myref.GF_MOD),
    if not ((i+1) % 8):
        print '\n   ',
print '};'
print

#rho = 0x01010101L
#print 'KeyConsts = ['
#for i in range(20):
#    print '    [ 0x%08XL, 0x%08XL ],' % (2*i*rho, 2*i*rho + rho)
#print ']'
#print
