assume adl=1
    section .bss                ; Tells the eZ80 this is variable RAM
    public scratch1
    public scratch2
    public scratch3 ; where yoou assign color
    public rectaddr
    scratch1:  rb 1             ; Allocates 1 byte (Base address)
    scratch2:  rb 1             ; Automatically equals scratch1 + 1
    scratch3:  rb 1             ; Automatically equals scratch2 + 1
    rectaddr: rb 3
    scratch_row: rb 32
    DX: rb 3
    DY: rb 3
    DZ: rb 3
    mulscr: rb 3
    section .data
    public _rect_8x8
    public _optomized_raycast_rect_8x8
    public _getscratch3
    public _getrectaddr
    public _notfullfillscreen
    public _optomized_raycast_rect_5x5
    public _optomized_raycast_rect_5x5_no2
    public _Sprite_32x16to32x32
    public _Sprite_32x8to32x32
    public _rect8x8complete
    public _quicknotfullfillscreen
    public _ASM_DDA_FULL
    public _fast_mul16 
    ;section .bss
    public dda_deltax
    public dda_deltay
    public dda_deltaz
    public dda_currblock
    public _getddadeltax 
    public _getddadeltay 
    public _getddadeltaz
    public _getddacurrblock
    dda_deltax:    rb 3
    dda_deltay:    rb 3
    dda_deltaz:    rb 3
    dda_currblock: rb 3
    public dda_tmaxx
    public dda_tmaxy
    public dda_tmaxz
    dda_tmaxx: rb 3
    dda_tmaxy: rb 3
    dda_tmaxz: rb 3
    public _getddatmaxx
    public _getddatmaxy
    public _getddatmaxz
    
    
; void rect_8x8(int x,  uint8_t y,  uint8_t color,  uint8_t* screenaddr)
;uint8* getscratch3()
_getscratch3:
    ld hl, scratch3
    ret
;uint8* getrectaddr()
_getrectaddr:
    ld hl, rectaddr
    ret
_rect_8x8:
    push ix
    ld ix, 0
    add ix, sp
    ld hl, 0
    ld de, 0
    ld bc, (ix+6)      ; bc = x
    ld d, (ix+9)       ; d = y
    ld l, d
    add hl, hl
    add hl, hl
    add hl, hl
    add hl, hl
    add hl, hl
    add hl, hl          ; hl = y*64

    add hl, de           ; += y*256 -> y*320
    add hl, bc            ; += x
    ld de, (ix+15)          ; de = screenaddr
    add hl, de                ; hl = start address

    ld a, (ix+12)               
    ld (scratch3), a
    ;ld (scratch2), a
    ;ld (scratch3), a
    ld de, (scratch1)           ; de = color in all 3 bytes
    ld d, a
    ld e, a
    ld bc, 320                    ; full row stride
    push hl
    pop ix                          ; ix = screen address

    ld (ix), de ; 0,1,2
    ld (ix+3), de; 3,4,5
    ld (ix+5), de;5,6,7
    add ix, bc

    ld (ix), de
    ld (ix+3), de
    ld (ix+5), de
    add ix, bc

    ld (ix), de
    ld (ix+3), de
    ld (ix+5), de
    add ix, bc

    ld (ix), de
    ld (ix+3), de
    ld (ix+5), de
    add ix, bc

    ld (ix), de
    ld (ix+3), de
    ld (ix+5), de
    add ix, bc

    ld (ix), de
    ld (ix+3), de
    ld (ix+5), de
    add ix, bc

    ld (ix), de
    ld (ix+3), de
    ld (ix+5), de
    add ix, bc

    ld (ix), de
    ld (ix+3), de
    ld (ix+5), de

    pop ix
    ret

; _optomized_raycast_rect_8x8(). be sure to change scratch every frame.
; int fast_mul16(int a, int b)
; Signed 16x16 -> 24-bit multiply. Result must fit in 24 bits
; (e.g. a 12-bit value times a 9-bit value).
_fast_mul16:
    push ix
    ld ix, 0
    add ix, sp
    push iy
    ld hl, (ix+6)      ; hl = a  (h=A1, l=A0)
    ld de, (ix+9)      ; de = b  (d=B1, e=B0)

    ld a, h
    xor d              ; bit7 of a = result sign. 'a' is untouched by mlt/push/pop
                       ; below, so it just rides here with no save/restore needed.

    bit 7, h
    jr z, .apos
    push de
    ex de, hl
    ld hl, 0
    or a
    sbc hl, de
    pop de
.apos:
    bit 7, d
    jr z, .bpos
    push hl
    ld hl, 0
    or a
    sbc hl, de
    ex de, hl
    pop hl
.bpos:
    ; hl = |a|, de = |b|, sign parked in a's bit7

    ld iy, 0
    ld bc, 0           ; cleared ONCE -- see caveat below

    ld b, l
    ld c, e
    mlt bc             ; bc = A0*B0
    add iy, bc

    ld b, h
    ld c, e

    mlt bc             ; bc = A1*B0
    xor a
    ld (mulscr), a
    ld a, c
    ld (mulscr+1), a
    ld a, b
    ld (mulscr+2), a
    push af
    ld bc, (mulscr)
    add iy, bc

    ld b, l
    ld c, d
    mlt bc             ; bc = A0*B1
    xor a
    ld (mulscr), a
    ld a, c
    ld (mulscr+1), a
    ld a, b
    ld (mulscr+2), a
    ld bc, (mulscr)
    add iy, bc
    pop af

    push iy
    pop hl
    bit 7, a
    jr z, .noneg
    ex de, hl
    ld hl, 0
    or a
    sbc hl, de
.noneg:
    pop iy
    pop ix
    ret
_optomized_raycast_rect_8x8:
    push ix
    ld ix, (rectaddr)
    ld a, (scratch3)        
    ld de, (scratch1)           ; de = color in all 3 bytes
    ld d, a
    ld e, a
    ld bc, 320

    ld (ix), de ; 0,1,2
    ld (ix+3), de; 3,4,5
    ld (ix+5), de;5,6,7
    add ix, bc

    ld (ix), de
    ld (ix+3), de
    ld (ix+5), de
    add ix, bc

    ld (ix), de
    ld (ix+3), de
    ld (ix+5), de
    add ix, bc

    ld (ix), de
    ld (ix+3), de
    ld (ix+5), de
    add ix, bc

    ld (ix), de
    ld (ix+3), de
    ld (ix+5), de
    add ix, bc

    ld (ix), de
    ld (ix+3), de
    ld (ix+5), de
    add ix, bc

    ld (ix), de
    ld (ix+3), de
    ld (ix+5), de
    add ix, bc

    ld (ix), de
    ld (ix+3), de
    ld (ix+5), de



    ld bc, 16
    ld hl, (rectaddr)
    add hl, bc
    ld (rectaddr), hl
    pop ix
    ret

