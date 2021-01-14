; Wyvern decompressor in RGBASM format.

_decompress::
    ld      hl, sp+5
    ; de = [sp+5] << 8 | [sp+4] = dest
    ld      a, [hld]
    ld      d, a
    ld      a, [hld]
    ld      e, a
    ; hl = [sp+3] | [sp+2]
    ld      a, [hld]
    ld      l, [hl]
    ld      h, a
    ; fallthrough

; hl = source; de = dest
; bc is not changed
decompress::
    printPC "Decompress start: "
    push    bc
.loop
    ld      a, [hli]                    ; load command
    or      a
    jr      z, .exit                    ; exit, if last byte
    bit     7, a
    jr      nz, .stringOrTrash          ; string functions
    bit     6, a
    jr      nz, .word
; .byte
    and     %00111111                   ; calc counter
    inc     a
    ld      b, a
    ld      a, [hli]
    ; hl = dest
    push    hl
    ld      h, d
    ld      l, e
.byteLoop
    ld      [hli], a
    dec     b
    jr      nz, .byteLoop
    ld      d, h
    ld      e, l
    pop     hl
    jr      .loop                       ; next command
.word                                   ; RLE word
    and     %00111111                   
    inc     a
    ld      b, a                        ; b = length
    ld      a, [hli]                    
    ld      c, [hl]
    inc     hl                          ; load word into ac
    push    hl
    ld      h, d
    ld      l, e                        ; hl = dest
.wordLoop
    ld      [hli], a                    ; store word(ac) into dest(hl)
    ld      [hl], c
    inc     hl
    dec     b
    jr      nz, .wordLoop
    ld      d, h
    ld      e, l
    pop     hl
    jr      .loop                       ; next command
.stringOrTrash
    bit     6, a
    jr      nz, .trash
; .string
    ; offset
    ld      c, [hl]                     ; c = offset_lower
    ld      b, %11111111                ; b = offset_upper(in shortString)
    bit     5, a
    jr      z, .done
    ; if bit5 is set, long string
    inc     hl                          ; b = offset_upper
    ld      b, [hl]
.done
    push    hl
    ld      h, d                        ; hl = de(dest)
    ld      l, e
    add     hl, bc                      ; hl(from) = hl(dest) + bc(offset)
    ; length
    and     a, %00011111
    inc     a
    ld      b, a
    ; check if a + e > 0xff
    add     a, e
    jr      c, .stringLoop
.fastStringLoop ; faster than .stringLoop because of using `inc e` instead of `inc de`
    ld      a, [hli]
    ld      [de], a
    inc     e                           ; reduce 1 cycle
    dec     b
    jr      nz, .fastStringLoop
    jr      .stringLoopDone
.stringLoop
    ld      a, [hli]
    ld      [de], a
    inc     de
    dec     b
    jr      nz, .stringLoop
.stringLoopDone
    pop     hl
    inc     hl
    jr      .loop                       ; next command
.trash                                  ; string copy
    and     %00111111
    inc     a
    ld      b, a
    ; check if a + e > 0xff
    add     a, e
    jr      c, .trashLoop
.fastTrashLoop  ; faster than .trashLoop because of using `inc e` instead of `inc de`
    ld      a, [hli]
    ld      [de], a
    inc     e                           ; reduce 1 cycle
    dec     b
    jr      nz, .fastTrashLoop
    jr      .loop 
.trashLoop                    
    ld      a, [hli]
    ld      [de], a
    inc     de
    dec     b
    jr      nz, .trashLoop
    jr      .loop                       ; next command. NOTE: bc=0x0000
.exit
    pop     bc
    ret
    printPC "Decompress end: "
