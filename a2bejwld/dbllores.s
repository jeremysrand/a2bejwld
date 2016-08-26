;
;  dbllores.s
;  a2bejwld
;
;  Created by Jeremy Rand on 2016-07-20.
;  Copyright © 2016 Jeremy Rand. All rights reserved.
;

    .export _showDblLoRes, _clearDblLoRes, _unshowDblLoRes
    .export _mixedTextMode
    
    .export _drawGem, _drawBgSquare
    .export _drawScore, _selectSquare, _starGem

    .export _drawGemAtXY, _starGemAtXY

    .export _explodeGemFrame1, _explodeGemFrame2
    .export _explodeGemFrame3, _explodeGemFrame4
    .export _explodeGemFrame5, _explodeGemFrame6

    .include "apple2.inc"

SETAN3      :=  $C05E
SET80VID    :=  $C00D

LINE1       :=  $400
LINE2       :=  $480
LINE3       :=  $500
LINE4       :=  $580
LINE5       :=  $600
LINE6       :=  $680
LINE7       :=  $700
LINE8       :=  $780
LINE9       :=  $428
LINE10      :=  $4a8
LINE11      :=  $528
LINE12      :=  $5a8
LINE13      :=  $628
LINE14      :=  $6a8
LINE15      :=  $728
LINE16      :=  $7a8
LINE17      :=  $450
LINE18      :=  $4d0
LINE19      :=  $550
LINE20      :=  $5d0
LINE21      :=  $650
LINE22      :=  $6d0
LINE23      :=  $750
LINE24      :=  $7d0


; I get a linker error with this so I am hard coding some ZP addresses instead
;.ZEROPAGE
;line1addr:  .WORD $0
;line2addr:  .WORD $0
;line3addr:  .WORD $0
;gemmask:    .WORD $0

line1addr   := $82
line2addr   := $84
line3addr   := $86
gemmask     := $88

.CODE

.proc _showDblLoRes
    lda #0
    sta WNDLFT
    lda #80
    sta WNDWDTH
    lda #0
    sta WNDTOP
    lda #24
    sta WNDBTM

    lda TXTCLR
    lda MIXCLR
    lda SETAN3
    sta SET80VID
    sta SET80COL
    rts
.endproc


.proc _unshowDblLoRes
    lda TXTSET
    rts
.endproc


.proc _mixedTextMode
    lda MIXSET
    sta LOWSCR
    ldx #40
    lda #$a0
@L1:
    dex
    sta LINE21, X
    sta LINE22, X
    sta LINE23, X
    sta LINE24, X
    cpx #0
    bne @L1

    sta HISCR
    ldx #40
@L2:
    dex
    sta LINE21, X
    sta LINE22, X
    sta LINE23, X
    sta LINE24, X
    cpx #0
    bne @L2

    lda #0
    sta WNDLFT
    lda #80
    sta WNDWDTH
    lda #20
    sta WNDTOP
    lda #24
    sta WNDBTM

    rts
.endproc


.proc _clearDblLoRes
    sta LOWSCR
    ldx #40
@L1:
    dex
    stz LINE1, X
    stz LINE2, X
    stz LINE3, X
    stz LINE4, X
    stz LINE5, X
    stz LINE6, X
    stz LINE7, X
    stz LINE8, X
    stz LINE9, X
    stz LINE10, X
    stz LINE11, X
    stz LINE12, X
    stz LINE13, X
    stz LINE14, X
    stz LINE15, X
    stz LINE16, X
    stz LINE17, X
    stz LINE18, X
    stz LINE19, X
    stz LINE20, X
    stz LINE21, X
    stz LINE22, X
    stz LINE23, X
    stz LINE24, X
    bne @L1

    sta HISCR
    ldx #40
@L2:
    dex
    stz LINE1, X
    stz LINE2, X
    stz LINE3, X
    stz LINE4, X
    stz LINE5, X
    stz LINE6, X
    stz LINE7, X
    stz LINE8, X
    stz LINE9, X
    stz LINE10, X
    stz LINE11, X
    stz LINE12, X
    stz LINE13, X
    stz LINE14, X
    stz LINE15, X
    stz LINE16, X
    stz LINE17, X
    stz LINE18, X
    stz LINE19, X
    stz LINE20, X
    stz LINE21, X
    stz LINE22, X
    stz LINE23, X
    stz LINE24, X
    bne @L2

    rts
