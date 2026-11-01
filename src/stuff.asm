assume adl=1
; =====================================================================
; Stuff.txt  (combined)  -  changes vs. the previous version:
;  8. Fractional player position: ray start tmax = (delta*k)>>4 (fracmul),
;     k = distance to first grid line in 1/16 blocks. TEXMUL uses a per-axis,
;     per-sign texture origin (_fr_of*). C fills _fr_* via update_frac().
;  1. TEXMUL replaces MUL24+TEXFRAC (see 10).
;  2. DDA core is a subroutine (dda_run). The C wrapper _ASM_DDA_FULL
;     still works for passes B/C. The dead tmax/currblock write-backs
;     are gone, so C must NOT read *ddatmax* / *ddacurrblock after a call.
;  3. _raycast_cols_A: pass A's whole column loop (32 columns) in asm.
;  7. The 8 quadrant loops are position independent (jr/djnz only; external jumps are absolute)
;     and can run from hardware-cursor RAM. DDA_CURSOR=1 + C must call init_cursor_dda().
;     (E30800h is OS cursor RAM: DDA_CURSOR=1 crashed in CEmu. Leave it 0.)
;  6. DDA is a COUNTED LOOP (b' = steps left, 15 per ray; a retry after a clear texel
;     keeps counting, so rays can no longer see past 15 steps). C must call
;     save_shadow_bc() / restore_shadow_bc() around the frame (inside di/ei).
;  5. DDA register layout:
;        main set   : hl=tmaxx  iy=tmaxy  de=tmaxz  bc=free temp  a=0
;        shadow set : hl'=current block   de'=scratch (step constant)   b'=steps left
;     ix is FREE inside the loop. On a hit, the dda_hit_* stubs copy hl' into ix
;     and flip back to the main set, so every hit handler still sees ix = block.
;     Handlers must NEVER execute exx (hl'/b' must survive a clear-texel resume).
;     save_shadow_bc/restore_shadow_bc save bc', hl' and de'.
;  9. Decision tree (same axis choice as the old asm, ties included):
;        if tmaxx < tmaxz:  (tmaxx < tmaxy ? X : Y)
;        else:              (tmaxz < tmaxy ? Z : Y)
;     so X and Z appear once and Y appears twice (Y1, Y2).
; 10. Self-modifying deltas. The "ld bc,imm24" operands inside the quadrant
;     blocks are patched instead of loading deltas from memory:
;        DDA_SMC_Y  : both Y copies in ALL 8 blocks, once per row (raycast_cols_A entry),
;                     or the active block per call (ASM_DDA_FULL).
;        DDA_SMC_XZ : X and Z in the ACTIVE block, once per column / per call.
;     _dda_deltax/y/z are STILL stored in memory (texture + slab code read them).
;     Patch addresses are relative to DDA_BASE / dda_entry, so DDA_CURSOR works too.
; 11. TEXMUL (exact, same results as the old one): the top byte of t is not multiplied
;     (always 0: a hit is within 15 steps, so t < ~14500 < 65536), |D| is negated in
;     registers, 4 mlt instead of 5, one origin push/pop. The texel column is patched into the
;     "add a,imm" at tex_col (SMC) instead of going through tex_u / tex_off in memory.
;  4. Floor voxel + water slab handling (ids RBP1..RBP1+4) in asm; they
;     no longer go through the texture path (slab side texture still does,
;     so nothing needs flagging: every textured block can now be see-through).
;
;  C side for pass A (per row, replaces  for(x...){TRACE_COLUMN(0,..)} ):
;     extern "C" { extern int row_sin,row_cos,row_tmaxy,row_w,col_cx,col_cz,DY;
;                  extern uint8_t row_tdybit;  void raycast_cols_A(); }
;     row_sin=sinxztdz.data;  row_cos=cosxztdz.data;
;     row_tmaxy=KMUL(deltay.data,KSEL(fry,tdy.data));
;     dda_deltay=deltay.data;  DY=tdy.data;   <- dda_deltay MUST be set before the call (patched into all 8 blocks)
;     row_tdybit=(tdy.data>0)<<1;  row_w=(int)w;
;     col_cx=(int)&cache[0];  col_cz=(int)&cache[320];
;     raycast_cols_A();
;  C also needs (non-static globals):  flint* inv_table;  (asm: _inv_table)
;  and the _fr_* variables (see update_frac() in main.cpp).
; =====================================================================
extern _colors
extern _ltexturedata
extern _inv_table
    section .bss                ; Tells the eZ80 this is variable RAM
    mm_a: rb 3
    mm_b: rb 3
    mm_t: rb 4
    public scratch1
    public scratch2
    public scratch3 ; where yoou assign color
    public rectaddr
    scratch1:  rb 1             ; Allocates 1 byte (Base address)
    scratch2:  rb 1             ; Automatically equals scratch1 + 1
    scratch3:  rb 1             ; Automatically equals scratch2 + 1
    rectaddr: rb 3
    scratch_row: rb 32
    _DX: rb 3
    _DY: rb 3
    _DZ: rb 3
    public _DX
    public _DY
    public _DZ

    _PX: rb 3
    _PY: rb 3
    _PZ: rb 3
    public _PX
    public _PY
    public _PZ
    
    mulscr: rb 3
    dda_entry: rb 3             ; entry address of the quadrant in use (so a clear texel can resume the DDA)
    public _col_cx
    public _col_cz
    public _row_sin
    public _row_cos
    public _row_tmaxy
    public _row_w
    public _row_tdybit
    _col_cx: rb 3               ; pointer into cache[] (x dir * cos), advances 30 bytes/column
    _col_cz: rb 3               ; pointer into cache[320..] (x dir * sin)
    _row_sin: rb 3              ; sinxztdz.data for this row
    _row_cos: rb 3              ; cosxztdz.data for this row
    _row_tmaxy: rb 3            ; starting tmaxy for this row
    _row_w: rb 3                ; start block pointer (world + player block)
    _row_tdybit: rb 1           ; (tdy>0)<<1
    _col_cnt: rb 1
    quad: rb 1
    fl_tx: rb 3                 ; slab scratch
    fl_tz: rb 3
    shadow_bc_save: rb 3        ; caller's bc' (see _save_shadow_bc)
    shadow_hl_save: rb 3        ; caller's hl'
    shadow_de_save: rb 3        ; caller's de'
    fl_axis: rb 1               ; axis of the current hit: 0=x 1=y 2=z (floor voxel code)

    ; ---- fractional player position (filled by update_frac() in C) ----
    public _fr_kxp
    public _fr_kxn
    public _fr_kzp
    public _fr_kzn
    public _fr_ofxp
    public _fr_ofxn
    public _fr_ofyp
    public _fr_ofyn
    public _fr_ofzp
    public _fr_ofzn
    _fr_kxp: rb 1               ; (16-f)<<4 : ray moving +x
    _fr_kxn: rb 1               ; f<<4      : ray moving -x
    _fr_kzp: rb 1
    _fr_kzn: rb 1
    _fr_ofxp: rb 3              ; texture origin (1024/unit) for D>0 : f*64
    _fr_ofxn: rb 3              ; texture origin for D<0            : (16-f)*64
    _fr_ofyp: rb 3
    _fr_ofyn: rb 3
    _fr_ofzp: rb 3
    _fr_ofzn: rb 3

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
    public _dda_deltax
    public _dda_deltay
    public _dda_deltaz
    public _dda_currblock
    public _getddadeltax 
    public _getddadeltay 
    public _getddadeltaz
    public _getddacurrblock
    _dda_deltax:    rb 3
    _dda_deltay:    rb 3
    _dda_deltaz:    rb 3
    _dda_currblock: rb 3
    public _dda_tmaxx
    public _dda_tmaxy
    public _dda_tmaxz
    _dda_tmaxx: rb 3
    _dda_tmaxy: rb 3
    _dda_tmaxz: rb 3
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
_optomized_raycast_rect_5x5:        ; C entry: colour comes from scratch3, same ABI as before
    ld a, (scratch3)
    push ix
    call draw5x5
    pop ix
    ret

