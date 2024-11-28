#ifndef DEF_BIT_H
#define DEF_BIT_H

/* Defines related to bit operations */

#define BIT_0  1
#define BIT_1  2
#define BIT_2  4
#define BIT_3  8
#define BIT_4  16
#define BIT_5  32
#define BIT_6  64
#define BIT_7  128
#define BIT_8  256
#define BIT_9  512
#define BIT_10 1024
#define BIT_11 2048
#define BIT_12 4096
#define BIT_13 8192
#define BIT_14 16384
#define BIT_15 32768
#define BIT_16 65536
#define BIT_17 131072
#define BIT_18 262144
#define BIT_19 524288
#define BIT_20 1048576
#define BIT_21 2097152
#define BIT_22 4194304
#define BIT_23 8388608
#define BIT_24 16777216
#define BIT_25 33554432
#define BIT_26 67108864
#define BIT_27 134217728
#define BIT_28 268435456
#define BIT_29 536870912
#define BIT_30 1073741824
#define BIT_31 2147483648

#define FLAGS_TRANSPARENCY BIT_0
#define FLAGS_LARGE_RLE BIT_1

/* should be removed, just use bitwise operations directly */
#define setBit(bitfield, bit)   (bitfield) |= (bit)
#define clearBit(bitfield, bit) (bitfield) &= ~(bit)
#define isBitSet(bitfield, bit) ((bitfield) & (bit))

#endif /* DEF_BIT_H */