.endproc


.proc _drawBgSquare
; A is the square position (from 0 to 63)
; 0 through 7 are on the top row
    tax
    lda bgColor,X
    sta color
    lda bgAuxColor,X
    sta colorAux

    txa
    sta square

    and #7
    sta xPos
    lda square
    lsr
    lsr
    lsr

; Get line addrs
    tax
    lda bgLoLines1,X
    sta line1addr
    lda bgHiLines1,X
    sta line1addr+1
    
    lda bgLoLines2,X
    sta line2addr
    lda bgHiLines2,X
    sta line2addr+1
    
    lda bgLoLines3,X
    sta line3addr
    lda bgHiLines3,X
    sta line3addr+1

; Write the square
    lda xPos
    asl
    asl
    tay
    ldx #4
@L1:
    lda color
    sta LOWSCR
    sta (line1addr),Y
    sta (line2addr),Y
    sta (line3addr),Y

    lda colorAux
    sta HISCR
    sta (line1addr),Y
    sta (line2addr),Y
    sta (line3addr),Y

    iny
    dex
    bne @L1


    rts
; Locals

xPos:       .BYTE $0
square:     .BYTE $0
color:      .BYTE $0
colorAux:   .BYTE $0
.endproc


.proc _drawGemAtXY
    stx xPos
    lsr
    tax
    bcc @L3

    lda maskLoAddrs2,Y
    sta gemmask
    lda maskHiAddrs2,Y
    sta gemmask+1
    bra @L4

@L3:
    lda maskLoAddrs,Y
    sta gemmask
    lda maskHiAddrs,Y
    sta gemmask+1

@L4:
    lda gemColours,Y
    sta gemColour
    lda gemAuxColours,Y
    sta gemAuxColour

    stz isAux
    lda xPos
    lsr
    sta xPos
    bcs @L5
    lda #1
    sta isAux

@L5:
; Get line addrs
    inx
    inx
    lda fakeLineLoAddrs,X
    clc
    adc xPos
    sta line1addr
    lda fakeLineHiAddrs,X
    sta line1addr+1

    inx
    lda fakeLineLoAddrs,X
    clc
    adc xPos
    sta line2addr
    lda fakeLineHiAddrs,X
    sta line2addr+1

    inx
    lda fakeLineLoAddrs,X
    clc
    adc xPos
    sta line3addr
    lda fakeLineHiAddrs,X
    sta line3addr+1

    ; Draw the gem
    ldy #0
    ldx #8
@L1:

    lda isAux
    beq @L6

    sta HISCR
    lda (line1addr)
    and (gemmask),Y
    sta square
    lda (gemmask),Y
    eor #$ff
    and gemAuxColour
    ora square
    sta (line1addr)
    iny

    lda (line2addr)
    and (gemmask),Y
    sta square
    lda (gemmask),Y
    eor #$ff
    and gemAuxColour
    ora square
    sta (line2addr)
    iny

    lda (line3addr)
    and (gemmask),Y
    sta square
    lda (gemmask),Y
    eor #$ff
    and gemAuxColour
    ora square
    sta (line3addr)
    iny

    stz isAux

    bra @L7

@L6:
    sta LOWSCR
    lda (line1addr)
    and (gemmask),Y
    sta square
    lda (gemmask),Y
    eor #$ff
    and gemColour
    ora square
    sta (line1addr)
    iny

    lda (line2addr)
    and (gemmask),Y
    sta square
    lda (gemmask),Y
    eor #$ff
    and gemColour
    ora square
    sta (line2addr)
    iny

    lda (line3addr)
    and (gemmask),Y
    sta square
    lda (gemmask),Y
    eor #$ff
    and gemColour
    ora square
    sta (line3addr)
    iny

    inc line1addr
    inc line2addr
    inc line3addr

    inc isAux

@L7:
    dex
    beq @L2

    jmp @L1
@L2:

    rts
; Locals

xPos:         .BYTE $0
square:       .BYTE $0
gemColour:    .BYTE $0
gemAuxColour: .BYTE $0
isAux:        .BYTE $0
.endproc


