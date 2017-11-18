#include <ctype.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "acmedisass.h"

enum {
    NONE,
    ACC,
    IMP,
    IMM,
    ZP,
    ZPX,
    ZPY,
    ABS,
    ABSX,
    ABSY,
    ABSI,
    INDX,
    INDY,
    REL
};

enum {
    MODE6502,
    MODE6510
};

opcode opcodes[] = {
    [0x69]{ "adc", 2, 2, IMM },
    [0x65]{ "adc", 2, 3, ZP },
    [0x75]{ "adc", 2, 4, ZPX },
    [0x6D]{ "adc", 3, 4, ABS },
    [0x7D]{ "adc", 3, 4, ABSX },
    [0x79]{ "adc", 3, 4, ABSY },
    [0x61]{ "adc", 2, 6, INDX },
    [0x71]{ "adc", 2, 5, INDY },

    [0x0B]{ "anc", 2, 2, IMM },
    [0x2B]{ "anc", 2, 2, IMM },

    [0x29]{ "and", 2, 2, IMM },
    [0x25]{ "and", 2, 3, ZP },
    [0x35]{ "and", 2, 4, ZPX },
    [0x2D]{ "and", 3, 4, ABS },
    [0x3D]{ "and", 3, 4, ABSX },
    [0x39]{ "and", 3, 4, ABSY },
    [0x21]{ "and", 2, 6, INDX },
    [0x31]{ "and", 2, 5, INDY },

    [0x8B]{ "ane", 2, 2, IMM },

    [0x6B]{ "arr", 2, 2, IMM },

    [0x4B]{ "asr", 2, 2, IMM },

    [0x0A]{ "asl", 1, 2, ACC },
    [0x06]{ "asl", 2, 5, ZP },
    [0x16]{ "asl", 2, 6, ZPX },
    [0x0E]{ "asl", 3, 6, ABS },
    [0x1E]{ "asl", 3, 7, ABSX },

    [0x90]{ "bcc", 2, 2, REL },

    [0xB0]{ "bcs", 2, 2, REL },

    [0xF0]{ "beq", 2, 2, REL },

    [0x24]{ "bit", 2, 3, ZP },
    [0x2C]{ "bit", 3, 4, ABS },

    [0x30]{ "bmi", 2, 2, REL },

    [0xD0]{ "bne", 2, 2, REL },

    [0x10]{ "bpl", 2, 2, REL },

    [0x00]{ "brk", 1, 7, IMP },

    [0x50]{ "bvc", 2, 2, REL },

    [0x70]{ "bvs", 2, 2, REL },

    [0x18]{ "clc", 1, 2, IMP },

    [0xD8]{ "cld", 1, 2, IMP },

    [0x58]{ "cli", 1, 2, IMP },

    [0xB8]{ "clv", 1, 2, IMP },

    [0xC9]{ "cmp", 2, 2, IMM },
    [0xC5]{ "cmp", 2, 3, ZP },
    [0xD5]{ "cmp", 2, 4, ZPX },
    [0xCD]{ "cmp", 3, 4, ABS },
    [0xDD]{ "cmp", 3, 4, ABSX },
    [0xD9]{ "cmp", 3, 4, ABSY },
    [0xC1]{ "cmp", 2, 6, INDX },
    [0xD1]{ "cmp", 2, 5, INDY },

    [0xE0]{ "cpx", 2, 2, IMM },
    [0xE4]{ "cpx", 2, 3, ZP },
    [0xEC]{ "cpx", 3, 4, ABS },

    [0xC0]{ "cpy", 2, 2, IMM },
    [0xC4]{ "cpy", 2, 3, ZP },
    [0xCC]{ "cpy", 3, 4, ABS },

    [0xC7]{ "dcp", 2, 5, ZP },
    [0xD7]{ "dcp", 2, 6, ZPX },
    [0xCF]{ "dcp", 3, 6, ABS },
    [0xDF]{ "dcp", 3, 7, ABSX },
    [0xDB]{ "dcp", 3, 7, ABSY },
    [0xC3]{ "dcp", 2, 8, INDX },
    [0xD3]{ "dcp", 2, 8, INDY },

    [0xC6]{ "dec", 2, 5, ZP },
    [0xD6]{ "dec", 2, 6, ZPX },
    [0xCE]{ "dec", 3, 6, ABS },
    [0xDE]{ "dec", 3, 7, ABSX },

    [0xCA]{ "dex", 1, 2, IMP },

    [0x88]{ "dey", 1, 2, IMP },

    [0x49]{ "eor", 2, 2, IMM },
    [0x45]{ "eor", 2, 3, ZP },
    [0x55]{ "eor", 2, 4, ZPX },
    [0x4D]{ "eor", 3, 4, ABS },
    [0x5D]{ "eor", 3, 4, ABSX },
    [0x59]{ "eor", 3, 4, ABSY },
    [0x41]{ "eor", 2, 6, INDX },
    [0x51]{ "eor", 2, 5, INDY },

    [0xE6]{ "inc", 2, 5, ZP },
    [0xF6]{ "inc", 2, 6, ZPX },
    [0xEE]{ "inc", 3, 6, ABS },
    [0xFE]{ "inc", 3, 7, ABSX },

    [0xE8]{ "inx", 1, 2, IMP },

    [0xC8]{ "iny", 1, 2, IMP },

    [0xE7]{ "isb", 2, 5, ZP },
    [0xF7]{ "isb", 2, 6, ZPX },
    [0xEF]{ "isb", 3, 6, ABS },
    [0xFF]{ "isb", 3, 7, ABSX },
    [0xFB]{ "isb", 3, 7, ABSY },
    [0xE3]{ "isb", 2, 8, INDX },
    [0xF3]{ "isb", 2, 8, INDY },

    [0x02]{ "jam", 1, 0, IMP },
    [0x12]{ "jam", 1, 0, IMP },
    [0x22]{ "jam", 1, 0, IMP },
    [0x32]{ "jam", 1, 0, IMP },
    [0x42]{ "jam", 1, 0, IMP },
    [0x52]{ "jam", 1, 0, IMP },
    [0x62]{ "jam", 1, 0, IMP },
    [0x72]{ "jam", 1, 0, IMP },
    [0x92]{ "jam", 1, 0, IMP },
    [0xB2]{ "jam", 1, 0, IMP },
    [0xD2]{ "jam", 1, 0, IMP },
    [0xF2]{ "jam", 1, 0, IMP },

    [0x4C]{ "jmp", 3, 3, ABS },
    [0x6C]{ "jmp", 3, 5, ABSI },

    [0x20]{ "jsr", 3, 6, ABS },

    [0xBB]{ "lae", 3, 4, ABSY },

    [0xA7]{ "lax", 2, 3, ZP },
    [0xB7]{ "lax", 2, 4, ZPY },
    [0xAF]{ "lax", 3, 4, ABS },
    [0xBF]{ "lax", 3, 4, ABSY },
    [0xA3]{ "lax", 2, 6, INDX },
    [0xB3]{ "lax", 2, 5, ABS },

    [0xA9]{ "lda", 2, 2, IMM },
    [0xA5]{ "lda", 2, 3, ZP },
    [0xB5]{ "lda", 2, 4, ZPX },
    [0xAD]{ "lda", 3, 4, ABS },
    [0xBD]{ "lda", 3, 4, ABSX },
    [0xB9]{ "lda", 3, 4, ABSY },
    [0xA1]{ "lda", 2, 6, INDX },
    [0xB1]{ "lda", 2, 5, INDY },

    [0xA2]{ "ldx", 2, 2, IMM },
    [0xA6]{ "ldx", 2, 3, ZP },
    [0xB6]{ "ldx", 2, 4, ZPY },
    [0xAE]{ "ldx", 3, 4, ABS },
    [0xBE]{ "ldx", 3, 4, ABSY },

    [0xA0]{ "ldy", 2, 2, IMM },
    [0xA4]{ "ldy", 2, 3, ZP },
    [0xB4]{ "ldy", 2, 4, ZPX },
    [0xAC]{ "ldy", 3, 4, ABS },
    [0xBC]{ "ldy", 3, 4, ABSX },

    [0x4A]{ "lsr", 1, 2, ACC },
    [0x46]{ "lsr", 2, 5, ZP },
    [0x56]{ "lsr", 2, 6, ZPX },
    [0x4E]{ "lsr", 3, 6, ABS },
    [0x5E]{ "lsr", 3, 7, ABSX },

    [0xAB]{ "lxa", 2, 2, IMM },

    [0xEA]{ "nop", 1, 2, IMP },
    [0x1A]{ "nop", 1, 2, IMP },
    [0x3A]{ "nop", 1, 2, IMP },
    [0x5A]{ "nop", 1, 2, IMP },
    [0x7A]{ "nop", 1, 2, IMP },
    [0xDA]{ "nop", 1, 2, IMP },
    [0xFA]{ "nop", 1, 2, IMP },
    [0x80]{ "nop", 2, 2, IMM },
    [0x82]{ "nop", 2, 2, IMM },
    [0x89]{ "nop", 2, 2, IMM },
    [0xC2]{ "nop", 2, 2, IMM },
    [0xE2]{ "nop", 2, 2, IMM },
    [0x04]{ "nop", 2, 3, ZP },
    [0x44]{ "nop", 2, 3, ZP },
    [0x64]{ "nop", 2, 3, ZP },
    [0x14]{ "nop", 2, 4, ZPX },
    [0x34]{ "nop", 2, 4, ZPX },
    [0x54]{ "nop", 2, 4, ZPX },
    [0x74]{ "nop", 2, 4, ZPX },
    [0xD4]{ "nop", 2, 4, ZPX },
    [0xF4]{ "nop", 2, 4, ZPX },
    [0x0C]{ "nop", 3, 4, ABS },
    [0x1C]{ "nop", 3, 4, ABSX },
    [0x3C]{ "nop", 3, 4, ABSX },
    [0x5C]{ "nop", 3, 4, ABSX },
    [0x7C]{ "nop", 3, 4, ABSX },
    [0xDC]{ "nop", 3, 4, ABSX },
    [0xFC]{ "nop", 3, 4, ABSX },

    [0x09]{ "ora", 2, 2, IMM },
    [0x05]{ "ora", 2, 3, ZP },
    [0x15]{ "ora", 2, 4, ZPX },
    [0x0D]{ "ora", 2, 4, ABS },
    [0x1D]{ "ora", 2, 4, ABSX },
    [0x19]{ "ora", 2, 4, ABSY },
    [0x01]{ "ora", 2, 6, INDX },
    [0x11]{ "ora", 2, 5, INDY },

    [0x48]{ "pha", 1, 3, IMP },

    [0x08]{ "php", 1, 3, IMP },

    [0x68]{ "pla", 1, 4, IMP },

    [0x28]{ "plp", 1, 4, IMP },

    [0x27]{ "rla", 2, 5, ZP },
    [0x37]{ "rla", 2, 6, ZPX },
    [0x2F]{ "rla", 3, 6, ABS },
    [0x3F]{ "rla", 3, 7, ABSX },
    [0x3B]{ "rla", 3, 7, ABSY },
    [0x23]{ "rla", 2, 8, INDX },
    [0x33]{ "rla", 2, 8, INDY },

    [0x2A]{ "rol", 1, 2, ACC },
    [0x26]{ "rol", 2, 5, ZP },
    [0x36]{ "rol", 2, 6, ZPX },
    [0x2E]{ "rol", 3, 6, ABS },
    [0x3E]{ "rol", 3, 7, ABSX },

    [0x6A]{ "ror", 1, 2, ACC },
    [0x66]{ "ror", 2, 5, ZP },
    [0x76]{ "ror", 2, 6, ZPX },
    [0x6E]{ "ror", 3, 6, ABS },
    [0x7E]{ "ror", 3, 7, ABSX },

    [0x67]{ "rra", 2, 5, ZP },
    [0x77]{ "rra", 2, 6, ZPX },
    [0x6F]{ "rra", 3, 6, ABS },
    [0x7F]{ "rra", 3, 7, ABSX },
    [0x7B]{ "rra", 3, 7, ABSY },
    [0x63]{ "rra", 2, 8, INDX },
    [0x73]{ "rra", 2, 8, INDY },

    [0x40]{ "rti", 1, 6, IMP },

    [0x60]{ "rts", 1, 6, IMP },

    [0x87]{ "sax", 2, 3, ZP },
    [0x97]{ "sax", 2, 4, ZPY },
    [0x8F]{ "sax", 3, 4, ABS },
    [0x83]{ "sax", 2, 6, INDX },

    [0xE9]{ "sbc", 2, 2, IMM },
    [0xE5]{ "sbc", 2, 3, ZP },
    [0xF5]{ "sbc", 2, 4, ZPX },
    [0xED]{ "sbc", 3, 4, ABS },
    [0xFD]{ "sbc", 3, 4, ABSX },
    [0xF9]{ "sbc", 3, 4, ABSY },
    [0xE1]{ "sbc", 2, 6, INDX },
    [0xF1]{ "sbc", 2, 5, INDY },

    [0xEB]{ "sbc", 2, 2, IMM },

    [0xCB]{ "sbx", 2, 2, IMM },

    [0x38]{ "sec", 1, 2, IMP },

    [0xF8]{ "sed", 1, 2, IMP },

    [0x78]{ "sei", 1, 2, IMP },

    [0x93]{ "sha", 3, 5, ABSX },
    [0x9F]{ "sha", 3, 5, ABSY },

    [0x9B]{ "shs", 3, 5, ABSY },

    [0x9E]{ "shx", 3, 5, ABSY },

    [0x9C]{ "shy", 3, 5, ABSX },

    [0x07]{ "slo", 2, 5, ZP },
    [0x17]{ "slo", 2, 6, ZPX },
    [0x0F]{ "slo", 3, 6, ABS },
    [0x1F]{ "slo", 3, 7, ABSX },
    [0x1B]{ "slo", 3, 7, ABSY },
    [0x03]{ "slo", 2, 8, INDX },
    [0x13]{ "slo", 2, 8, INDY },

    [0x47]{ "sre", 2, 5, ZP },
    [0x57]{ "sre", 2, 6, ZPX },
    [0x4F]{ "sre", 3, 6, ABS },
    [0x5F]{ "sre", 3, 7, ABSX },
    [0x5B]{ "sre", 3, 7, ABSY },
    [0x43]{ "sre", 2, 8, INDX },
    [0x53]{ "sre", 2, 8, INDY },

    [0x85]{ "sta", 2, 3, ZP },
    [0x95]{ "sta", 2, 4, ZPX },
    [0x8D]{ "sta", 3, 4, ABS },
    [0x9D]{ "sta", 3, 4, ABSX },
    [0x99]{ "sta", 3, 4, ABSY },
    [0x81]{ "sta", 2, 6, INDX },
    [0x91]{ "sta", 2, 5, INDY },

    [0x86]{ "stx", 2, 3, ZP },
    [0x96]{ "stx", 3, 4, ZPY },
    [0x8E]{ "stx", 3, 4, ABS },

    [0x84]{ "sty", 2, 3, ZP },
    [0x94]{ "sty", 2, 4, ZPX },
    [0x8C]{ "sty", 3, 4, ABS },

    [0xAA]{ "tax", 1, 2, IMP },

    [0xA8]{ "tay", 1, 2, IMP },

    [0xBA]{ "tsx", 1, 2, IMP },

    [0x8A]{ "txa", 1, 2, IMP },

    [0x9A]{ "txs", 1, 2, IMP },

    [0x98]{ "tya", 1, 2, IMP },
};

