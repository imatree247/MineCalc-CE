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
    dda_deltax:    rb 3
    dda_deltay:    rb 3
    dda_deltaz:    rb 3
    dda_currblock: rb 3
    
    
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