.proc _drawGem
; A is the square position (from 0 to 63)
; 0 through 7 are on the top row
    sta square

    and #7
    asl
    asl
    asl
    tax
    lda square
; Need to divide by 8 to get the y square
; and then multiply by 6 to get the y
; position (0-47) on the screen.
    lsr
    lsr
    lsr
    sta square
    asl
    clc
    adc square
    asl
    jmp _drawGemAtXY
; Locals

square:     .BYTE $0
.endproc


.proc _selectSquare
    ldy #0
    jmp _drawGem
.endproc


.proc _starGemAtXY
    stx xPos
    lsr
    tax
    bcc @L1
    lda #$f0
    bra @L2
@L1:
    lda #$0f
@L2:
    sta starVal
    inx

    sta LOWSCR
    lda xPos
    lsr
    tay
    bcc @L3
    sta HISCR
    iny
@L3:
    iny
    iny
    sty xPos

; Get line addrs
    lda lineLoAddrs,X
    clc
    adc xPos
    sta line2addr
    lda lineHiAddrs,X
    sta line2addr+1

    lda starVal
    ora (line2addr)
    sta (line2addr)
    rts

; Locals

xPos:       .BYTE $0
square:     .BYTE $0
starVal:    .BYTE $0

.endproc


.proc _starGem
; A is the square position (from 0 to 63)
; 0 through 7 are on the top row
    sta square

    and #7
    asl
    asl
    asl
    tax
    lda square
; Need to divide by 8 to get the y square
; and then multiply by 3 to get the y
; position (0-23) on the screen.
    lsr
    lsr
    lsr
    sta square
    asl
    clc
    adc square
    asl
    jmp _starGemAtXY

; Locals

xPos:       .BYTE $0
square:     .BYTE $0
.endproc

.proc _drawScore
; A is a number from 0 to 24
    tay
    ldx #24
    lda #$dd
    sta color
    sta LOWSCR
@L1:
    dex
    bmi @L2
    cpy #0
    bne @L3
    lda #$22
    sta color
@L3:
    dey

    lda lineLoAddrs,X
    clc
    adc #39
    sta line1addr
    lda lineHiAddrs,X
    sta line1addr+1

    lda color
    sta (line1addr)

    bra @L1

@L2:
    rts
    
; Locals

color:      .BYTE $0
.endproc
    
.proc _explodeGemFrame1
; A is the square position (from 0 to 63)
; 0 through 7 are on the top row
    sta square

    and #7
    asl
    asl
    sta xPos
    lda square
    lsr
    lsr
    lsr

; Get line addrs
    tax
    lda bgLoLines2,X
    clc
    adc xPos
    sta line2addr
    lda bgHiLines2,X
    sta line2addr+1

    ldy #0
@L1:
    ; Draw the frame
    sta HISCR
    lda (line2addr),Y
    ora #$0f
    sta (line2addr),Y
    sta LOWSCR
    lda (line2addr),Y
    ora #$0f
    sta (line2addr),Y
    iny
    cpy #4
    bne @L1

    rts

; Locals

xPos:       .BYTE $0
square:     .BYTE $0
.endproc


.proc _explodeGemFrame2
; A is the square position (from 0 to 63)
; 0 through 7 are on the top row
    sta square

    and #7
    asl
    asl
    sta xPos
    lda square
    lsr
    lsr
    lsr

; Get line addrs
    tax
    lda bgLoLines2,X
    clc
    adc xPos
    sta line2addr
    lda bgHiLines2,X
    sta line2addr+1

    ; Draw the frame
    lda #$ff
    ldy #0

@L1:
    sta HISCR
    sta (line2addr),Y
    sta LOWSCR
    sta (line2addr),Y

    iny
    cpy #4
    bne @L1

    rts

; Locals

xPos:       .BYTE $0
square:     .BYTE $0
.endproc


.proc _explodeGemFrame3
; A is the square position (from 0 to 63)
; 0 through 7 are on the top row
    sta square

    and #7
    asl
    asl
    sta xPos
    lda square
    lsr
    lsr
    lsr

; Get line addrs
    tax
    lda bgLoLines1,X
    clc
    adc xPos
    sta line1addr
    lda bgHiLines1,X
    sta line1addr+1

    ldy #0