int opcodes6502[] = {
    // 0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
    0x00,0x01,0x05,0x06,0x08,0x09,0x0A,0x0D,0x0E,0x10,0x11,0x15,0x16,0x18,0x19,0x1D,    // 0
    0x1E,0x20,0x21,0x24,0x25,0x26,0x28,0x29,0x2A,0x2C,0x2D,0x2E,0x30,0x31,0x35,0x36,    // 1
    0x38,0x39,0x3D,0x3E,0x40,0x41,0x45,0x46,0x48,0x49,0x4A,0x4C,0x4D,0x4E,0x51,0x55,    // 2
    0x56,0x58,0x59,0x5D,0x5E,0x60,0x61,0x65,0x66,0x68,0x69,0x6A,0x6C,0x6D,0x6E,0x70,    // 3
    0x71,0x75,0x76,0x78,0x79,0x7D,0x7E,0x81,0x84,0x85,0x86,0x88,0x8A,0x8C,0x8D,0x8E,    // 4
    0x90,0x91,0x94,0x95,0x96,0x98,0x99,0x9A,0x9D,0xA0,0xA1,0xA2,0xA4,0xA5,0xA6,0xA8,    // 5
    0xA9,0xAA,0xAC,0xAD,0xAE,0xB0,0xB1,0xB4,0xB5,0xB6,0xB8,0xB9,0xBA,0xBC,0xBD,0xBE,    // 6
    0xC0,0xC1,0xC4,0xC5,0xC6,0xC8,0xC9,0xCA,0xCC,0xCD,0xCE,0xD0,0xD1,0xD5,0xD6,0xD8,    // 7
    0xD9,0xDD,0xDE,0xE0,0xE1,0xE4,0xE5,0xE6,0xE8,0xE9,0xEA,0xEB,0xEC,0xED,0xEE,0xF0,    // 8
    0xF1,0xF5,0xF6,0xF8,0xF9,0xFD,0xFE                                                  // 9
};