_rect8x8complete:
    push ix
    ld ix, 0
    add ix, sp
    ld de, (ix+6)
    ld hl, 320;
    add hl, de
    ex de, hl
    ;now hl is first row of screen and de is second
    ld a, 23
    .sloop:
    ld bc, 320*7 ; length
    ldir
    ld bc, 320
    add hl, bc;go to next row
    ex de, hl
    add hl, bc; go to next row
    ex de, hl
    dec a
    cp a, 0
    jr nz, .sloop
    
    pop ix
    ret
    
_notfullfillscreen:
	push ix
	ld ix, 0
	add ix, sp
	ld a, (ix+6); loads color into a
	ld hl, (ix+9)
	ld de, (ix+9)
	inc de
	ld (hl), a
	ld bc, 320*185-1
	ldir
	pop ix
	ret

_optomized_raycast_rect_5x5_no2:
    push ix
    ld ix, (rectaddr)
    ld a, (scratch3)        
    ld de, (scratch1)           ; de = color in all 3 bytes
    ld d, a
    ld e, a
    ld bc, 320

    ld (ix), de
    ld (ix+2), de
    add ix, bc

    ld (ix), de
    ld (ix+2), de
    add ix, bc

    ld (ix), de
    ld (ix+2), de
    add ix, bc

    ld (ix), de
    ld (ix+2), de
    add ix, bc

    ld (ix), de
    ld (ix+2), de
    add ix, bc


    ld bc, 5
    ld hl, (rectaddr)
    add hl, bc
    ld (rectaddr), hl
    pop ix
    ret
_optomized_raycast_rect_5x5:
    push ix
    ld ix, (rectaddr)
    ld a, (scratch3)        
    ld de, (scratch1)           ; de = color in all 3 bytes
    ld d, a
    ld e, a
    ld bc, 320

    ld (ix), de
    ld (ix+2), de
    add ix, bc

    ld (ix), de
    ld (ix+2), de
    add ix, bc

    ld (ix), de
    ld (ix+2), de
    add ix, bc

    ld (ix), de
    ld (ix+2), de
    add ix, bc

    ld (ix), de
    ld (ix+2), de
    add ix, bc


    ld bc, 10
    ld hl, (rectaddr)
    add hl, bc
    ld (rectaddr), hl
    pop ix
    ret

; takes in a pointer, and pointer to screen where to start

_Sprite_32x16to32x32:
    push ix
    ld ix, 6
    add ix, sp
    ld hl, (ix)      ; hl = src (32x16 sprite, 32 bytes/row)
    ld de, (ix+3)    ; de = dst (into 320-wide destination buffer)

    ld a, 16         ; 16 source rows to process

.row_loop:
    push hl          ; save source row start, we need it twice

    ld bc, 32
    ldir             ; copy row -> dst row A   (hl+=32, de+=32, bc=0)

    pop hl           ; restore source row start for the duplicate copy

    ld bc, 288       ; 320 - 32, skip rest of dst row A to reach dst row B
    ex de, hl
    add hl, bc
    ex de, hl

    ld bc, 32
    ldir             ; copy same row -> dst row B (hl+=32 -> next src row! de+=32)

    ld bc, 288       ; skip rest of dst row B to reach the next dst row-pair
    ex de, hl
    add hl, bc
    ex de, hl

    dec a
    jr nz, .row_loop
    pop ix
    ret


_Sprite_32x8to32x32:
    push ix
    ld ix, 6
    add ix, sp
    ld hl, (ix)      ; hl = src (32x8 sprite, 32 bytes/row)
    ld de, (ix+3)    ; de = dst (into 320-wide destination buffer)

    ld a, 8          ; 8 source rows to process

.row_loop2:
    push hl          ; save src row start (need it for copies 2 and 3)
    ld bc, 32
    ldir             ; copy1: hl+=32 (past row), de+=32

    pop hl           ; restore src row start
    ld bc, 288
    ex de, hl
    add hl, bc
    ex de, hl        ; de advances to next dst row

    push hl          ; save src row start again (need it for copy 3)
    ld bc, 32
    ldir             ; copy2

    pop hl           ; restore src row start
    ld bc, 288
    ex de, hl
    add hl, bc
    ex de, hl

    push hl          ; save src row start once more (need it for copy 4)
    ld bc, 32
    ldir             ; copy3

    pop hl           ; restore src row start
    ld bc, 288
    ex de, hl
    add hl, bc
    ex de, hl

    ld bc, 32
    ldir             ; copy4 — no push/pop here, let hl advance to next src row

    ld bc, 288
    ex de, hl
    add hl, bc
    ex de, hl        ; de skips to the start of the next 4-row destination block

    dec a
    jr nz, .row_loop2

    pop ix
    ret