@L1:
    ; Draw the frame
    sta HISCR
    lda (line1addr),Y
    ora #$f0
    sta (line1addr),Y
    sta LOWSCR
    lda (line1addr),Y
    ora #$f0
    sta (line1addr),Y

    iny
    cpy #4
    bne @L1

    rts

; Locals

xPos:       .BYTE $0
square:     .BYTE $0
.endproc


.proc _explodeGemFrame4
; A is the square position (from 0 to 63)
; 0 through 7 are on the top row
    sta square

    and #7
    asl
    asl
    sta xPos
    lda square
    lsr
    lsr
    lsr

; Get line addrs
    tax
    lda bgLoLines3,X
    clc
    adc xPos
    sta line3addr
    lda bgHiLines3,X
    sta line3addr+1

    ldy #0
@L1:
    ; Draw the frame
    sta HISCR
    lda (line3addr),Y
    ora #$0f
    sta (line3addr),Y
    sta LOWSCR
    lda (line3addr),Y
    ora #$0f
    sta (line3addr),Y

    iny
    cpy #4
    bne @L1

    rts

; Locals

xPos:       .BYTE $0
square:     .BYTE $0
.endproc


.proc _explodeGemFrame5
; A is the square position (from 0 to 63)
; 0 through 7 are on the top row
    sta square

    and #7
    asl
    asl
    sta xPos
    lda square
    lsr
    lsr
    lsr

; Get line addrs
    tax
    lda bgLoLines1,X
    clc
    adc xPos
    sta line1addr
    lda bgHiLines1,X
    sta line1addr+1

    ; Draw the frame
    lda #$ff
    ldy #0

@L1:
    sta HISCR
    sta (line1addr),Y
    sta LOWSCR
    sta (line1addr),Y

    iny
    cpy #4
    bne @L1

    rts

; Locals

xPos:       .BYTE $0
square:     .BYTE $0
.endproc


.proc _explodeGemFrame6
; A is the square position (from 0 to 63)
; 0 through 7 are on the top row
    sta square

    and #7
    asl
    asl
    sta xPos
    lda square
    lsr
    lsr
    lsr

; Get line addrs
    tax
    lda bgLoLines3,X
    clc
    adc xPos
    sta line3addr
    lda bgHiLines3,X
    sta line3addr+1

    ; Draw the frame
    lda #$ff
    ldy #0

@L1:
    sta HISCR
    sta (line3addr),Y
    sta LOWSCR
    sta (line3addr),Y

    iny
    cpy #4
    bne @L1

    rts

; Locals

xPos:       .BYTE $0
square:     .BYTE $0
.endproc


.DATA

.align 64

; This block of bytes is used for writing to gems "above" the top of the screen.
; Because we draw gems half off the screen, we have two fake lines above the
; top of the screen which points to this buffer of 40 bytes (one line).
FakeLine:
    .BYTE $0, $0, $0, $0, $0, $0, $0, $0
    .BYTE $0, $0, $0, $0, $0, $0, $0, $0
    .BYTE $0, $0, $0, $0, $0, $0, $0, $0
    .BYTE $0, $0, $0, $0, $0, $0, $0, $0
    .BYTE $0, $0, $0, $0, $0, $0, $0, $0

; Prefix this array with two pointers to "fake lines"
fakeLineLoAddrs:
    .LOBYTES  FakeLine, FakeLine
lineLoAddrs:
    .LOBYTES  LINE1,  LINE2,  LINE3,  LINE4,  LINE5,  LINE6,  LINE7,  LINE8
    .LOBYTES  LINE9, LINE10, LINE11, LINE12, LINE13, LINE14, LINE15, LINE16
    .LOBYTES LINE17, LINE18, LINE19, LINE20, LINE21, LINE22, LINE23, LINE24

; Prefix this array with two pointers to "fake lines"
fakeLineHiAddrs:
    .HIBYTES  FakeLine, FakeLine
lineHiAddrs:
    .HIBYTES  LINE1,  LINE2,  LINE3,  LINE4,  LINE5,  LINE6,  LINE7,  LINE8
    .HIBYTES  LINE9, LINE10, LINE11, LINE12, LINE13, LINE14, LINE15, LINE16
    .HIBYTES LINE17, LINE18, LINE19, LINE20, LINE21, LINE22, LINE23, LINE24