int opcodes6510[] = {
    // 0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
    0x00,0x01,0x02,0x03,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0D,0x0E,0x0F,0x10,0x11,    // 0
    0x13,0x15,0x16,0x17,0x18,0x19,0x1B,0x1D,0x1E,0x1F,0x20,0x21,0x23,0x24,0x25,0x26,    // 1
    0x27,0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,0x30,0x31,0x33,0x35,0x36,0x37,0x38,    // 2
    0x39,0x3B,0x3D,0x3E,0x3F,0x40,0x41,0x43,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,    // 3
    0x4D,0x4E,0x4F,0x51,0x53,0x55,0x56,0x57,0x58,0x59,0x5B,0x5D,0x5E,0x5F,0x60,0x61,    // 4
    0x63,0x65,0x66,0x67,0x68,0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,0x70,0x71,0x73,0x75,    // 5
    0x76,0x77,0x78,0x79,0x7B,0x7D,0x7E,0x7F,0x81,0x83,0x84,0x85,0x86,0x87,0x88,0x8A,    // 6
    0x8B,0x8C,0x8D,0x8E,0x8F,0x90,0x91,0x94,0x95,0x96,0x97,0x98,0x99,0x9A,0x9C,0x9D,    // 7
    0x9E,0x9F,0xA0,0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,    // 8
    0xAE,0xAF,0xB0,0xB1,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBC,0xBD,0xBE,0xBF,    // 9
    0xC0,0xC1,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xD0,    // A
    0xD1,0xD3,0xD5,0xD6,0xD7,0xD8,0xD9,0xDB,0xDD,0xDE,0xDF,0xE0,0xE1,0xE4,0xE5,0xE6,    // B
    0xE8,0xE9,0xEA,0xEB,0xEC,0xED,0xEE,0xF0,0xF1,0xF5,0xF6,0xF8,0xF9,0xFD,0xFE          // C
};