; draw5x5: a = colour. Draws the 5x5 rect at (rectaddr), then rectaddr += 10.
; Clobbers ix, bc, de, hl, a.  Only call it where ix is dead (every DDA exit is).
draw5x5:
    ld (scratch3), a            ; deu of the 3-byte stores below must also hold the colour
    ld ix, (rectaddr)
    ld de, (scratch1)
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
    ld bc, -1270                ; ix = rectaddr+1280  ->  rectaddr+10
    add ix, bc
    ld (rectaddr), ix
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
; tmaxx/tmaxy/tmaxz are persistent state (_dda_tmaxx/y/z) that
; ASM_DDA_FULL reads on entry:
;  (a) retries (a clear texel pixel) resume from where the ray
;      actually was instead of restarting at t=0.
; Caller must set _dda_tmaxx/y/z = starting tmax ONCE at the start
; of a column (KMUL(delta,KSEL(f,d)) in TRACE_COLUMN; fracmul in
; _raycast_cols_A).
; ============================================================

; TEXMUL: a = texel coordinate (0..7) of t*D, sign-corrected.
;   in : hl = t (>= 0 and < 65536: the top byte is assumed 0),  DVAR = 24-bit signed D, |D| < 1024
;        OFFP = texture origin (1024/unit) for D>0  (= f*64, < 1024)
;        OFFN = texture origin for D<0              (= (16-f)*64)
;   out: a.   clobbers bc, de, hl, flags.   LABS = unique label per use.
; P = t*|D| is built from byte products; only P>>8 (mod 2^10) matters (t2 = 0, d2 = 0):
;   P>>8 = hi8(t0*d0) + t1*d0 + t0*d1 + (t1*d1 << 8)
; texel = ((P>>8) + origin) >> 7 & 7.  D<0 -> mirror (xor 7), which needs the origin 1-f
; (that is why OFFN exists). f=0.5 gives the old +0200h.
; Only bits 0..9 of the 16-bit sum matter, so the junk in the upper bytes of
; hl/bc/de after mlt is harmless.
macro TEXMUL DVAR, OFFP, OFFN, LABS
    ld de, (DVAR)
    ld a, (DVAR+2)
    or a, a                 ; clears carry, Z = (D >= 0)
    ld bc, (OFFP)           ; ld leaves flags alone
    jr z, LABS
        ld bc, (OFFN)
        xor a, a            ; de = -de, 16 bit, registers only (only d and e are used below)
        sub a, e
        ld e, a
        ld a, 0             ; ld doesn't touch the borrow
        sbc a, d
        ld d, a
LABS:
    push bc                 ; texture origin
    ld b, h
    ld c, d
    mlt bc                  ; t1*d1
    ld a, c                 ; a = lo(t1*d1)
    ld b, l
    ld c, e
    mlt bc                  ; t0*d0
    ld c, b                 ; c = hi8(t0*d0)
    ld b, a                 ; bc = (lo(t1*d1) << 8) | hi8(t0*d0)
    push bc
    ld b, h
    ld c, e
    mlt bc                  ; t1*d0
    ld e, l                 ; de = d1 : t0   (d0 is not needed any more)
    mlt de                  ; t0*d1
    ld h, b
    ld l, c                 ; hl = t1*d0
    add hl, de              ; + t0*d1
    pop bc
    add hl, bc              ; + (lo(t1*d1) << 8) + hi8(t0*d0)
    pop bc
    add hl, bc              ; + origin
    add hl, hl              ; bits 7..9 of the sum end up in h bits 0..2
    ld a, (DVAR+2)          ; 00h or FFh (|D| < 1024)
    xor a, h
    and a, 7                ; (h & 7) ^ (7 if D < 0)
end macro
DDA_CURSOR equ 1       ; 1 = run the 8 quadrant loops from hardware-cursor RAM (E30800h, call init_cursor_dda() first)
                        ; 0 = leave them where they are (for A/B speed tests)
DDA_SMC_Y  equ 1       ; 1 = patch deltay into both Y "ld bc,imm24" copies (per row / per call)
                        ; 0 = "ld bc,(_dda_deltay)" from memory
DDA_SMC_XZ equ 1       ; 1 = patch deltax/deltaz into the active quadrant (per column / per call)
                        ; 0 = "ld bc,(_dda_deltax/z)" from memory