; Index this with (xPos << 3) + yPos
bgColor:
    .BYTE   $0, $55, $0, $55, $0, $55, $0, $55
    .BYTE   $55, $0, $55, $0, $55, $0, $55, $0
    .BYTE   $0, $55, $0, $55, $0, $55, $0, $55
    .BYTE   $55, $0, $55, $0, $55, $0, $55, $0
    .BYTE   $0, $55, $0, $55, $0, $55, $0, $55
    .BYTE   $55, $0, $55, $0, $55, $0, $55, $0
    .BYTE   $0, $55, $0, $55, $0, $55, $0, $55
    .BYTE   $55, $0, $55, $0, $55, $0, $55, $0

; Index this with (xPos << 3) + yPos
bgAuxColor:
    .BYTE   $0, $aa, $0, $aa, $0, $aa, $0, $aa
    .BYTE   $aa, $0, $aa, $0, $aa, $0, $aa, $0
    .BYTE   $0, $aa, $0, $aa, $0, $aa, $0, $aa
    .BYTE   $aa, $0, $aa, $0, $aa, $0, $aa, $0
    .BYTE   $0, $aa, $0, $aa, $0, $aa, $0, $aa
    .BYTE   $aa, $0, $aa, $0, $aa, $0, $aa, $0
    .BYTE   $0, $aa, $0, $aa, $0, $aa, $0, $aa
    .BYTE   $aa, $0, $aa, $0, $aa, $0, $aa, $0

bgLoLines1:
    .LOBYTES LINE1, LINE4, LINE7, LINE10, LINE13, LINE16, LINE19, LINE22
bgLoLines2:
    .LOBYTES LINE2, LINE5, LINE8, LINE11, LINE14, LINE17, LINE20, LINE23
bgLoLines3:
    .LOBYTES LINE3, LINE6, LINE9, LINE12, LINE15, LINE18, LINE21, LINE24

bgHiLines1:
    .HIBYTES LINE1, LINE4, LINE7, LINE10, LINE13, LINE16, LINE19, LINE22
bgHiLines2:
    .HIBYTES LINE2, LINE5, LINE8, LINE11, LINE14, LINE17, LINE20, LINE23
bgHiLines3:
    .HIBYTES LINE3, LINE6, LINE9, LINE12, LINE15, LINE18, LINE21, LINE24


orangeMask:
greenMask:
    .BYTE $ff, $ff, $ff
    .BYTE $ff, $00, $ff
    .BYTE $0f, $00, $f0
    .BYTE $0f, $00, $f0
    .BYTE $0f, $00, $f0
    .BYTE $0f, $00, $f0
    .BYTE $ff, $00, $ff
    .BYTE $ff, $ff, $ff

orangeMask2:
greenMask2:
    .BYTE $ff, $ff, $ff
    .BYTE $ff, $0f, $f0
    .BYTE $ff, $00, $00
    .BYTE $ff, $00, $00
    .BYTE $ff, $00, $00
    .BYTE $ff, $00, $00
    .BYTE $ff, $0f, $f0
    .BYTE $ff, $ff, $ff


greyMask:
purpleMask:
    .BYTE $ff, $ff, $ff
    .BYTE $ff, $0f, $f0
    .BYTE $ff, $00, $f0
    .BYTE $0f, $00, $f0
    .BYTE $0f, $00, $f0
    .BYTE $ff, $00, $f0
    .BYTE $ff, $0f, $f0
    .BYTE $ff, $ff, $ff


greyMask2:
purpleMask2:
    .BYTE $ff, $ff, $ff
    .BYTE $ff, $ff, $00
    .BYTE $ff, $0f, $00
    .BYTE $ff, $00, $00
    .BYTE $ff, $00, $00
    .BYTE $ff, $0f, $00
    .BYTE $ff, $ff, $00
    .BYTE $ff, $ff, $ff