int main( int argc, char *argv[] )
{
    char    *infile_name        = NULL;
    char    *infile_nopath      = NULL;
    char    *temp_string        = NULL;

    int     c                   = 0;
    opcode  *current_opcode;
    int     i                   = 0;
    int     intend              = DEFAULT_INTEND;
    int     mode                = MODE6502;
    int     skipbytes           = 2;
    int     pc_end              = 0;
    int     pc_start            = 0x0801;
    int     pc                  = 0x0801;

    virtual_file assembly;

    if ( ( argc == 1 ) ||
         ( strcmp( argv[1], "-h" ) == 0 ) ||
         ( strcmp( argv[1], "-help" ) == 0 ) ||
         ( strcmp( argv[1], "-?" ) == 0 ) ||
         ( strcmp( argv[1], "--help" ) == 0 ) )
    {
        print_info();
        print_help();
        exit( EXIT_SUCCESS );
    }

    // getopt cmdline-argument handler
    opterr = 1;

    while ( ( c = getopt ( argc, argv, "m:s:" ) ) != -1 )
    {
        switch ( c )
        {
            case 'm':
                if ( sscanf( optarg, "%i", &mode) != 1 )
                {
                    printf( "\nError: -m needs an integer value for file offset\n" );
                    exit( EXIT_FAILURE );
                }
                if ( mode > 1 )
                {
                    printf( "\nError: -m illegal mode\n" );
                    exit( EXIT_FAILURE );
                }
                break;
            case 's':
                if ( sscanf( optarg, "%i", &skipbytes) != 1 )
                {
                    printf( "\nError: -s needs an integer value for file offset\n" );
                    exit( EXIT_FAILURE );
                }
                if ( skipbytes < 2 )
                {
                    printf( "\nError: -s must be at least 2 to calculate program counter\n" );
                    exit( EXIT_FAILURE );
                }
                break;
        }
    }

    // make sure a file was given
    if ( ( optind ) == argc )
    {
        printf( "\nError: no file specified.\n" );
        exit( EXIT_FAILURE );
    }

    // open files
    infile_name = newstr( argv[optind] );
    infile_nopath = basename( infile_name );

    printf( "; input filename:    %s\n", infile_nopath );
    printf( "; skip bytes:        %d\n", skipbytes );
    printf( "\n" );

    assembly = read_file( infile_name, skipbytes );
    pc = get_pc( infile_name, skipbytes );

    pc_end = pc_start + assembly.length;

    print_intend( intend );
    print_mode( mode );

    print_intend( intend );
    printf( "*= 0x%04x \n", pc );

    while ( pc < pc_end )
    {
        i = pc - pc_start;
        current_opcode = &opcodes[ assembly.data[i] ];

        print_intend( intend );

        if ( is_in( assembly.data[i], mode ) )
        {
            int bytes = current_opcode->bytes;
            int operand = 0x0000;

            switch ( current_opcode->addressing_mode )
            {
                case ACC:
                case IMP:
                default:
                    break;
                case IMM:
                case ZP:
                case ZPX:
                case ZPY:
                case INDX:
                case INDY:
                    operand = assembly.data[i+1];
                    break;
                case ABS:
                case ABSI:
                case ABSX:
                case ABSY:
                    operand = ( assembly.data[i+1] ) + ( assembly.data[i+2] << 8 );
                    break;
                case REL:
                    if ( assembly.data[i+1] < 0x80 )
                    {
                        operand = pc + ( assembly.data[i+1] ) + 2;
                    }
                    else
                    {
                        operand = pc - ( 0x100 - assembly.data[i+1] ) + 2;
                    }
                    break;
            }

            print_instruction( *current_opcode, operand );

            pc += bytes;
        }
        else
        {
            printf( "!byte 0x%02x", assembly.data[i] );
            pc++;
        }
        printf( "\n" );
    };
    printf( "\n" );

    free( infile_name );
    exit( EXIT_SUCCESS );
}