; DDA_BASE = ENTRY address of quadrant 0 (the 1-byte "exx" pad sits just before it)
QSIZE equ (dda_q1 - dda_q0)   ; every quadrant block is the same size (must stay < 256)
CUR_OFS equ 128
if DDA_CURSOR
    DDA_BASE equ 0E30800h + (dda_q0 - dda_blk_start)+128
else
DDA_BASE equ dda_q0
end if
FAR_T equ 16        ; basically is block cutoff*2 (well average bcs is non euclidean distance)

; displacement (from a quadrant's ENTRY) of the imm24 operand of each patchable "ld bc,imm24".
; (ix+d) wants d <= 127, the assembler will complain if the macro ever grows past that.
SMC_X  equ (dda_q0_ox  + 1 - dda_q0)
SMC_Y1 equ (dda_q0_oy1 + 1 - dda_q0)
SMC_Y2 equ (dda_q0_oy2 + 1 - dda_q0)
SMC_Z  equ (dda_q0_oz  + 1 - dda_q0)
DDA_ITERS equ 15        ; loop passes per ray = 15 / (steps unrolled per pass, currently 1)
RBP1   equ 23   ; reg_block_num+1  (reg_block_num=22 in main.cpp)
RBP1X2 equ 46   ; 2*(reg_block_num+1)
COLORS1 equ _colors
COLORS2 equ _colors+RBP1
COLORS3 equ _colors+RBP1X2
FLOOREND1 equ RBP1+5      ; first id AFTER the floor voxels (= CRAFTTABLE). 5 = FLOORNUM
SLABID    equ RBP1+4      ; WATERSLAB (the half-height one)

macro INVLOOK                   ; hl = d (signed) -> hl = inv_table[d]
    push hl
    add hl, hl
    pop de
    add hl, de                  ; 3*d
    ld de, (_inv_table)
    add hl, de
    ld hl, (hl)
end macro

macro HALF                      ; hl = hl >> 1 (hl >= 0). clobbers hl only
    ld (mulscr), hl
    ld hl, mulscr+2
    srl (hl)
    dec hl                      ; 16-bit dec leaves carry alone
    rr (hl)
    dec hl
    rr (hl)
    ld hl, (mulscr)
end macro

macro BLKSTEP STP               ; runs in the SHADOW set: hl' (block) += STP. de' is scratch.
    if (STP) = 1
        inc hl
    else if (STP) = -1
        dec hl
    else
        ld de, STP
        add hl, de
    end if
end macro

macro LDDELTA LBL, MEM, SMC     ; bc = delta. SMC=1: "ld bc,imm24" with a patchable operand at LBL+1
    if SMC
LBL:
        ld bc, 0
    else
        ld bc, (MEM)
    end if
end macro

macro ASM_DDA_LOOP SX, SY, SZ, ENDX, ENDY, ENDZ, LNX, LY1, LZ, LDONE, OX, OY1, OY2, OZ
; MAIN set  : hl = tmaxx, iy = tmaxy, de = tmaxz, bc = free temp, a = 0 (held for the whole routine)
; SHADOW set: hl' = current block, de' = scratch, b' = steps left
; Entered in the main set. Falls out at LDONE (air) IN THE SHADOW SET; a hit does
; "jp ENDx" (a dda_hit_* stub) also from the shadow set.
;   tmaxx < tmaxz ?  (tmaxx < tmaxy ? X : Y1)  :  (tmaxz < tmaxy ? Z : Y2)
or a
sbc hl, de              ; tmaxx - tmaxz
jr nc, LNX              ; carry = tmaxx < tmaxz
    add hl, de          ; hl = tmaxx
    lea bc, iy+0        ; bc = tmaxy
    or a
    sbc hl, bc          ; tmaxx - tmaxy
    jr nc, LY1          ; tmaxx >= tmaxy -> y step
        add hl, bc      ; ---- x step (tmaxx < tmaxy, tmaxx < tmaxz)
        LDDELTA OX, _dda_deltax, DDA_SMC_XZ
        add hl, bc      ; tmaxx += deltax
        exx
        BLKSTEP SX
        cp a, (hl)
        jr z, LDONE     ; air: keep walking (short jump)
        jp ENDX

LY1:
        add hl, bc      ; ---- y step (tmaxy <= tmaxx < tmaxz); hl = tmaxx again
        LDDELTA OY1, _dda_deltay, DDA_SMC_Y
        add iy, bc      ; tmaxy += deltay
        exx
        BLKSTEP SY
        cp a, (hl)
        jr z, LDONE
        jp ENDY

LNX:                    ; tmaxx >= tmaxz ; hl = tmaxx - tmaxz
    add hl, de          ; hl = tmaxx
    ex de, hl           ; hl = tmaxz, de = tmaxx
    lea bc, iy+0        ; bc = tmaxy
    or a
    sbc hl, bc          ; tmaxz - tmaxy
    jr c, LZ            ; tmaxz < tmaxy -> z step
        add hl, bc      ; ---- y step (tmaxy <= tmaxz <= tmaxx); hl = tmaxz
        ex de, hl       ; hl = tmaxx, de = tmaxz
        LDDELTA OY2, _dda_deltay, DDA_SMC_Y
        add iy, bc      ; tmaxy += deltay
        exx
        BLKSTEP SY
        cp a, (hl)
        jr z, LDONE
        jp ENDY

LZ:
        add hl, bc      ; ---- z step (tmaxz < tmaxy, tmaxz <= tmaxx); hl = tmaxz
        LDDELTA OZ, _dda_deltaz, DDA_SMC_XZ
        add hl, bc      ; tmaxz += deltaz
        ex de, hl       ; hl = tmaxx, de = new tmaxz
        exx
        BLKSTEP SZ
        cp a, (hl)
        jp nz, ENDZ

LDONE:
end macro

; XJ=WY*WZ=1024, YJ=WZ=32 (WX=WY=WZ=32 per main.cpp)
; ------------------------------------------------------------------
; Each quadrant block = [pad: exx] [dda_qN: macro] [djnz pad] [exx] [jp dda_exit]
;   - dda_qN (the ENTRY) is reached in the MAIN set.
;   - After an air step we are in the SHADOW set, djnz decrements b' and jumps to the pad,
;     whose exx brings us back to the main set and falls into dda_qN.
;   - Out of steps: exx back to the main set, then dda_exit.
;   - Hits jump to the dda_hit_* stubs (outside the relocatable block).
; All blocks MUST stay identical in layout (QSIZE) and the whole block < 128 bytes for djnz.
; (current size: about 106 bytes, djnz displacement about -101)
; ------------------------------------------------------------------
dda_blk_start:
dda_q0_pad:
    exx
dda_q0:                 ; entered (main set) with hl = tmaxx, iy = tmaxy, de = tmaxz, a = 0, hl' = block, b' = steps
    ASM_DDA_LOOP -1024, -32, -1, dda_hit_x, dda_hit_y, dda_hit_z, dda_q0_nx, dda_q0_y1, dda_q0_z, dda_q0_done, dda_q0_ox, dda_q0_oy1, dda_q0_oy2, dda_q0_oz
    djnz dda_q0_pad
    exx
    jp dda_exit             ; out of steps: no hit

dda_q1_pad:
    exx
dda_q1:
    ASM_DDA_LOOP 1024, -32, -1, dda_hit_x, dda_hit_y, dda_hit_z, dda_q1_nx, dda_q1_y1, dda_q1_z, dda_q1_done, dda_q1_ox, dda_q1_oy1, dda_q1_oy2, dda_q1_oz
    djnz dda_q1_pad
    exx
    jp dda_exit

dda_q2_pad:
    exx
dda_q2:
    ASM_DDA_LOOP -1024, 32, -1, dda_hit_x, dda_hit_y, dda_hit_z, dda_q2_nx, dda_q2_y1, dda_q2_z, dda_q2_done, dda_q2_ox, dda_q2_oy1, dda_q2_oy2, dda_q2_oz
    djnz dda_q2_pad
    exx
    jp dda_exit

dda_q3_pad:
    exx
dda_q3:
    ASM_DDA_LOOP 1024, 32, -1, dda_hit_x, dda_hit_y, dda_hit_z, dda_q3_nx, dda_q3_y1, dda_q3_z, dda_q3_done, dda_q3_ox, dda_q3_oy1, dda_q3_oy2, dda_q3_oz
    djnz dda_q3_pad
    exx
    jp dda_exit

dda_q4_pad:
    exx
dda_q4:
    ASM_DDA_LOOP -1024, -32, 1, dda_hit_x, dda_hit_y, dda_hit_z, dda_q4_nx, dda_q4_y1, dda_q4_z, dda_q4_done, dda_q4_ox, dda_q4_oy1, dda_q4_oy2, dda_q4_oz
    djnz dda_q4_pad
    exx
    jp dda_exit

dda_q5_pad:
    exx
dda_q5:
    ASM_DDA_LOOP 1024, -32, 1, dda_hit_x, dda_hit_y, dda_hit_z, dda_q5_nx, dda_q5_y1, dda_q5_z, dda_q5_done, dda_q5_ox, dda_q5_oy1, dda_q5_oy2, dda_q5_oz
    djnz dda_q5_pad
    exx
    jp dda_exit

dda_q6_pad:
    exx
dda_q6:
    ASM_DDA_LOOP -1024, 32, 1, dda_hit_x, dda_hit_y, dda_hit_z, dda_q6_nx, dda_q6_y1, dda_q6_z, dda_q6_done, dda_q6_ox, dda_q6_oy1, dda_q6_oy2, dda_q6_oz
    djnz dda_q6_pad
    exx
    jp dda_exit

dda_q7_pad:
    exx
dda_q7:
    ASM_DDA_LOOP 1024, 32, 1, dda_hit_x, dda_hit_y, dda_hit_z, dda_q7_nx, dda_q7_y1, dda_q7_z, dda_q7_done, dda_q7_ox, dda_q7_oy1, dda_q7_oy2, dda_q7_oz
    djnz dda_q7_pad
    exx
    jp dda_exit
dda_blk_end:               ; end of the relocatable block (dda_blk_start .. here)

; Hit stubs (NOT in the relocatable block). We arrive in the SHADOW set with hl' = hit block.
; Copy it to ix and flip to the main set, where hl/iy/de = tmaxx/y/z as the handlers expect.
dda_hit_x:
    push hl
    exx
    pop ix
    jp dda_end_x
dda_hit_y:
    push hl
    exx
    pop ix
    jp dda_end_y
dda_hit_z:
    push hl
    exx
    pop ix
    jp dda_end_z

; patch_y_all: write _dda_deltay into both Y operands of all 8 quadrant blocks.
; Clobbers a, bc, hl, ix.
patch_y_all:
    ld hl, (_dda_deltay)
    ld ix, DDA_BASE
    ld bc, QSIZE
    ld a, 8
patch_y_loop:
    ld (ix + SMC_Y1), hl
    ld (ix + SMC_Y2), hl
    add ix, bc
    dec a
    jr nz, patch_y_loop
    ret

; uint8_t ASM_DDA_FULL(uint8_t quadrant)
; caller must set _dda_deltax/y/z (fixed per column) and _dda_tmaxx/y/z
; (= deltax/y/z at the START of a column; thereafter carried forward
; automatically via this function's write-back) and _dda_currblock.
; returns axis-hit stride: 0=x, RBP1=y, RBP1X2=z, 3=miss.
; _dda_currblock and _dda_tmaxx/y/z are updated in place.
; The active quadrant's SMC delta operands are refreshed from _dda_deltax/y/z on every call.


_ASM_DDA_FULL:
    push ix
    ld ix, 0
    add ix, sp
    ld hl,0
    ld h, QSIZE
    ld l, (ix+6)          ; quadrant — read BEFORE ix gets reused below
    mlt hl                ; quadrant * size of one quadrant block
    ld de, DDA_BASE
    add hl, de
    ld (dda_entry), hl      ; remember the quadrant so a transparent texel can resume the walk
if DDA_SMC_XZ | DDA_SMC_Y
    push hl
    pop ix                  ; ix = entry of the active quadrant (frame pointer no longer needed)
end if
if DDA_SMC_XZ
    ld hl, (_dda_deltax)
    ld (ix + SMC_X), hl
    ld hl, (_dda_deltaz)
    ld (ix + SMC_Z), hl
end if
if DDA_SMC_Y
    ld hl, (_dda_deltay)
    ld (ix + SMC_Y1), hl
    ld (ix + SMC_Y2), hl
end if
    ld ix, (_dda_currblock) ; frame pointer no longer needed past this point
    push iy                 ; iy is callee-saved; the DDA keeps tmaxy in it
    ld iy, (_dda_tmaxy)     ; tmaxy = current tmax
    ld de, (_dda_tmaxz)     ; tmaxz = current tmax
    xor a
    call dda_run
    pop iy
    pop ix
    ret

dda_run:                        ; in: (dda_entry) = quadrant ENTRY address (DDA_BASE + q*QSIZE), ix = start block,
                                ;     iy = tmaxy, de = tmaxz, a = 0, _dda_tmaxx set. returns with ret.
                                ; (hl is NOT an input: the SMC patching in the callers clobbers it)
    push ix
    exx
    pop hl                      ; hl' = block
    ld b, DDA_ITERS             ; step budget lives in b'
    exx
    ld hl, (dda_entry)
    push hl                     ; entry -> dispatch with ret, so hl is free to carry tmaxx
    ld hl, (_dda_tmaxx)
    ret
; ------------------------------------------------------------------
; Block hit handlers.  On entry (from the dda_hit_* stubs):
;   hl = tmaxx, iy = tmaxy, de = tmaxz   (already advanced past the hit face)
;   ix = hit block, a = 0, MAIN register set.  NEVER exx in here.
; Block ids 0..RBP1-1 are solid colours; ids >= RBP1 are textured
; (RBP1 = reg_block_num+1 is the FIRST textured id, so test id >= RBP1).
; ------------------------------------------------------------------


dda_end_x:
    ld a, (ix+0)
    cp a, RBP1
    jp nc, fl_x            ; id >= RBP1: floor voxel or textured
        ld de, 0
        ld e, a
        ld hl, COLORS1
        add hl, de
        ld a, (hl)
        jp draw5x5
txtmthx:
    ; X face: column comes from z (DZ), row comes from y (DY), texture side 0
    push de                 ; always save tmaxz/tmaxx: a clear (26) texel resumes the walk
    push hl
    ld de, (_dda_deltax)
    or a, a
    sbc hl, de              ; hl = t at the moment of the hit (tmaxx was already advanced)
    ld a, h                 ; NEW
    cp a, FAR_T             ; NEW
    jp nc, far_x 
    push hl
    TEXMUL _DZ, _fr_ofzp, _fr_ofzn, tm_1
    ld hl, _DX+2
    bit 7, (hl)
    jr nz, tx_x_noflip      ; ray moving -x: no flip
    xor a, 7                ; ray moving +x: we are looking at the -x face, mirror the column
tx_x_noflip:
    ld (tex_col+1), a
    pop hl
    TEXMUL _DY, _fr_ofyp, _fr_ofyn, tm_2
    ld de, 0                ; side 0 -> byte offset 0
    jp tex_fetch

dda_end_y:
    ld a, (ix+0)
    cp a, RBP1
    jp nc, fl_y            ; id >= RBP1: floor voxel or textured
        ld de, 0
        ld e, a
        ld hl, COLORS2
        add hl, de
        ld a, (hl)
        jp draw5x5
txtmthy:
    ; Y face: column from x (DX), row from z (DZ). side 1 = top, side 3 = bottom
    push de                 ; always save tmaxz/tmaxx: a clear (26) texel resumes the walk
    push hl
    lea hl, iy+0            ; hl = tmaxy
    ld de, (_dda_deltay)
    or a, a
    sbc hl, de              ; hl = t at the hit
    ld a, h                 ; NEW
    cp a, FAR_T             ; NEW
    jp nc, far_y            ; NEW
    push hl
    TEXMUL _DX, _fr_ofxp, _fr_ofxn, tm_3
    ld hl, _DY+2
    bit 7, (hl)
    jr nz, tx_y_noflip      ; ray moving down -> we hit the TOP face
    xor a, 7                ; ray moving up -> BOTTOM face, mirror
tx_y_noflip:
    ld (tex_col+1), a
    pop hl
    TEXMUL _DZ, _fr_ofzp, _fr_ofzn, tm_4
    ld hl, _DY+2
    bit 7, (hl)
    ld de, 3                ; side 1 (top)    -- ld doesn't touch flags
    jp nz, tex_fetch
    ld de, 9                ; side 3 (bottom)
    jp tex_fetch

dda_end_z:
    ld a, (ix+0)
    cp a, RBP1
    jp nc, fl_z            ; id >= RBP1: floor voxel or textured
        ld de, 0
        ld e, a
        ld hl, COLORS3
        add hl, de
        ld a, (hl)
        jp draw5x5
txtmthz:
    ; Z face: column from x (DX), row from y (DY), texture side 2
    push de                 ; always save tmaxz/tmaxx: a clear (26) texel resumes the walk
    push hl
    ex de, hl               ; hl = tmaxz
    ld de, (_dda_deltaz)
    or a, a
    sbc hl, de              ; hl = t at the hit
    ld a, h                 ; NEW
    cp a, FAR_T             ; NEW
    jp nc, far_z            ; NEW
    push hl
    TEXMUL _DX, _fr_ofxp, _fr_ofxn, tm_5
    ld hl, _DZ+2
    bit 7, (hl)
    jr z, tx_z_noflip       ; ray moving +z: no flip
    xor a, 7                ; ray moving -z: mirror
tx_z_noflip:
    ld (tex_col+1), a
    pop hl
    TEXMUL _DY, _fr_ofyp, _fr_ofyn, tm_6
    ld de, 6                ; side 2 -> byte offset 6
    ; falls into tex_fetch

; in:  a = texel row (0-7), de = 3*side, the texel column is already patched into tex_col+1, ix = block
; Looks the colour up in ltexturedata[id*4+side] (an array of 3-byte pointers).
; Clear (26) -> keep walking the ray, otherwise draw the 5x5 rect.
tex_fetch:
    add a, a
    add a, a
    add a, a                ; a = row*8
tex_col:
    add a, 0                ; + column (operand patched by the handler)
    ld bc, 0
    ld c, a                 ; bc = texel offset inside the 8x8 image
tex_lookup:
    ld hl, 0
    ld h, 12
    ld l, (ix+0)
    mlt hl                  ; hl = 12*id  (= id*4 pointers * 3 bytes)
    add hl, de              ; + 3*side
    ld de, (_ltexturedata)  ; ltexturedata is a POINTER VARIABLE: read its value, not its address
    add hl, de
    ld hl, (hl)             ; hl -> the 8x8 texture for this face
    add hl, bc
    ld a, (hl)              ; a = colour
    cp a, 26
    jr z, tex_clear
    pop hl                  ; solid texel: drop the saved state and draw
    pop hl
    jp draw5x5
tex_clear:
    ; clear texel: restore the walk state and keep going from the same quadrant.
    ; dda_qN reloads nothing else: hl' (block) and b' (steps) are still intact in the shadow set.
    pop hl                  ; tmaxx
    pop de                  ; tmaxz
    xor a, a
    push hl
    ld hl, (dda_entry)
    ex (sp), hl             ; hl = tmaxx, top of stack = jump-table slot
    ret                     ; -> jp dda_qN
far_x:
    ld de, 0
    jp tex_far
far_z:
    ld de, 6
    jp tex_far
far_y:
    ld hl, _DY+2
    bit 7, (hl)
    ld de, 3                ; top
    jp nz, tex_far
    ld de, 9                ; bottom
    ; falls into tex_far

; in: de = 3*side, ix = block, stack = tmaxx, tmaxz
tex_far:
    ;lea hl, ix+0            ; hash the block address (stable in world space)
    ld a, (rectaddr)        ; low byte of the screen address (per-pixel grain)
    xor a, l
    xor a, h
    and a, 63               ; texel index 0..63
    ld bc, 0
    ld c, a
    jp tex_lookup
dda_exit:; like if set a to 3. no hit a block
    ld hl,(rectaddr)
    ld bc, 10
    add hl, bc
    ld (rectaddr), hl
realdda_exit:
    ret
; ------------------------------------------------------------------
; Floor voxels (RBP1..FLOOREND1-1) and the water slab (SLABID).
; Entered with a = id, hl/bc/de = tmaxx/y/z (already advanced), ix = block.
; Same rules as the old C code:
;   plain floor : drawn only when looking down (tdy<0) and the ray's next
;                 step is a y step (tmaxy < tmaxx and < tmaxz); otherwise the
;                 ray just keeps walking.
;   slab        : looking down -> same test with tmaxy pulled back half a
;                 block (top of the slab is mid-block); looking up -> drawn
;                 if we entered through the y face (its underside).
;                 anything else falls through to the slab's side texture.
; ------------------------------------------------------------------
fl_x:
    cp a, FLOOREND1
    jp nc, txtmthx
    xor a, a
    ld (fl_axis), a
    jp floor_hit
fl_y:
    cp a, FLOOREND1
    jp nc, txtmthy
    ld a, 1
    ld (fl_axis), a
    jp floor_hit
fl_z:
    cp a, FLOOREND1
    jp nc, txtmthz
    ld a, 2
    ld (fl_axis), a
    ; falls into floor_hit

floor_hit:
    ld a, (ix+0)
    cp a, SLABID
    jr nc, fh_slab
    ld a, (_DY+2)
    bit 7, a
    jr z, fh_continue       ; tdy >= 0: looking up/flat, carpets are invisible
    lea bc, iy+0            ; bc = tmaxy
    call fl_ymin
    jp c, fl_draw
fh_continue:                ; keep walking from the same quadrant (same trick as tex_clear)
    xor a, a
    push hl
    ld hl, (dda_entry)
    ex (sp), hl
    ret

fh_slab:
    ld a, (_DY+2)
    bit 7, a
    jr z, fh_slab_up
    ; looking down: slab top is half a block lower, i.e. tmaxy-half. Compare
    ; tmaxy < tmaxx+half and tmaxy < tmaxz+half instead (same thing, no negation).
    ld (fl_tx), hl
    ld (fl_tz), de
    ld hl, (_dda_deltay)
    HALF                    ; hl = deltay/2
    push hl
    pop bc                  ; bc = half
    ld hl, (fl_tx)
    add hl, bc              ; tmaxx + half
    ex de, hl               ; de = tmaxx+half, hl = tmaxz
    add hl, bc              ; tmaxz + half
    ex de, hl               ; hl = tmaxx+half, de = tmaxz+half
    lea bc, iy+0            ; bc = tmaxy
    call fl_ymin
    ld hl, (fl_tx)          ; ld leaves the carry flag alone
    ld de, (fl_tz)
    jp c, fl_draw
    jp fl_tex
fh_slab_up:
    ld a, (fl_axis)
    cp a, 1
    jp z, fl_draw
    ; falls into fl_tex

fl_tex:                     ; back to the normal textured-block path for this axis
    ld a, (fl_axis)
    or a, a
    jp z, txtmthx
    dec a
    jp z, txtmthy
    jp txtmthz

; ids RBP1..RBP1+4. KEEP IN SYNC with floorcolorsconst[] in main.cpp
floorcolors_tbl:
    db 255, 96, 224, 30, 28

fl_draw:                    ; draw floorcolors[id] as a 5x5 rect
    ld de, 0
    ld a, (ix+0)
    ld e, a
    ld hl, floorcolors_tbl-RBP1   ; same as floorcolors[id] = floorcolorsconst[id-(FLOORVOXELS+1)]
    add hl, de
    ld a, (hl)
    jp draw5x5

; fl_ymin: in hl = tmaxx, de = tmaxz, bc = tmaxy.  carry set <=> tmaxy < tmaxx AND tmaxy < tmaxz.  Preserves hl, bc, de.
fl_ymin:
    push hl
    or a, a
    sbc hl, bc              ; tmaxx - tmaxy
    jr z, fl_ymin_no
    jp m, fl_ymin_no
    push de
    ex de, hl
    or a, a
    sbc hl, bc              ; tmaxz - tmaxy
    pop de
    jr z, fl_ymin_no
    jp m, fl_ymin_no
    pop hl
    scf
    ret
fl_ymin_no:
    pop hl
    or a, a
    ret


; ------------------------------------------------------------------
; fracmul: starting tmax for a fractional player position.
;   in : hl = delta (0..2^19), a = k*16 (16..240)
;   out: hl = (delta*k)>>4 = (delta*a)>>8      clobbers a, bc, de
; bytes: result = hi(d0*a) + d1*a + ((d2*a)<<8)
; (BCU after mlt is never relied on: products go through mulscr.)
; ------------------------------------------------------------------
fracmul:
    ld b, h
    ld c, a
    ld d, a
    ld e, l
    mlt de              ; d = hi(d0*a)
    mlt bc              ; bc = d1*a
    ld hl, 0
    ld l, c
    ld h, b             ; hl = d1*a (clean upper byte)
    ld bc, 0
    ld c, d             ; bc = hi(d0*a)
    add hl, bc
    ret

public _fracmul_c           ; int fracmul_c(int delta, uint8_t k16)
_fracmul_c:
    ld hl, 3
    add hl, sp              ; hl -> first arg (return address is at sp)
    ld de, (hl)             ; de = delta
    inc hl
    inc hl
    inc hl
    ld a, (hl)              ; a = k*16 (second arg, pushed as 3 bytes)
    ex de, hl               ; hl = delta
    jp fracmul              ; fracmul's ret returns to the C caller



; ---------------------------------------------------------------------
; Pass A column loop: 32 columns (x = 0,10,..,310) of one row.
; Needs _dda_deltay (memory) set by C before the call: it is patched into all 8 blocks here.
; ---------------------------------------------------------------------
public _raycast_cols_A          ; void raycast_cols_A()
_raycast_cols_A:
    push ix
    push iy
if DDA_SMC_Y
    call patch_y_all        ; once per row (clobbers a, bc, hl, ix)
end if
    ld a, 32
    ld (_col_cnt), a
colA:
    ; ---- dx = cx - sinxztdz ----
    ld hl, (_col_cx)
    ld de, (hl)
    ld bc, 30                   ; res*2 flints * 3 bytes
    add hl, bc
    ld (_col_cx), hl
    ld hl, (_row_sin)
    ex de, hl
    or a
    sbc hl, de                  ; hl = dx
    ld (_DX), hl
    ld a, (_DX+2)               ; top byte of dx
    rla                         ; carry = dx < 0
    ld a, (_row_tdybit)         ; ld keeps carry
    jr c, qx_np
    inc a                       ; dx >= 0 -> bit 0 set
qx_np:
    ld (quad), a
    INVLOOK                     ; hl is still dx
    ld (_dda_deltax), hl
    ; ---- starting tmaxx = (deltax*k)>>4 ----
    ld a, (quad)
    bit 0, a                    ; set = dx>0
    ld a, (_fr_kxn)             ; ld leaves flags alone
    jr z, fx_k
    ld a, (_fr_kxp)
fx_k:
    call fracmul
    ld (_dda_tmaxx), hl
    ; ---- dz = cz + cosxztdz ----
    ld hl, (_col_cz)
    ld de, (hl)
    ld bc, 30
    add hl, bc
    ld (_col_cz), hl
    ld hl, (_row_cos)
    add hl, de                  ; hl = dz
    ld (_DZ), hl
    ex de, hl                   ; de = dz
    ld hl, 0
    or a
    sbc hl, de                  ; -dz
    jp p, qz_np
    ld a, (quad)
    or a, 4
    ld (quad), a
qz_np:
    ex de, hl                   ; hl = dz
    INVLOOK
    ld (_dda_deltaz), hl
    ; ---- starting tmaxz = (deltaz*k)>>4 ----
    ld a, (quad)
    bit 2, a                    ; set = dz>0
    ld a, (_fr_kzn)
    jr z, fz_k
    ld a, (_fr_kzp)
fz_k:
    call fracmul
    ex de, hl                   ; de = tmaxz
    ; ---- run the DDA ----
    ld iy, (_row_tmaxy)
    push de
    ld hl, 0
    ld a, (quad)
    ld l, a
    ld h, QSIZE
    mlt hl
    ld de, DDA_BASE
    add hl, de
    pop de                      ; de = tmaxz
    ld (dda_entry), hl
if DDA_SMC_XZ
    push hl
    pop ix                      ; ix = entry of the active quadrant
    ld hl, (_dda_deltax)
    ld (ix + SMC_X), hl         ; patch deltax / deltaz into the active block
    ld hl, (_dda_deltaz)
    ld (ix + SMC_Z), hl
end if
    ld ix, (_row_w)             ; dda_run copies this into hl' (ix is clobbered by the hit stubs, so reload every column)
    xor a
    call dda_run                ; (dda_run reloads the entry from dda_entry, hl is free)
    ld a, (_col_cnt)
    dec a
    ld (_col_cnt), a
    jp nz, colA
    pop iy
    pop ix
    ret


public _raycast_cols_B
_raycast_cols_B:
    push ix
    push iy
if DDA_SMC_Y
    call patch_y_all
end if
    ld a, 64
    ld (_col_cnt), a
colB:
    ld hl, (rectaddr)
    ld bc, -320
    add hl, bc
    ld a, (hl)                  ; colour of the pixel above
    ld bc, 2240                 ; (+320*6) - (-320) -> pixel two rects down
    add hl, bc
    cp a, (hl)
    jr nz, colB_trace
    ld (scratch3), a            ; neighbours match: flat fill, rectaddr += 5
    call _optomized_raycast_rect_5x5_no2
    ld hl, (_col_cx)
    ld bc, 15                   ; res flints * 3 bytes
    add hl, bc
    ld (_col_cx), hl
    ld hl, (_col_cz)
    add hl, bc
    ld (_col_cz), hl
    jp colB_next
colB_trace:
    ld hl, (_col_cx)
    ld de, (hl)
    ld bc, 15
    add hl, bc
    ld (_col_cx), hl
    ld hl, (_row_sin)
    ex de, hl
    or a
    sbc hl, de                  ; dx
    ld (_DX), hl
    ld a, (_DX+2)
    rla
    ld a, (_row_tdybit)
    jr c, qxb_np
    inc a
qxb_np:
    ld (quad), a
    INVLOOK
    ld (_dda_deltax), hl
    ld a, (quad)
    bit 0, a
    ld a, (_fr_kxn)
    jr z, fxb_k
    ld a, (_fr_kxp)
fxb_k:
    call fracmul
    ld (_dda_tmaxx), hl
    ld hl, (_col_cz)
    ld de, (hl)
    ld bc, 15
    add hl, bc
    ld (_col_cz), hl
    ld hl, (_row_cos)
    add hl, de                  ; dz
    ld (_DZ), hl
    ex de, hl
    ld hl, 0
    or a
    sbc hl, de
    jp p, qzb_np
    ld a, (quad)
    or a, 4
    ld (quad), a
qzb_np:
    ex de, hl
    INVLOOK
    ld (_dda_deltaz), hl
    ld a, (quad)
    bit 2, a
    ld a, (_fr_kzn)
    jr z, fzb_k
    ld a, (_fr_kzp)
fzb_k:
    call fracmul
    ex de, hl                   ; de = tmaxz
    ld iy, (_row_tmaxy)
    push de
    ld hl, 0
    ld a, (quad)
    ld l, a
    ld h, QSIZE
    mlt hl
    ld de, DDA_BASE
    add hl, de
    pop de
    ld (dda_entry), hl
if DDA_SMC_XZ
    push hl
    pop ix
    ld hl, (_dda_deltax)
    ld (ix + SMC_X), hl
    ld hl, (_dda_deltaz)
    ld (ix + SMC_Z), hl
end if
    ld ix, (_row_w)
    xor a
    call dda_run                ; every exit path advances rectaddr by 10
    ld hl, (rectaddr)
    ld bc, -5                   ; pass B columns are 5 apart, not 10
    add hl, bc
    ld (rectaddr), hl
colB_next:
    ld a, (_col_cnt)
    dec a
    ld (_col_cnt), a
    jp nz, colB
    pop iy
    pop ix
    ret

public _raycast_cols_A2
_raycast_cols_A2:
    push ix
    push iy
    ld a, 32
    ld (_col_cnt), a
colA2:
    ld hl, (rectaddr)
    dec hl
    ld a, (hl)                  ; color = *(rectaddr-1)
    ld bc, 7
    add hl, bc                  ; hl = rectaddr+6 (res+1)
    cp a, (hl)
    jr nz, colA2_trace
    call draw5x5                ; a = colour, rectaddr += 10
    ld hl, (_col_cx)
    ld bc, 30
    add hl, bc
    ld (_col_cx), hl
    ld hl, (_col_cz)
    add hl, bc
    ld (_col_cz), hl
    jp colA2_next
colA2_trace:
    ld hl, (_col_cx)
    ld de, (hl)
    ld bc, 30
    add hl, bc
    ld (_col_cx), hl
    ld hl, (_row_sin)
    ex de, hl
    or a
    sbc hl, de                  ; dx
    ld (_DX), hl
    ld a, (_DX+2)
    rla
    ld a, (_row_tdybit)
    jr c, qxc_np
    inc a
qxc_np:
    ld (quad), a
    INVLOOK
    ld (_dda_deltax), hl
    ld a, (quad)
    bit 0, a
    ld a, (_fr_kxn)
    jr z, fxc_k
    ld a, (_fr_kxp)
fxc_k:
    call fracmul
    ld (_dda_tmaxx), hl
    ld hl, (_col_cz)
    ld de, (hl)
    ld bc, 30
    add hl, bc
    ld (_col_cz), hl
    ld hl, (_row_cos)
    add hl, de                  ; dz
    ld (_DZ), hl
    ex de, hl
    ld hl, 0
    or a
    sbc hl, de
    jp p, qzc_np
    ld a, (quad)
    or a, 4
    ld (quad), a
qzc_np:
    ex de, hl
    INVLOOK
    ld (_dda_deltaz), hl
    ld a, (quad)
    bit 2, a
    ld a, (_fr_kzn)
    jr z, fzc_k
    ld a, (_fr_kzp)
fzc_k:
    call fracmul
    ex de, hl                   ; de = tmaxz
    ld iy, (_row_tmaxy)
    push de
    ld hl, 0
    ld a, (quad)
    ld l, a
    ld h, QSIZE
    mlt hl
    ld de, DDA_BASE
    add hl, de
    pop de
    ld (dda_entry), hl
if DDA_SMC_XZ
    push hl
    pop ix
    ld hl, (_dda_deltax)
    ld (ix + SMC_X), hl
    ld hl, (_dda_deltaz)
    ld (ix + SMC_Z), hl
end if
    ld ix, (_row_w)
    xor a
    call dda_run                ; advances rectaddr by 10 on every exit path
colA2_next:
    ld a, (_col_cnt)
    dec a
    ld (_col_cnt), a
    jp nz, colA2
    pop iy
    pop ix
    ret

; The DDA keeps its step counter in b', the block pointer in hl' and a scratch in de'.
; Call these once per frame, inside di/ei:
public _save_shadow_bc
public _restore_shadow_bc
_save_shadow_bc:
    exx
    ld (shadow_bc_save), bc
    ld (shadow_hl_save), hl
    ld (shadow_de_save), de
    exx
    ret
_restore_shadow_bc:
    exx
    ld bc, (shadow_bc_save)
    ld hl, (shadow_hl_save)
    ld de, (shadow_de_save)
    exx
    ret

; void init_cursor_dda(void): copy the (position-independent) quadrant loops into cursor RAM.
; Call after gfx_Begin(), and again after anything that might reset the LCD (gc_after).
; (copies from dda_blk_start so the 1-byte pad in front of dda_q0 comes along)
; The copy carries zeroed delta operands; they get patched again on the next row / call.
public _init_cursor_dda
_init_cursor_dda:
if DDA_CURSOR
    ld hl, dda_blk_start
    ld de, 0E30800h + CUR_OFS
    ld bc, dda_blk_end - dda_blk_start
    ldir
end if
    ret

_getddadeltax:
    ld hl, _dda_deltax
    ret
_getddadeltay:
    ld hl, _dda_deltay
    ret
_getddadeltaz:
    ld hl, _dda_deltaz
    ret
_getddacurrblock:
    ld hl, _dda_currblock
    ret
_getddatmaxx:
    ld hl, _dda_tmaxx
    ret
_getddatmaxy:
    ld hl, _dda_tmaxy
    ret
_getddatmaxz:
    ld hl, _dda_tmaxz
    ret