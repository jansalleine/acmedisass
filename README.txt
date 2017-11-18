===============================================================================
acmedisass - Version 1.0
                                                            by Spider Jerusalem
===============================================================================
Very simple 6502/6510 disassembler that outputs sourcecode for the acme
crossassembler by Marco Baye. 

Usage:
======
   acmedisass [options] {file}

Command line options:
=====================
   -m mode    : acme cpu mode. 0 : !cpu 6502, 1 : !cpu 6510
                [default: 0]
   -s skip    : number of bytes to be skipped.
                low-/highbyte combination in ( skipbytes - 2 )
                will be used for initial program counter.
                [default: 2]

Have fun!