// file reader

virtual_file read_file( char *filename, int skipbytes )
{
    FILE    *infile             = NULL;
    int     i                   = 0;
    int     input_data          = 0;
    virtual_file vfile;

    infile = fopen( filename, "rb" );
    if ( infile == NULL )
    {
        printf( "\nError: couldn't read file \"%s\".\n", filename );
        exit( EXIT_FAILURE );
    }

    // forward infile according to skipbytes
    fseek( infile, skipbytes, 0 );

    while  ( ( input_data = fgetc( infile ) ) != EOF )
    {
        vfile.data[i] = input_data;
        i++;
    }
    vfile.length = i;

    strcpy ( vfile.name, filename );

    fclose( infile );
    return vfile;
}

// helper

int get_pc( char *filename, int skipbytes )
{
    FILE    *infile             = NULL;
    int     lobyte              = 0x00;
    int     hibyte              = 0x00;
    int     pc                  = 0x0801;

    infile = fopen( filename, "rb" );
    if ( infile == NULL )
    {
        printf( "\nError: couldn't read file \"%s\".\n", filename );
        exit( EXIT_FAILURE );
    }

    // forward infile according to skipbytes
    if ( skipbytes > 2 )
    {
        fseek( infile, skipbytes - 2, 0 );
    }

    lobyte = fgetc( infile );
    hibyte = fgetc( infile );

    fclose( infile );

    pc = ( lobyte + ( hibyte << 8 ) ) & 0xFFFF;

    return pc;
}