; shared 32-byte scratch row, reused by both routines below
; (adjust the reservation directive if your assembler doesn't support `rb`
;  — match whatever convention getscratch3()'s buffer uses elsewhere in stuff.asm)

; =========================================================
; void Sprite_8x8to32x32(uint8_t* src, uint8_t* dst)
; src: 8x8 sprite, 8 bytes/row (64 bytes total)
; dst: destination inside a 320-byte-stride buffer
; =========================================================
public _Sprite_8x8to32x32
_Sprite_8x8to32x32:
    push ix
    ld ix, 6
    add ix, sp
    ld hl, (ix)      ; hl = src
    ld de, (ix+3)    ; de = dst

    ld a, 8          ; 8 source rows

.row8_loop:
    push af

    ; expand this source row (8 bytes) into scratch_row (32 bytes), 4x each byte
    ld b, 8
    push de
    ld de, scratch_row
.expand8_loop:
    ld a, (hl)
    inc hl
    ld (de), a
    inc de
    ld (de), a
    inc de
    ld (de), a
    inc de
    ld (de), a
    inc de
    djnz .expand8_loop
    pop de

    push hl              ; save next src row pointer across the 4 vertical writes

    ld hl, scratch_row
    ld bc, 32
    ldir                 ; dst row 1
    ld bc, 288
    ex de, hl
    add hl, bc
    ex de, hl

    ld hl, scratch_row
    ld bc, 32
    ldir                 ; dst row 2
    ld bc, 288
    ex de, hl
    add hl, bc
    ex de, hl

    ld hl, scratch_row
    ld bc, 32
    ldir                 ; dst row 3
    ld bc, 288
    ex de, hl
    add hl, bc
    ex de, hl

    ld hl, scratch_row
    ld bc, 32
    ldir                 ; dst row 4
    ld bc, 288
    ex de, hl
    add hl, bc
    ex de, hl

    pop hl                ; restore next src row pointer
    pop af
    dec a
    jr nz, .row8_loop

    pop ix
    ret

; =========================================================
; void Sprite_16x16to32x32(uint8_t* src, uint8_t* dst)
; src: 16x16 sprite, 16 bytes/row (256 bytes total)
; dst: destination inside a 320-byte-stride buffer
; =========================================================
public _Sprite_16x16to32x32
_Sprite_16x16to32x32:
    push ix
    ld ix, 6
    add ix, sp
    ld hl, (ix)
    ld de, (ix+3)

    ld a, 16         ; 16 source rows

.row16_loop:
    push af

    ; expand this source row (16 bytes) into scratch_row (32 bytes), 2x each byte
    ld b, 16
    push de
    ld de, scratch_row
.expand16_loop:
    ld a, (hl)
    inc hl
    ld (de), a
    inc de
    ld (de), a
    inc de
    djnz .expand16_loop
    pop de

    push hl              ; save next src row pointer across the 2 vertical writes

    ld hl, scratch_row
    ld bc, 32
    ldir                 ; dst row A
    ld bc, 288
    ex de, hl
    add hl, bc
    ex de, hl

    ld hl, scratch_row
    ld bc, 32
    ldir                 ; dst row B
    ld bc, 288
    ex de, hl
    add hl, bc
    ex de, hl

    pop hl                ; restore next src row pointer
    pop af
    dec a
    jr nz, .row16_loop

    pop ix
    ret

; ============================================================
; FOURTH FIX: tmaxx/tmaxy/tmaxz are no longer reset to
; delta on every call. They're now persistent state
; (dda_tmaxx/y/z) that ASM_DDA_FULL reads on entry and
; writes back on exit, so:
;  (a) retries (goto start_loop after a transparent texture
;      pixel) resume from where the ray actually was instead
;      of restarting at t=0, and
;  (b) the caller can read back the exact tmax at the moment
;      of the hit, which the texture-coordinate math needs.
; Caller must set dda_tmaxx/y/z = deltax/y/z ONCE at the start
; of a column (mirroring the existing flint tmaxx=deltax etc.
; in TRACE_COLUMN), then just let each call's write-back feed
; the next call's read.
; ============================================================

RBP1   equ 19   ; reg_block_num+1  (reg_block_num=16 in main.cpp)
RBP1X2 equ 38   ; 2*(reg_block_num+1)

macro ASM_DDA_LOOP SX, SY, SZ, ENDX, ENDY, ENDZ, LX, LXZ, LYZ, LDONE
;assuming tmaxx in hl, tmaxy in bc, tmaxz in de, curr_block in IX.
or a
sbc hl, bc
jp p, LX
    add hl, bc
    or a
    sbc hl, de
    jp p, LXZ
        add hl, de
        push de
            ld de, SX
            add ix, de
            ld de, (dda_deltax)
            add hl, de
        pop de
        xor a
        cp a, (ix)
        jp nz, ENDX
        jp LDONE

LXZ:
        add hl, de
        push bc
            ld bc, SZ
            add ix, bc
            ex de, hl
            ld bc, (dda_deltaz)
            add hl, bc
            ex de, hl
        pop bc
        xor a
        cp a, (ix)
        jp nz, ENDZ
        jp LDONE


LX:
    add hl, bc
    ex de, hl
    or a
    sbc hl, bc
    jp m, LYZ
        add hl, bc
        ex de, hl
        push hl              ; save tmaxx
        ld h, b
        ld l, c              ; hl = tmaxy
        push bc
        ld bc, SY
        add ix, bc
        pop bc
        ld bc, (dda_deltay)
        add hl, bc           ; hl = tmaxy+deltay
        ld b, h
        ld c, l              ; bc = new tmaxy
        pop hl               ; hl = tmaxx restored
        xor a
        cp a, (ix)
        jp nz, ENDY
        jr LDONE


LYZ:
        add hl, bc
        push bc
            ld bc, SZ
            add ix, bc
            ld bc, (dda_deltaz)
            add hl, bc
            ex de, hl
        pop bc
        xor a
        cp a, (ix)
        jp nz, ENDZ

LDONE:
end macro

; XJ=WY*WZ=1024, YJ=WZ=32 (WX=WY=WZ=32 per main.cpp)
dda_q0:
    ld hl, (dda_tmaxx)     ; tmaxx = current tmax (NOT reset to delta -- see fix note above)
    ASM_DDA_LOOP -1024, -32, -1, dda_q0_end0, dda_q0_end1, dda_q0_end2, dda_q0_s1x, dda_q0_s1xz, dda_q0_s1yz, dda_q0_s1d
    ASM_DDA_LOOP -1024, -32, -1, dda_q0_end0, dda_q0_end1, dda_q0_end2, dda_q0_s2x, dda_q0_s2xz, dda_q0_s2yz, dda_q0_s2d
    ASM_DDA_LOOP -1024, -32, -1, dda_q0_end0, dda_q0_end1, dda_q0_end2, dda_q0_s3x, dda_q0_s3xz, dda_q0_s3yz, dda_q0_s3d
    ASM_DDA_LOOP -1024, -32, -1, dda_q0_end0, dda_q0_end1, dda_q0_end2, dda_q0_s4x, dda_q0_s4xz, dda_q0_s4yz, dda_q0_s4d
    ASM_DDA_LOOP -1024, -32, -1, dda_q0_end0, dda_q0_end1, dda_q0_end2, dda_q0_s5x, dda_q0_s5xz, dda_q0_s5yz, dda_q0_s5d
    ASM_DDA_LOOP -1024, -32, -1, dda_q0_end0, dda_q0_end1, dda_q0_end2, dda_q0_s6x, dda_q0_s6xz, dda_q0_s6yz, dda_q0_s6d
    ASM_DDA_LOOP -1024, -32, -1, dda_q0_end0, dda_q0_end1, dda_q0_end2, dda_q0_s7x, dda_q0_s7xz, dda_q0_s7yz, dda_q0_s7d
    ASM_DDA_LOOP -1024, -32, -1, dda_q0_end0, dda_q0_end1, dda_q0_end2, dda_q0_s8x, dda_q0_s8xz, dda_q0_s8yz, dda_q0_s8d
    ASM_DDA_LOOP -1024, -32, -1, dda_q0_end0, dda_q0_end1, dda_q0_end2, dda_q0_s9x, dda_q0_s9xz, dda_q0_s9yz, dda_q0_s9d
    ASM_DDA_LOOP -1024, -32, -1, dda_q0_end0, dda_q0_end1, dda_q0_end2, dda_q0_s10x, dda_q0_s10xz, dda_q0_s10yz, dda_q0_s10d
    ASM_DDA_LOOP -1024, -32, -1, dda_q0_end0, dda_q0_end1, dda_q0_end2, dda_q0_s11x, dda_q0_s11xz, dda_q0_s11yz, dda_q0_s11d
    ASM_DDA_LOOP -1024, -32, -1, dda_q0_end0, dda_q0_end1, dda_q0_end2, dda_q0_s12x, dda_q0_s12xz, dda_q0_s12yz, dda_q0_s12d
    ASM_DDA_LOOP -1024, -32, -1, dda_q0_end0, dda_q0_end1, dda_q0_end2, dda_q0_s13x, dda_q0_s13xz, dda_q0_s13yz, dda_q0_s13d
    ASM_DDA_LOOP -1024, -32, -1, dda_q0_end0, dda_q0_end1, dda_q0_end2, dda_q0_s14x, dda_q0_s14xz, dda_q0_s14yz, dda_q0_s14d
    ASM_DDA_LOOP -1024, -32, -1, dda_q0_end0, dda_q0_end1, dda_q0_end2, dda_q0_s15x, dda_q0_s15xz, dda_q0_s15yz, dda_q0_s15d
    ld a, 3            ; ran off the end, no hit (shouldn't happen if VD steps are enough)
    jp dda_q0_exit
dda_q0_end0:
    ld a, 0            ; x hit
    jp dda_q0_exit
dda_q0_end1:
    ld a, RBP1         ; y hit
    jp dda_q0_exit
dda_q0_end2:
    ld a, RBP1X2       ; z hit
dda_q0_exit:
    ld (dda_tmaxx), hl  ; write back tmax state (hl/bc/de are correct here regardless
    ld (dda_tmaxy), bc  ; of which path got us to this label -- see macro trace)
    ld (dda_tmaxz), de
    ld (dda_currblock), ix
    pop ix              ; restore caller's ix
    ret

dda_q1:
    ld hl, (dda_tmaxx)     ; tmaxx = current tmax (NOT reset to delta -- see fix note above)
    ASM_DDA_LOOP 1024, -32, -1, dda_q1_end0, dda_q1_end1, dda_q1_end2, dda_q1_s1x, dda_q1_s1xz, dda_q1_s1yz, dda_q1_s1d
    ASM_DDA_LOOP 1024, -32, -1, dda_q1_end0, dda_q1_end1, dda_q1_end2, dda_q1_s2x, dda_q1_s2xz, dda_q1_s2yz, dda_q1_s2d
    ASM_DDA_LOOP 1024, -32, -1, dda_q1_end0, dda_q1_end1, dda_q1_end2, dda_q1_s3x, dda_q1_s3xz, dda_q1_s3yz, dda_q1_s3d
    ASM_DDA_LOOP 1024, -32, -1, dda_q1_end0, dda_q1_end1, dda_q1_end2, dda_q1_s4x, dda_q1_s4xz, dda_q1_s4yz, dda_q1_s4d
    ASM_DDA_LOOP 1024, -32, -1, dda_q1_end0, dda_q1_end1, dda_q1_end2, dda_q1_s5x, dda_q1_s5xz, dda_q1_s5yz, dda_q1_s5d
    ASM_DDA_LOOP 1024, -32, -1, dda_q1_end0, dda_q1_end1, dda_q1_end2, dda_q1_s6x, dda_q1_s6xz, dda_q1_s6yz, dda_q1_s6d
    ASM_DDA_LOOP 1024, -32, -1, dda_q1_end0, dda_q1_end1, dda_q1_end2, dda_q1_s7x, dda_q1_s7xz, dda_q1_s7yz, dda_q1_s7d
    ASM_DDA_LOOP 1024, -32, -1, dda_q1_end0, dda_q1_end1, dda_q1_end2, dda_q1_s8x, dda_q1_s8xz, dda_q1_s8yz, dda_q1_s8d
    ASM_DDA_LOOP 1024, -32, -1, dda_q1_end0, dda_q1_end1, dda_q1_end2, dda_q1_s9x, dda_q1_s9xz, dda_q1_s9yz, dda_q1_s9d
    ASM_DDA_LOOP 1024, -32, -1, dda_q1_end0, dda_q1_end1, dda_q1_end2, dda_q1_s10x, dda_q1_s10xz, dda_q1_s10yz, dda_q1_s10d
    ASM_DDA_LOOP 1024, -32, -1, dda_q1_end0, dda_q1_end1, dda_q1_end2, dda_q1_s11x, dda_q1_s11xz, dda_q1_s11yz, dda_q1_s11d
    ASM_DDA_LOOP 1024, -32, -1, dda_q1_end0, dda_q1_end1, dda_q1_end2, dda_q1_s12x, dda_q1_s12xz, dda_q1_s12yz, dda_q1_s12d
    ASM_DDA_LOOP 1024, -32, -1, dda_q1_end0, dda_q1_end1, dda_q1_end2, dda_q1_s13x, dda_q1_s13xz, dda_q1_s13yz, dda_q1_s13d
    ASM_DDA_LOOP 1024, -32, -1, dda_q1_end0, dda_q1_end1, dda_q1_end2, dda_q1_s14x, dda_q1_s14xz, dda_q1_s14yz, dda_q1_s14d
    ASM_DDA_LOOP 1024, -32, -1, dda_q1_end0, dda_q1_end1, dda_q1_end2, dda_q1_s15x, dda_q1_s15xz, dda_q1_s15yz, dda_q1_s15d
    ld a, 3            ; ran off the end, no hit (shouldn't happen if VD steps are enough)
    jp dda_q1_exit
dda_q1_end0:
    ld a, 0            ; x hit
    jp dda_q1_exit
dda_q1_end1:
    ld a, RBP1         ; y hit
    jp dda_q1_exit
dda_q1_end2:
    ld a, RBP1X2       ; z hit
dda_q1_exit:
    ld (dda_tmaxx), hl  ; write back tmax state (hl/bc/de are correct here regardless
    ld (dda_tmaxy), bc  ; of which path got us to this label -- see macro trace)
    ld (dda_tmaxz), de
    ld (dda_currblock), ix
    pop ix              ; restore caller's ix
    ret

dda_q2:
    ld hl, (dda_tmaxx)     ; tmaxx = current tmax (NOT reset to delta -- see fix note above)
    ASM_DDA_LOOP -1024, 32, -1, dda_q2_end0, dda_q2_end1, dda_q2_end2, dda_q2_s1x, dda_q2_s1xz, dda_q2_s1yz, dda_q2_s1d
    ASM_DDA_LOOP -1024, 32, -1, dda_q2_end0, dda_q2_end1, dda_q2_end2, dda_q2_s2x, dda_q2_s2xz, dda_q2_s2yz, dda_q2_s2d
    ASM_DDA_LOOP -1024, 32, -1, dda_q2_end0, dda_q2_end1, dda_q2_end2, dda_q2_s3x, dda_q2_s3xz, dda_q2_s3yz, dda_q2_s3d
    ASM_DDA_LOOP -1024, 32, -1, dda_q2_end0, dda_q2_end1, dda_q2_end2, dda_q2_s4x, dda_q2_s4xz, dda_q2_s4yz, dda_q2_s4d
    ASM_DDA_LOOP -1024, 32, -1, dda_q2_end0, dda_q2_end1, dda_q2_end2, dda_q2_s5x, dda_q2_s5xz, dda_q2_s5yz, dda_q2_s5d
    ASM_DDA_LOOP -1024, 32, -1, dda_q2_end0, dda_q2_end1, dda_q2_end2, dda_q2_s6x, dda_q2_s6xz, dda_q2_s6yz, dda_q2_s6d
    ASM_DDA_LOOP -1024, 32, -1, dda_q2_end0, dda_q2_end1, dda_q2_end2, dda_q2_s7x, dda_q2_s7xz, dda_q2_s7yz, dda_q2_s7d
    ASM_DDA_LOOP -1024, 32, -1, dda_q2_end0, dda_q2_end1, dda_q2_end2, dda_q2_s8x, dda_q2_s8xz, dda_q2_s8yz, dda_q2_s8d
    ASM_DDA_LOOP -1024, 32, -1, dda_q2_end0, dda_q2_end1, dda_q2_end2, dda_q2_s9x, dda_q2_s9xz, dda_q2_s9yz, dda_q2_s9d
    ASM_DDA_LOOP -1024, 32, -1, dda_q2_end0, dda_q2_end1, dda_q2_end2, dda_q2_s10x, dda_q2_s10xz, dda_q2_s10yz, dda_q2_s10d
    ASM_DDA_LOOP -1024, 32, -1, dda_q2_end0, dda_q2_end1, dda_q2_end2, dda_q2_s11x, dda_q2_s11xz, dda_q2_s11yz, dda_q2_s11d
    ASM_DDA_LOOP -1024, 32, -1, dda_q2_end0, dda_q2_end1, dda_q2_end2, dda_q2_s12x, dda_q2_s12xz, dda_q2_s12yz, dda_q2_s12d
    ASM_DDA_LOOP -1024, 32, -1, dda_q2_end0, dda_q2_end1, dda_q2_end2, dda_q2_s13x, dda_q2_s13xz, dda_q2_s13yz, dda_q2_s13d
    ASM_DDA_LOOP -1024, 32, -1, dda_q2_end0, dda_q2_end1, dda_q2_end2, dda_q2_s14x, dda_q2_s14xz, dda_q2_s14yz, dda_q2_s14d
    ASM_DDA_LOOP -1024, 32, -1, dda_q2_end0, dda_q2_end1, dda_q2_end2, dda_q2_s15x, dda_q2_s15xz, dda_q2_s15yz, dda_q2_s15d
    ld a, 3            ; ran off the end, no hit (shouldn't happen if VD steps are enough)
    jp dda_q2_exit
dda_q2_end0:
    ld a, 0            ; x hit
    jp dda_q2_exit
dda_q2_end1:
    ld a, RBP1         ; y hit
    jp dda_q2_exit
dda_q2_end2:
    ld a, RBP1X2       ; z hit
dda_q2_exit:
    ld (dda_tmaxx), hl  ; write back tmax state (hl/bc/de are correct here regardless
    ld (dda_tmaxy), bc  ; of which path got us to this label -- see macro trace)
    ld (dda_tmaxz), de
    ld (dda_currblock), ix
    pop ix              ; restore caller's ix
    ret

dda_q3:
    ld hl, (dda_tmaxx)     ; tmaxx = current tmax (NOT reset to delta -- see fix note above)
    ASM_DDA_LOOP 1024, 32, -1, dda_q3_end0, dda_q3_end1, dda_q3_end2, dda_q3_s1x, dda_q3_s1xz, dda_q3_s1yz, dda_q3_s1d
    ASM_DDA_LOOP 1024, 32, -1, dda_q3_end0, dda_q3_end1, dda_q3_end2, dda_q3_s2x, dda_q3_s2xz, dda_q3_s2yz, dda_q3_s2d
    ASM_DDA_LOOP 1024, 32, -1, dda_q3_end0, dda_q3_end1, dda_q3_end2, dda_q3_s3x, dda_q3_s3xz, dda_q3_s3yz, dda_q3_s3d
    ASM_DDA_LOOP 1024, 32, -1, dda_q3_end0, dda_q3_end1, dda_q3_end2, dda_q3_s4x, dda_q3_s4xz, dda_q3_s4yz, dda_q3_s4d
    ASM_DDA_LOOP 1024, 32, -1, dda_q3_end0, dda_q3_end1, dda_q3_end2, dda_q3_s5x, dda_q3_s5xz, dda_q3_s5yz, dda_q3_s5d
    ASM_DDA_LOOP 1024, 32, -1, dda_q3_end0, dda_q3_end1, dda_q3_end2, dda_q3_s6x, dda_q3_s6xz, dda_q3_s6yz, dda_q3_s6d
    ASM_DDA_LOOP 1024, 32, -1, dda_q3_end0, dda_q3_end1, dda_q3_end2, dda_q3_s7x, dda_q3_s7xz, dda_q3_s7yz, dda_q3_s7d
    ASM_DDA_LOOP 1024, 32, -1, dda_q3_end0, dda_q3_end1, dda_q3_end2, dda_q3_s8x, dda_q3_s8xz, dda_q3_s8yz, dda_q3_s8d
    ASM_DDA_LOOP 1024, 32, -1, dda_q3_end0, dda_q3_end1, dda_q3_end2, dda_q3_s9x, dda_q3_s9xz, dda_q3_s9yz, dda_q3_s9d
    ASM_DDA_LOOP 1024, 32, -1, dda_q3_end0, dda_q3_end1, dda_q3_end2, dda_q3_s10x, dda_q3_s10xz, dda_q3_s10yz, dda_q3_s10d
    ASM_DDA_LOOP 1024, 32, -1, dda_q3_end0, dda_q3_end1, dda_q3_end2, dda_q3_s11x, dda_q3_s11xz, dda_q3_s11yz, dda_q3_s11d
    ASM_DDA_LOOP 1024, 32, -1, dda_q3_end0, dda_q3_end1, dda_q3_end2, dda_q3_s12x, dda_q3_s12xz, dda_q3_s12yz, dda_q3_s12d
    ASM_DDA_LOOP 1024, 32, -1, dda_q3_end0, dda_q3_end1, dda_q3_end2, dda_q3_s13x, dda_q3_s13xz, dda_q3_s13yz, dda_q3_s13d
    ASM_DDA_LOOP 1024, 32, -1, dda_q3_end0, dda_q3_end1, dda_q3_end2, dda_q3_s14x, dda_q3_s14xz, dda_q3_s14yz, dda_q3_s14d
    ASM_DDA_LOOP 1024, 32, -1, dda_q3_end0, dda_q3_end1, dda_q3_end2, dda_q3_s15x, dda_q3_s15xz, dda_q3_s15yz, dda_q3_s15d
    ld a, 3            ; ran off the end, no hit (shouldn't happen if VD steps are enough)
    jp dda_q3_exit
dda_q3_end0:
    ld a, 0            ; x hit
    jp dda_q3_exit
dda_q3_end1:
    ld a, RBP1         ; y hit
    jp dda_q3_exit
dda_q3_end2:
    ld a, RBP1X2       ; z hit
dda_q3_exit:
    ld (dda_tmaxx), hl  ; write back tmax state (hl/bc/de are correct here regardless
    ld (dda_tmaxy), bc  ; of which path got us to this label -- see macro trace)
    ld (dda_tmaxz), de
    ld (dda_currblock), ix
    pop ix              ; restore caller's ix
    ret

dda_q4:
     ld hl, (dda_tmaxx)     ; tmaxx = current tmax (NOT reset to delta -- see fix note above)
    ASM_DDA_LOOP -1024, -32, 1, dda_q4_end0, dda_q4_end1, dda_q4_end2, dda_q4_s1x, dda_q4_s1xz, dda_q4_s1yz, dda_q4_s1d
    ASM_DDA_LOOP -1024, -32, 1, dda_q4_end0, dda_q4_end1, dda_q4_end2, dda_q4_s2x, dda_q4_s2xz, dda_q4_s2yz, dda_q4_s2d
    ASM_DDA_LOOP -1024, -32, 1, dda_q4_end0, dda_q4_end1, dda_q4_end2, dda_q4_s3x, dda_q4_s3xz, dda_q4_s3yz, dda_q4_s3d
    ASM_DDA_LOOP -1024, -32, 1, dda_q4_end0, dda_q4_end1, dda_q4_end2, dda_q4_s4x, dda_q4_s4xz, dda_q4_s4yz, dda_q4_s4d
    ASM_DDA_LOOP -1024, -32, 1, dda_q4_end0, dda_q4_end1, dda_q4_end2, dda_q4_s5x, dda_q4_s5xz, dda_q4_s5yz, dda_q4_s5d
    ASM_DDA_LOOP -1024, -32, 1, dda_q4_end0, dda_q4_end1, dda_q4_end2, dda_q4_s6x, dda_q4_s6xz, dda_q4_s6yz, dda_q4_s6d
    ASM_DDA_LOOP -1024, -32, 1, dda_q4_end0, dda_q4_end1, dda_q4_end2, dda_q4_s7x, dda_q4_s7xz, dda_q4_s7yz, dda_q4_s7d
    ASM_DDA_LOOP -1024, -32, 1, dda_q4_end0, dda_q4_end1, dda_q4_end2, dda_q4_s8x, dda_q4_s8xz, dda_q4_s8yz, dda_q4_s8d
    ASM_DDA_LOOP -1024, -32, 1, dda_q4_end0, dda_q4_end1, dda_q4_end2, dda_q4_s9x, dda_q4_s9xz, dda_q4_s9yz, dda_q4_s9d
    ASM_DDA_LOOP -1024, -32, 1, dda_q4_end0, dda_q4_end1, dda_q4_end2, dda_q4_s10x, dda_q4_s10xz, dda_q4_s10yz, dda_q4_s10d
    ASM_DDA_LOOP -1024, -32, 1, dda_q4_end0, dda_q4_end1, dda_q4_end2, dda_q4_s11x, dda_q4_s11xz, dda_q4_s11yz, dda_q4_s11d
    ASM_DDA_LOOP -1024, -32, 1, dda_q4_end0, dda_q4_end1, dda_q4_end2, dda_q4_s12x, dda_q4_s12xz, dda_q4_s12yz, dda_q4_s12d
    ASM_DDA_LOOP -1024, -32, 1, dda_q4_end0, dda_q4_end1, dda_q4_end2, dda_q4_s13x, dda_q4_s13xz, dda_q4_s13yz, dda_q4_s13d
    ASM_DDA_LOOP -1024, -32, 1, dda_q4_end0, dda_q4_end1, dda_q4_end2, dda_q4_s14x, dda_q4_s14xz, dda_q4_s14yz, dda_q4_s14d
    ASM_DDA_LOOP -1024, -32, 1, dda_q4_end0, dda_q4_end1, dda_q4_end2, dda_q4_s15x, dda_q4_s15xz, dda_q4_s15yz, dda_q4_s15d
    ld a, 3            ; ran off the end, no hit (shouldn't happen if VD steps are enough)
    jp dda_q4_exit
dda_q4_end0:
    ld a, 0            ; x hit
    jp dda_q4_exit
dda_q4_end1:
    ld a, RBP1         ; y hit
    jp dda_q4_exit
dda_q4_end2:
    ld a, RBP1X2       ; z hit
dda_q4_exit:
    ld (dda_tmaxx), hl  ; write back tmax state (hl/bc/de are correct here regardless
    ld (dda_tmaxy), bc  ; of which path got us to this label -- see macro trace)
    ld (dda_tmaxz), de
    ld (dda_currblock), ix
    pop ix              ; restore caller's ix
    ret

dda_q5:
 ld hl, (dda_tmaxx)     ; tmaxx = current tmax (NOT reset to delta -- see fix note above)
    ASM_DDA_LOOP 1024, -32, 1, dda_q5_end0, dda_q5_end1, dda_q5_end2, dda_q5_s1x, dda_q5_s1xz, dda_q5_s1yz, dda_q5_s1d
    ASM_DDA_LOOP 1024, -32, 1, dda_q5_end0, dda_q5_end1, dda_q5_end2, dda_q5_s2x, dda_q5_s2xz, dda_q5_s2yz, dda_q5_s2d
    ASM_DDA_LOOP 1024, -32, 1, dda_q5_end0, dda_q5_end1, dda_q5_end2, dda_q5_s3x, dda_q5_s3xz, dda_q5_s3yz, dda_q5_s3d
    ASM_DDA_LOOP 1024, -32, 1, dda_q5_end0, dda_q5_end1, dda_q5_end2, dda_q5_s4x, dda_q5_s4xz, dda_q5_s4yz, dda_q5_s4d
    ASM_DDA_LOOP 1024, -32, 1, dda_q5_end0, dda_q5_end1, dda_q5_end2, dda_q5_s5x, dda_q5_s5xz, dda_q5_s5yz, dda_q5_s5d
    ASM_DDA_LOOP 1024, -32, 1, dda_q5_end0, dda_q5_end1, dda_q5_end2, dda_q5_s6x, dda_q5_s6xz, dda_q5_s6yz, dda_q5_s6d
    ASM_DDA_LOOP 1024, -32, 1, dda_q5_end0, dda_q5_end1, dda_q5_end2, dda_q5_s7x, dda_q5_s7xz, dda_q5_s7yz, dda_q5_s7d
    ASM_DDA_LOOP 1024, -32, 1, dda_q5_end0, dda_q5_end1, dda_q5_end2, dda_q5_s8x, dda_q5_s8xz, dda_q5_s8yz, dda_q5_s8d
    ASM_DDA_LOOP 1024, -32, 1, dda_q5_end0, dda_q5_end1, dda_q5_end2, dda_q5_s9x, dda_q5_s9xz, dda_q5_s9yz, dda_q5_s9d
    ASM_DDA_LOOP 1024, -32, 1, dda_q5_end0, dda_q5_end1, dda_q5_end2, dda_q5_s10x, dda_q5_s10xz, dda_q5_s10yz, dda_q5_s10d
    ASM_DDA_LOOP 1024, -32, 1, dda_q5_end0, dda_q5_end1, dda_q5_end2, dda_q5_s11x, dda_q5_s11xz, dda_q5_s11yz, dda_q5_s11d
    ASM_DDA_LOOP 1024, -32, 1, dda_q5_end0, dda_q5_end1, dda_q5_end2, dda_q5_s12x, dda_q5_s12xz, dda_q5_s12yz, dda_q5_s12d
    ASM_DDA_LOOP 1024, -32, 1, dda_q5_end0, dda_q5_end1, dda_q5_end2, dda_q5_s13x, dda_q5_s13xz, dda_q5_s13yz, dda_q5_s13d
    ASM_DDA_LOOP 1024, -32, 1, dda_q5_end0, dda_q5_end1, dda_q5_end2, dda_q5_s14x, dda_q5_s14xz, dda_q5_s14yz, dda_q5_s14d
    ASM_DDA_LOOP 1024, -32, 1, dda_q5_end0, dda_q5_end1, dda_q5_end2, dda_q5_s15x, dda_q5_s15xz, dda_q5_s15yz, dda_q5_s15d
    ld a, 3            ; ran off the end, no hit (shouldn't happen if VD steps are enough)
    jp dda_q5_exit
dda_q5_end0:
    ld a, 0            ; x hit
    jp dda_q5_exit
dda_q5_end1:
    ld a, RBP1         ; y hit
    jp dda_q5_exit
dda_q5_end2:
    ld a, RBP1X2       ; z hit
dda_q5_exit:
    ld (dda_tmaxx), hl  ; write back tmax state (hl/bc/de are correct here regardless
    ld (dda_tmaxy), bc  ; of which path got us to this label -- see macro trace)
    ld (dda_tmaxz), de
    ld (dda_currblock), ix
    pop ix              ; restore caller's ix
    ret

dda_q6:
 ld hl, (dda_tmaxx)     ; tmaxx = current tmax (NOT reset to delta -- see fix note above)
    ASM_DDA_LOOP -1024, 32, 1, dda_q6_end0, dda_q6_end1, dda_q6_end2, dda_q6_s1x, dda_q6_s1xz, dda_q6_s1yz, dda_q6_s1d
    ASM_DDA_LOOP -1024, 32, 1, dda_q6_end0, dda_q6_end1, dda_q6_end2, dda_q6_s2x, dda_q6_s2xz, dda_q6_s2yz, dda_q6_s2d
    ASM_DDA_LOOP -1024, 32, 1, dda_q6_end0, dda_q6_end1, dda_q6_end2, dda_q6_s3x, dda_q6_s3xz, dda_q6_s3yz, dda_q6_s3d
    ASM_DDA_LOOP -1024, 32, 1, dda_q6_end0, dda_q6_end1, dda_q6_end2, dda_q6_s4x, dda_q6_s4xz, dda_q6_s4yz, dda_q6_s4d
    ASM_DDA_LOOP -1024, 32, 1, dda_q6_end0, dda_q6_end1, dda_q6_end2, dda_q6_s5x, dda_q6_s5xz, dda_q6_s5yz, dda_q6_s5d
    ASM_DDA_LOOP -1024, 32, 1, dda_q6_end0, dda_q6_end1, dda_q6_end2, dda_q6_s6x, dda_q6_s6xz, dda_q6_s6yz, dda_q6_s6d
    ASM_DDA_LOOP -1024, 32, 1, dda_q6_end0, dda_q6_end1, dda_q6_end2, dda_q6_s7x, dda_q6_s7xz, dda_q6_s7yz, dda_q6_s7d
    ASM_DDA_LOOP -1024, 32, 1, dda_q6_end0, dda_q6_end1, dda_q6_end2, dda_q6_s8x, dda_q6_s8xz, dda_q6_s8yz, dda_q6_s8d
    ASM_DDA_LOOP -1024, 32, 1, dda_q6_end0, dda_q6_end1, dda_q6_end2, dda_q6_s9x, dda_q6_s9xz, dda_q6_s9yz, dda_q6_s9d
    ASM_DDA_LOOP -1024, 32, 1, dda_q6_end0, dda_q6_end1, dda_q6_end2, dda_q6_s10x, dda_q6_s10xz, dda_q6_s10yz, dda_q6_s10d
    ASM_DDA_LOOP -1024, 32, 1, dda_q6_end0, dda_q6_end1, dda_q6_end2, dda_q6_s11x, dda_q6_s11xz, dda_q6_s11yz, dda_q6_s11d
    ASM_DDA_LOOP -1024, 32, 1, dda_q6_end0, dda_q6_end1, dda_q6_end2, dda_q6_s12x, dda_q6_s12xz, dda_q6_s12yz, dda_q6_s12d
    ASM_DDA_LOOP -1024, 32, 1, dda_q6_end0, dda_q6_end1, dda_q6_end2, dda_q6_s13x, dda_q6_s13xz, dda_q6_s13yz, dda_q6_s13d
    ASM_DDA_LOOP -1024, 32, 1, dda_q6_end0, dda_q6_end1, dda_q6_end2, dda_q6_s14x, dda_q6_s14xz, dda_q6_s14yz, dda_q6_s14d
    ASM_DDA_LOOP -1024, 32, 1, dda_q6_end0, dda_q6_end1, dda_q6_end2, dda_q6_s15x, dda_q6_s15xz, dda_q6_s15yz, dda_q6_s15d
    ld a, 3            ; ran off the end, no hit (shouldn't happen if VD steps are enough)
    jp dda_q6_exit
dda_q6_end0:
    ld a, 0            ; x hit
    jp dda_q6_exit
dda_q6_end1:
    ld a, RBP1         ; y hit
    jp dda_q6_exit
dda_q6_end2:
    ld a, RBP1X2       ; z hit
dda_q6_exit:
    ld (dda_tmaxx), hl  ; write back tmax state (hl/bc/de are correct here regardless
    ld (dda_tmaxy), bc  ; of which path got us to this label -- see macro trace)
    ld (dda_tmaxz), de
    ld (dda_currblock), ix
    pop ix              ; restore caller's ix
    ret

dda_q7:
 ld hl, (dda_tmaxx)     ; tmaxx = current tmax (NOT reset to delta -- see fix note above)
    ASM_DDA_LOOP 1024, 32, 1, dda_q7_end0, dda_q7_end1, dda_q7_end2, dda_q7_s1x, dda_q7_s1xz, dda_q7_s1yz, dda_q7_s1d
    ASM_DDA_LOOP 1024, 32, 1, dda_q7_end0, dda_q7_end1, dda_q7_end2, dda_q7_s2x, dda_q7_s2xz, dda_q7_s2yz, dda_q7_s2d
    ASM_DDA_LOOP 1024, 32, 1, dda_q7_end0, dda_q7_end1, dda_q7_end2, dda_q7_s3x, dda_q7_s3xz, dda_q7_s3yz, dda_q7_s3d
    ASM_DDA_LOOP 1024, 32, 1, dda_q7_end0, dda_q7_end1, dda_q7_end2, dda_q7_s4x, dda_q7_s4xz, dda_q7_s4yz, dda_q7_s4d
    ASM_DDA_LOOP 1024, 32, 1, dda_q7_end0, dda_q7_end1, dda_q7_end2, dda_q7_s5x, dda_q7_s5xz, dda_q7_s5yz, dda_q7_s5d
    ASM_DDA_LOOP 1024, 32, 1, dda_q7_end0, dda_q7_end1, dda_q7_end2, dda_q7_s6x, dda_q7_s6xz, dda_q7_s6yz, dda_q7_s6d
    ASM_DDA_LOOP 1024, 32, 1, dda_q7_end0, dda_q7_end1, dda_q7_end2, dda_q7_s7x, dda_q7_s7xz, dda_q7_s7yz, dda_q7_s7d
    ASM_DDA_LOOP 1024, 32, 1, dda_q7_end0, dda_q7_end1, dda_q7_end2, dda_q7_s8x, dda_q7_s8xz, dda_q7_s8yz, dda_q7_s8d
    ASM_DDA_LOOP 1024, 32, 1, dda_q7_end0, dda_q7_end1, dda_q7_end2, dda_q7_s9x, dda_q7_s9xz, dda_q7_s9yz, dda_q7_s9d
    ASM_DDA_LOOP 1024, 32, 1, dda_q7_end0, dda_q7_end1, dda_q7_end2, dda_q7_s10x, dda_q7_s10xz, dda_q7_s10yz, dda_q7_s10d
    ASM_DDA_LOOP 1024, 32, 1, dda_q7_end0, dda_q7_end1, dda_q7_end2, dda_q7_s11x, dda_q7_s11xz, dda_q7_s11yz, dda_q7_s11d
    ASM_DDA_LOOP 1024, 32, 1, dda_q7_end0, dda_q7_end1, dda_q7_end2, dda_q7_s12x, dda_q7_s12xz, dda_q7_s12yz, dda_q7_s12d
    ASM_DDA_LOOP 1024, 32, 1, dda_q7_end0, dda_q7_end1, dda_q7_end2, dda_q7_s13x, dda_q7_s13xz, dda_q7_s13yz, dda_q7_s13d
    ASM_DDA_LOOP 1024, 32, 1, dda_q7_end0, dda_q7_end1, dda_q7_end2, dda_q7_s14x, dda_q7_s14xz, dda_q7_s14yz, dda_q7_s14d
    ASM_DDA_LOOP 1024, 32, 1, dda_q7_end0, dda_q7_end1, dda_q7_end2, dda_q7_s15x, dda_q7_s15xz, dda_q7_s15yz, dda_q7_s15d
    ld a, 3            ; ran off the end, no hit (shouldn't happen if VD steps are enough)
    jp dda_q7_exit
dda_q7_end0:
    ld a, 0            ; x hit
    jp dda_q7_exit
dda_q7_end1:
    ld a, RBP1         ; y hit
    jp dda_q7_exit
dda_q7_end2:
    ld a, RBP1X2       ; z hit
dda_q7_exit:
    ld (dda_tmaxx), hl  ; write back tmax state (hl/bc/de are correct here regardless
    ld (dda_tmaxy), bc  ; of which path got us to this label -- see macro trace)
    ld (dda_tmaxz), de
    ld (dda_currblock), ix
    pop ix              ; restore caller's ix
    ret

; uint8_t ASM_DDA_FULL(uint8_t quadrant)
; caller must set dda_deltax/y/z (fixed per column) and dda_tmaxx/y/z
; (= deltax/y/z at the START of a column; thereafter carried forward
; automatically via this function's write-back) and dda_currblock.
; returns axis-hit stride: 0=x, RBP1=y, RBP1X2=z, 3=miss.
; dda_currblock and dda_tmaxx/y/z are updated in place.


_ASM_DDA_FULL:
    push ix
    ld ix, 0
    add ix, sp
    ld a, (ix+6)          ; quadrant — read BEFORE ix gets reused below
    ld ix, (dda_currblock) ; frame pointer no longer needed past this point

    push iy
    ld iy, dda_jump_table
    ld de, 0
    ld hl, 0
    ld l, a
    ld e, l
    add hl, hl          ; hl = a*2
    add hl, de          ; hl = a*3
    push hl
    pop de
    add iy, de          ; iy = dda_jump_table + a*3
    ld hl, (iy+0)        ; 24-bit load: hl = target address
    pop iy
    ld bc, (dda_tmaxy)     ; tmaxy = current tmax
    ld de, (dda_tmaxz)     ; tmaxz = current tmax

    jp (hl)


dda_jump_table:
    dl dda_q0
    dl dda_q1
    dl dda_q2
    dl dda_q3
    dl dda_q4
    dl dda_q5
    dl dda_q6
    dl dda_q7


_getddadeltax:
    ld hl, dda_deltax
    ret
_getddadeltay:
    ld hl, dda_deltay
    ret
_getddadeltaz:
    ld hl, dda_deltaz
    ret
_getddacurrblock:
    ld hl, dda_currblock
    ret
_getddatmaxx:
    ld hl, dda_tmaxx
    ret
_getddatmaxy:
    ld hl, dda_tmaxy
    ret
_getddatmaxz:
    ld hl, dda_tmaxz
    ret