specialMask:
yellowMask:
    .BYTE $ff, $ff, $ff
    .BYTE $ff, $ff, $ff
    .BYTE $ff, $00, $ff
    .BYTE $0f, $00, $f0
    .BYTE $0f, $00, $f0
    .BYTE $ff, $00, $ff
    .BYTE $ff, $ff, $ff
    .BYTE $ff, $ff, $ff


specialMask2:
yellowMask2:
    .BYTE $ff, $ff, $ff
    .BYTE $ff, $ff, $ff
    .BYTE $ff, $0f, $f0
    .BYTE $ff, $00, $00
    .BYTE $ff, $00, $00
    .BYTE $ff, $0f, $f0
    .BYTE $ff, $ff, $ff
    .BYTE $ff, $ff, $ff


blueMask:
    .BYTE $ff, $ff, $ff
    .BYTE $ff, $f0, $ff
    .BYTE $0f, $00, $ff
    .BYTE $0f, $00, $f0
    .BYTE $0f, $00, $f0
    .BYTE $0f, $00, $ff
    .BYTE $ff, $f0, $ff
    .BYTE $ff, $ff, $ff


blueMask2:
    .BYTE $ff, $ff, $ff
    .BYTE $ff, $0f, $ff
    .BYTE $ff, $00, $f0
    .BYTE $ff, $00, $00
    .BYTE $ff, $00, $00
    .BYTE $ff, $00, $f0
    .BYTE $ff, $0f, $ff
    .BYTE $ff, $ff, $ff


redMask:
    .BYTE $ff, $ff, $ff
    .BYTE $0f, $00, $f0
    .BYTE $0f, $00, $f0
    .BYTE $0f, $00, $f0
    .BYTE $0f, $00, $f0
    .BYTE $0f, $00, $f0
    .BYTE $0f, $00, $f0
    .BYTE $ff, $ff, $ff


redMask2:
    .BYTE $ff, $ff, $ff
    .BYTE $ff, $00, $00
    .BYTE $ff, $00, $00
    .BYTE $ff, $00, $00
    .BYTE $ff, $00, $00
    .BYTE $ff, $00, $00
    .BYTE $ff, $00, $00
    .BYTE $ff, $ff, $ff


selectMask:
selectMask2:
    .BYTE $00, $00, $00
    .BYTE $f0, $ff, $0f
    .BYTE $f0, $ff, $0f
    .BYTE $f0, $ff, $0f
    .BYTE $f0, $ff, $0f
    .BYTE $f0, $ff, $0f
    .BYTE $f0, $ff, $0f
    .BYTE $00, $00, $00


; The order of these must match the defines for the gems in types.h.
; I also reuse 0 to mean "select" which isn't a real gem type but I
; draw it like a gem.

maskLoAddrs:
    .LOBYTES selectMask, greenMask, redMask, purpleMask, orangeMask
    .LOBYTES greyMask, yellowMask, blueMask, specialMask

maskHiAddrs:
    .HIBYTES selectMask, greenMask, redMask, purpleMask, orangeMask
    .HIBYTES greyMask, yellowMask, blueMask, specialMask

maskLoAddrs2:
    .LOBYTES selectMask2, greenMask2, redMask2, purpleMask2, orangeMask2
    .LOBYTES greyMask2, yellowMask2, blueMask2, specialMask2

maskHiAddrs2:
    .HIBYTES selectMask2, greenMask2, redMask2, purpleMask2, orangeMask2
    .HIBYTES greyMask2, yellowMask2, blueMask2, specialMask2

gemColours:
    .BYTE $ff   ; select "gem" colour
    .BYTE $cc   ; green gem colour
    .BYTE $11   ; red gem colour
    .BYTE $33   ; purple gem colour
    .BYTE $99   ; orange gem colour
    .BYTE $22   ; grey gem colour
    .BYTE $dd   ; yellow gem colour
    .BYTE $66   ; blue gem colour
    .BYTE $ff   ; special gem colour
gemAuxColours:
    .BYTE $ff   ; select "gem" colour
    .BYTE $66   ; green gem colour
    .BYTE $88   ; red gem colour
    .BYTE $99   ; purple gem colour
    .BYTE $cc   ; orange gem colour
    .BYTE $11   ; grey gem colour
    .BYTE $ee   ; yellow gem colour
    .BYTE $33   ; blue gem colour
    .BYTE $ff   ; special gem colour