int is_in( int opcode, int mode )
{
    int i           = 0;
    int bool_val    = 0;

    switch ( mode )
    {
        case MODE6502:
        default:
            for ( i = 0; i < 0x97; i++ )
            {
                if ( opcode == opcodes6502[i] )
                {
                    bool_val = 1;
                }
            }
            break;
        case MODE6510:
            for ( i = 0; i < 0xCF; i++ )
            {
                if ( opcode == opcodes6510[i] )
                {
                    bool_val = 1;
                }
            }
            break;
    }

    return bool_val;
}

// print functions

void print_bits( unsigned int x )
{
    int i;

    for( i = ( sizeof( int ) * 8 ) - 1; i >= 0; i-- )
            ( x & ( 1u << i ) ) ? putchar( '1' ) : putchar( '0' );

    printf( "\n" );
}

void print_mode( int mode )
{
    switch ( mode )
    {
        case MODE6502:
        default:
            printf( "!cpu 6502" );
            break;
        case MODE6510:
            printf( "!cpu 6510" );
            break;
    }

    printf( "\n" );
}

void print_instruction( opcode opcode, int operand )
{
    int lobyte  = 0x00;
    int hibyte  = 0x00;

    lobyte = operand & 0xFF;

    if ( operand > 0xFF )
    {
        hibyte = ( operand & 0xFF00 ) >> 8;
    }

    printf( "%s", opcode.name );

    switch ( opcode.addressing_mode )
    {
        case ACC:
            // printf( "a" );
            break;
        case IMP:
            break;
        case IMM:
            printf( " #0x%02x", lobyte );
            break;
        case ZP:
            printf( " 0x%02x", lobyte );
            break;
        case ZPX:
            printf( " 0x%02x,x", lobyte );
            break;
        case ZPY:
            printf( " 0x%02x,y", lobyte );
            break;
        case ABS:
            printf( " 0x%02x%02x", hibyte, lobyte );
            break;
        case ABSI:
            printf( " (0x%02x%02x)", hibyte, lobyte );
            break;
        case ABSX:
            printf( " 0x%02x%02x,x", hibyte, lobyte );
            break;
        case ABSY:
            printf( " 0x%02x%02x,y", hibyte, lobyte );
            break;
        case INDX:
            printf( " (0x%02x,x)", lobyte );
            break;
        case INDY:
            printf( " (0x%02x),y", lobyte );
            break;
        case REL:
            printf( " 0x%02x%02x", hibyte, lobyte );
            break;
        default:
            break;
    }
    // printf( "\n" );
}

