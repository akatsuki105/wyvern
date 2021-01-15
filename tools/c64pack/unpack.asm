
;* C64PACK unpacker for GB
;*
;*   Written in RGBDS
;*
;*  V1.0 - Ported to GB by Jeff Frohwein, started 24-Jul-99
;*
;* Note: If you unpack to VRAM than the screen needs to be
;* turned off because no checks for VRAM available are made.

; File format

; db 'P'
; db escape
; db escMask
; .... data ....

; /* LZ77: yet yet yet improved encoding */
; /* escape bits default = 3 */
; /* eee00010                            - eof */
; /* eee00000 nnneeeee                   - escape (n = new escape) */
; /* eeelllp0 pppppppp                   - short LZ77 L=1..7 (L+1) bytes copied */
; 
; /* eee00001 llllllll                   - 0-RLE  (L+1) 0-bytes */
; /* eeelll01 vvvvvvvv                   - short RLE  L=1..7 (L+1) bytes */
; /* eeelll11 1lllllll vvvvvvvv          - long RLE  L=0..1023 (L+1) bytes */
; /* eeeppp11 0llllllp pppppppp          - long LZ77 L=0..63 (L+1) bytes copied */

        PUSHS

        SECTION "Unpack Vars",HRAM

esc       DB
escMask   DB
NescMask  DB
OutPtr    DW
regx      DB
regy      DB
lzpos     DW

        POPS

; ****** Unpack c64pack data ******
; Entry: HL = Source packed data
;        DE = Destination for unpacked data

Unpack:
        ld16r   OutPtr,de

        inc     hl

        ld      a,[hli]
        ld      [esc],a

        ld      a,[hli]
        ld      [escMask],a
        cpl
        ld      [NescMask],a

        ; ld16    de,hl
        ld      d, h
        ld      e, l

        jr      .main

; DE = InPtr

.doesc:
        ret     c               ; All done!

        ld      a,[de]
        inc     de

        ld      b,a

        ld      a,[escMask]
        and     b
        ld      [regx],a

        ld      a,[NescMask]
        and     b
        ld      b,a

        ld      a,[esc]
        or      b
        ld      b,a

        ld      a,[regx]
        ld      [esc],a

        ld      a,b
        ld      [regx],a

.normal:

        ld16r   hl,OutPtr

        ld      a,[regx]
        ld      [hl+],a         ; no, add it as-is

        ld16r   OutPtr,hl

.main:
        ld      a,[de]          ; get char
        inc     de

        ld      [regx],a        ; store it

        ld      c,a
        ld      a,[escMask]
        and     c               ; *** compare escape bits

        ld      c,a
        ld      a,[esc]
        cp      c
        ld      a,c

        jr      nz,.normal


        xor     a
        ld      [regy],a        ; Reset Y to a predefined state

        ld      a,[NescMask]
        ld      b,a

        ld      a,[regx]        ; X has the escape byte, now separate the data
        and     b               ; *** mask MSbits out (escape bits)
        srl     a               ; ??? 0/1 to C (0=Short LZ/escape, 1=Long LZ/RLE)
        jp      nc,.lz          ; Y is zero

        srl     a               ; 0/1 to C (0=Short RLE/0-RLE, 1=Long LZ/Long RLE)
        jr      c,.longs

        ld      [regx],a        ; X has "len", Y = "msb" = 0

        ld      a,[de]          ; Short RLE or 0-RLE, get value or len
        inc     de

        ld      b,a
        ld      a,[regx]
        or      a
        ld      a,b

        jr      nz,.dorle       ; Short RLE     Y=0, X=len, A=value

        ld      [regx],a        ; 0-RLE A->len  Y=0, X=len, A=0
        xor     a
.dorle:
        ld      hl,regx         ; adjust for cpx#$ff;bne -> bne
        inc     [hl]
        ld      b,[hl]

        ld      c,a
        ld16r   hl,OutPtr
        ld      a,c

        bit     0,b
        jr      nz,.orleloop
.rleloop:
        ld      [hl+],a         ; copy it 1..1024 times
        dec     b
.orleloop:
        ld      [hl+],a         ; copy it 1..1024 times
        dec     b

        jr      nz,.rleloop     ; 1 pass for 0, 256 for 255 etc.

        ld16    OutPtr,hl

        ld      hl,regy
        dec     [hl]

;        bmi main

        bit     7,[hl]
        jr      nz,.main

        push    af
        ld      a,[regy]
        ld      [regx],a

        xor     a
        ld      [regy],a
        pop     af

.loop1:
        ld16    hl,OutPtr

        ld      b,256/16
.loop0:
        ld      [hl+],a
        ld      [hl+],a
        ld      [hl+],a
        ld      [hl+],a
        ld      [hl+],a
        ld      [hl+],a
        ld      [hl+],a
        ld      [hl+],a
        ld      [hl+],a
        ld      [hl+],a
        ld      [hl+],a
        ld      [hl+],a
        ld      [hl+],a
        ld      [hl+],a
        ld      [hl+],a
        ld      [hl+],a

        dec     b
        jr      nz,.loop0

        ld16    OutPtr,hl

        ld      hl,regx
        dec     [hl]

        bit     7,[hl]
        jr      z,.loop1

        jp      .main

.longs: ld      [regy],a        ; "msb"

        ld      a,[de]
        inc     de

        bit     7,a
        jr      z,.lz           ; Negative=long RLE, Positive=long LZ77

        sla     a               ; C set (lllllll0 C=1)

        ld      hl,regy         ; (lll C=1)
        srl     [hl]            ; (0ll C=l)
                                ; Y is 0..127 (actually 0..3)

                                ; (lllllll0 C=l)
        rra                     ; (llllllll C=0)

        ld      [regx],a

        ld      a,[de]
        inc     de

        jp      .dorle


.lz:
        srl     a               ; Shift p (9th bit) to C
        ld      [regx],a        ; X has "len" 0..127 (0..63)

        jp      z,.doesc        ; len==0, escape

        ld      a,[regy]        ; upper 0/3 P bits (zero for short lz)
        rla                     ; msb = msb*2 + C, C gets cleared
        ld      [lzpos+1],a

        ld      a,[OutPtr]
        ld      b,a

        ld      a,[lzpos+1]
        ld      c,a

        ld      a,[de]          ; get "lsb", clears C
        inc     de
                                ; already eor:ed in the compressor
        ;eor #255               ; offset LSB 2's complement -1 (i.e. -X = ~X+1)


        add     b               ; -offset -1 + curpos (C is clear)
        ld      [lzpos],a

        ld      a,[OutPtr+1]
        ccf
        sbc     c               ; takes C into account
        ld      [lzpos+1],a     ; copy X+1 number of chars from LZPOS to OUTPOS


        push    de

        ld16r   hl,lzpos
        ld16r   de,OutPtr

        ld      a,[regx]
        ld      b,a
        inc     b

        inc     b
        srl     b
        jr      nc,.olzloop
.lzloop:
        ld      a,[hl+]
        ld      [de],a
        inc     de
.olzloop:
        ld      a,[hl+]
        ld      [de],a
        inc     de

        dec     b
        jr      nz,.lzloop       ; X+1 loops, (X = 0..127)

        ld16r   OutPtr,de

        pop     de

        jp      .main