void print_intend( int n )
{
    int i = 0;
    for ( i = 0; i < n; i++ )
    {
        printf( " " );
    }
}

// string helper functions

char *newstr( char *initial_str )
{
    int     num_chars;
    char    *new_str;

    num_chars = strlen( initial_str ) + 1;
    new_str = malloc ( num_chars );

    strcpy ( new_str, initial_str );

    return new_str;
}

// info print stuff

void print_info()
{
    const char* version = VERSION;

    printf( "===============================================================================\n" );
    printf( "acmedisass - Version %s\n", version );
    printf( "                                                            by Spider Jerusalem\n");
    printf( "===============================================================================\n" );
    printf( "Very simple 6502/6510 disassembler that outputs sourcecode for the acme\n" );
    printf( "crossassembler by Marco Baye. \n" );
    printf( "\n" );
}

void print_help()
{
    printf( "Usage:\n" );
    printf( "======\n" );
    printf( "   acmedisass [options] {file}\n" );
    printf( "\n" );

  //printf( "===============================================================================\n" );
    printf( "Command line options:\n" );
    printf( "=====================\n" );
    printf( "   -m mode    : acme cpu mode. 0 : !cpu 6502, 1 : !cpu 6510\n" );
    printf( "                [default: 0]\n" );
    printf( "   -s skip    : number of bytes to be skipped.\n" );
    printf( "                low-/highbyte combination in ( skipbytes - 2 )\n" );
    printf( "                will be used for initial program counter.\n" );
    printf( "                [default: 2]\n" );
    printf( "\n" );
    printf( "Have fun!\n" );
}
