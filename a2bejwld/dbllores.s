;
;  dbllores.s
;  a2bejwld
;
;  Created by Jeremy Rand on 2016-07-20.
;  Copyright © 2016 Jeremy Rand. All rights reserved.
;

    .export _showDblLoRes, _clearDblLoRes, _unshowDblLoRes
    .export _mixedTextMode
    
    .export _drawGreenGem, _drawPurpleGem, _drawYellowGem
    .export _drawBlueGem, _drawRedGem, _drawGreyGem
    .export _drawOrangeGem, _drawSpecialGem, _drawBgSquare
    .export _drawScore, _selectSquare, _starGem

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
; .ZEROPAGE
; line1addr:  .WORD $0
; line2addr:  .WORD $0
; line3addr:  .WORD $0
line1addr   := $82
line2addr   := $84
line3addr   := $86
gemaddr     := $88
gemmask     := $8A


.proc _showDblLoRes
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
    lda color
    ldx #4
@L1:
    sta LOWSCR
    sta (line1addr),Y
    sta (line2addr),Y
    sta (line3addr),Y

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
.endproc


.proc _drawGem
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

    lda bgLoLines2,X
    clc
    adc xPos
    sta line2addr
    lda bgHiLines2,X
    sta line2addr+1

    lda bgLoLines3,X
    clc
    adc xPos
    sta line3addr
    lda bgHiLines3,X
    sta line3addr+1

    ; Draw the gem
    ldy #0
    ldx #4
@L1:
    sta HISCR
    lda (line1addr)
    sta square
    lda (gemmask),Y
    and square
    sta square
    lda (gemaddr),Y
    ora square
    sta (line1addr)
    iny

    lda (line2addr)
    sta square
    lda (gemmask),Y
    and square
    sta square
    lda (gemaddr),Y
    ora square
    sta (line2addr)
    iny

    lda (line3addr)
    sta square
    lda (gemmask),Y
    and square
    sta square
    lda (gemaddr),Y
    ora square
    sta (line3addr)
    iny

    sta LOWSCR
    lda (line1addr)
    sta square
    lda (gemmask),Y
    and square
    sta square
    lda (gemaddr),Y
    ora square
    sta (line1addr)
    iny

    lda (line2addr)
    sta square
    lda (gemmask),Y
    and square
    sta square
    lda (gemaddr),Y
    ora square
    sta (line2addr)
    iny

    lda (line3addr)
    sta square
    lda (gemmask),Y
    and square
    sta square
    lda (gemaddr),Y
    ora square
    sta (line3addr)
    iny

    dex
    beq @L2

    inc line1addr
    inc line2addr
    inc line3addr

    jmp @L1
@L2:

    rts
; Locals

xPos:       .BYTE $0
square:     .BYTE $0
.endproc

.proc _drawGreenGem
    ldx #<greenGem
    stx gemaddr
    ldx #>greenGem
    stx gemaddr+1
    ldx #<greenMask
    stx gemmask
    ldx #>greenMask
    stx gemmask+1
    jmp _drawGem
.endproc

.proc _drawPurpleGem
    ldx #<purpleGem
    stx gemaddr
    ldx #>purpleGem
    stx gemaddr+1
    ldx #<purpleMask
    stx gemmask
    ldx #>purpleMask
    stx gemmask+1
    jmp _drawGem
.endproc

.proc _drawYellowGem
    ldx #<yellowGem
    stx gemaddr
    ldx #>yellowGem
    stx gemaddr+1
    ldx #<yellowMask
    stx gemmask
    ldx #>yellowMask
    stx gemmask+1
    jmp _drawGem
.endproc

.proc _drawBlueGem
    ldx #<blueGem
    stx gemaddr
    ldx #>blueGem
    stx gemaddr+1
    ldx #<blueMask
    stx gemmask
    ldx #>blueMask
    stx gemmask+1
    jmp _drawGem
.endproc

.proc _drawRedGem
    ldx #<redGem
    stx gemaddr
    ldx #>redGem
    stx gemaddr+1
    ldx #<redMask
    stx gemmask
    ldx #>redMask
    stx gemmask+1
    jmp _drawGem
.endproc

.proc _drawGreyGem
    ldx #<greyGem
    stx gemaddr
    ldx #>greyGem
    stx gemaddr+1
    ldx #<greyMask
    stx gemmask
    ldx #>greyMask
    stx gemmask+1
    jmp _drawGem
.endproc

.proc _drawOrangeGem
    ldx #<orangeGem
    stx gemaddr
    ldx #>orangeGem
    stx gemaddr+1
    ldx #<orangeMask
    stx gemmask
    ldx #>orangeMask
    stx gemmask+1
    jmp _drawGem
.endproc

.proc _drawSpecialGem
    ldx #<specialGem
    stx gemaddr
    ldx #>specialGem
    stx gemaddr+1
    ldx #<specialMask
    stx gemmask
    ldx #>specialMask
    stx gemmask+1
    jmp _drawGem
.endproc

.proc _selectSquare
    ldx #<selectGem
    stx gemaddr
    ldx #>selectGem
    stx gemaddr+1
    ldx #<selectMask
    stx gemmask
    ldx #>selectMask
    stx gemmask+1
    jmp _drawGem
.endproc

.proc _starGem
; A is the square position (from 0 to 63)
; 0 through 7 are on the top row
    sta square

    and #7
    asl
    asl
    inc A
    inc A
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
    
    sta HISCR
    lda #$ff
    sta (line2addr)
    rts

; Locals

xPos:       .BYTE $0
square:     .BYTE $0
.endproc

.proc _drawScore
; A is a number from 0 to 24
    tay
    ldx #24
    lda #$22
    sta color
    sta LOWSCR
@L1:
    dex
    bmi @L2
    cpy #0
    bne @L3
    lda #$aa
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

    ; Draw the frame
    sta HISCR
    lda (line2addr)
    ora #$0f
    sta (line2addr)
    sta LOWSCR
    lda (line2addr)
    ora #$0f
    sta (line2addr)
    inc line2addr

    sta HISCR
    lda (line2addr)
    ora #$0f
    sta (line2addr)
    sta LOWSCR
    lda (line2addr)
    ora #$0f
    sta (line2addr)
    inc line2addr

    sta HISCR
    lda (line2addr)
    ora #$0f
    sta (line2addr)
    sta LOWSCR
    lda (line2addr)
    ora #$0f
    sta (line2addr)
    inc line2addr

    sta HISCR
    lda (line2addr)
    ora #$0f
    sta (line2addr)
    sta LOWSCR
    lda (line2addr)
    ora #$0f
    sta (line2addr)

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

    sta HISCR
    sta (line2addr)
    sta LOWSCR
    sta (line2addr)
    inc line2addr

    sta HISCR
    sta (line2addr)
    sta LOWSCR
    sta (line2addr)
    inc line2addr

    sta HISCR
    sta (line2addr)
    sta LOWSCR
    sta (line2addr)
    inc line2addr

    sta HISCR
    sta (line2addr)
    sta LOWSCR
    sta (line2addr)

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

    ; Draw the frame
    sta HISCR
    lda (line1addr)
    ora #$f0
    sta (line1addr)
    sta LOWSCR
    lda (line1addr)
    ora #$f0
    sta (line1addr)
    inc line1addr

    sta HISCR
    lda (line1addr)
    ora #$f0
    sta (line1addr)
    sta LOWSCR
    lda (line1addr)
    ora #$f0
    sta (line1addr)
    inc line1addr

    sta HISCR
    lda (line1addr)
    ora #$f0
    sta (line1addr)
    sta LOWSCR
    lda (line1addr)
    ora #$f0
    sta (line1addr)
    inc line1addr

    sta HISCR
    lda (line1addr)
    ora #$f0
    sta (line1addr)
    sta LOWSCR
    lda (line1addr)
    ora #$f0
    sta (line1addr)

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

    ; Draw the frame
    sta HISCR
    lda (line3addr)
    ora #$0f
    sta (line3addr)
    sta LOWSCR
    lda (line3addr)
    ora #$0f
    sta (line3addr)
    inc line3addr

    sta HISCR
    lda (line3addr)
    ora #$0f
    sta (line3addr)
    sta LOWSCR
    lda (line3addr)
    ora #$0f
    sta (line3addr)
    inc line3addr

    sta HISCR
    lda (line3addr)
    ora #$0f
    sta (line3addr)
    sta LOWSCR
    lda (line3addr)
    ora #$0f
    sta (line3addr)
    inc line3addr

    sta HISCR
    lda (line3addr)
    ora #$0f
    sta (line3addr)
    sta LOWSCR
    lda (line3addr)
    ora #$0f
    sta (line3addr)

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

    sta HISCR
    sta (line1addr)
    sta LOWSCR
    sta (line1addr)
    inc line1addr

    sta HISCR
    sta (line1addr)
    sta LOWSCR
    sta (line1addr)
    inc line1addr

    sta HISCR
    sta (line1addr)
    sta LOWSCR
    sta (line1addr)
    inc line1addr

    sta HISCR
    sta (line1addr)
    sta LOWSCR
    sta (line1addr)

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

    sta HISCR
    sta (line3addr)
    sta LOWSCR
    sta (line3addr)
    inc line3addr

    sta HISCR
    sta (line3addr)
    sta LOWSCR
    sta (line3addr)
    inc line3addr

    sta HISCR
    sta (line3addr)
    sta LOWSCR
    sta (line3addr)
    inc line3addr

    sta HISCR
    sta (line3addr)
    sta LOWSCR
    sta (line3addr)

    rts

; Locals

xPos:       .BYTE $0
square:     .BYTE $0
.endproc


.DATA
lineAddrs:
    .WORD  LINE1,  LINE2,  LINE3,  LINE4,  LINE5,  LINE6,  LINE7,  LINE8
    .WORD  LINE9, LINE10, LINE11, LINE12, LINE13, LINE14, LINE15, LINE16
    .WORD LINE17, LINE18, LINE19, LINE20, LINE21, LINE22, LINE23, LINE24

lineLoAddrs:
    .LOBYTES  LINE1,  LINE2,  LINE3,  LINE4,  LINE5,  LINE6,  LINE7,  LINE8
    .LOBYTES  LINE9, LINE10, LINE11, LINE12, LINE13, LINE14, LINE15, LINE16
    .LOBYTES LINE17, LINE18, LINE19, LINE20, LINE21, LINE22, LINE23, LINE24

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


greenGem:
    .BYTE $00, $00, $00
    .BYTE $00, $cc, $00
    .BYTE $c0, $cc, $0c
    .BYTE $c0, $cc, $0c
    .BYTE $c0, $cc, $0c
    .BYTE $c0, $cc, $0c
    .BYTE $00, $cc, $00
    .BYTE $00, $00, $00
greenMask:
    .BYTE $ff, $ff, $ff
    .BYTE $ff, $00, $ff
    .BYTE $0f, $00, $f0
    .BYTE $0f, $00, $f0
    .BYTE $0f, $00, $f0
    .BYTE $0f, $00, $f0
    .BYTE $ff, $00, $ff
    .BYTE $ff, $ff, $ff


purpleGem:
    .BYTE $00, $00, $00
    .BYTE $00, $30, $03
    .BYTE $00, $33, $03
    .BYTE $30, $33, $03
    .BYTE $30, $33, $03
    .BYTE $00, $33, $03
    .BYTE $00, $30, $03
    .BYTE $00, $00, $00
purpleMask:
    .BYTE $ff, $ff, $ff
    .BYTE $ff, $0f, $f0
    .BYTE $ff, $00, $f0
    .BYTE $0f, $00, $f0
    .BYTE $0f, $00, $f0
    .BYTE $ff, $00, $f0
    .BYTE $ff, $0f, $f0
    .BYTE $ff, $ff, $ff


yellowGem:
    .BYTE $00, $00, $00
    .BYTE $00, $00, $00
    .BYTE $00, $dd, $00
    .BYTE $d0, $dd, $0d
    .BYTE $d0, $dd, $0d
    .BYTE $00, $dd, $00
    .BYTE $00, $00, $00
    .BYTE $00, $00, $00
yellowMask:
    .BYTE $ff, $ff, $ff
    .BYTE $ff, $ff, $ff
    .BYTE $ff, $00, $ff
    .BYTE $0f, $00, $f0
    .BYTE $0f, $00, $f0
    .BYTE $ff, $00, $ff
    .BYTE $ff, $ff, $ff
    .BYTE $ff, $ff, $ff


blueGem:
    .BYTE $00, $00, $00
    .BYTE $00, $06, $00
    .BYTE $60, $66, $00
    .BYTE $60, $66, $06
    .BYTE $60, $66, $06
    .BYTE $60, $66, $00
    .BYTE $00, $06, $00
    .BYTE $00, $00, $00
blueMask:
    .BYTE $ff, $ff, $ff
    .BYTE $ff, $f0, $ff
    .BYTE $0f, $00, $ff
    .BYTE $0f, $00, $f0
    .BYTE $0f, $00, $f0
    .BYTE $0f, $00, $ff
    .BYTE $ff, $f0, $ff
    .BYTE $ff, $ff, $ff


redGem:
    .BYTE $00, $00, $00
    .BYTE $10, $11, $01
    .BYTE $10, $11, $01
    .BYTE $10, $11, $01
    .BYTE $10, $11, $01
    .BYTE $10, $11, $01
    .BYTE $10, $11, $01
    .BYTE $00, $00, $00
redMask:
    .BYTE $ff, $ff, $ff
    .BYTE $0f, $00, $f0
    .BYTE $0f, $00, $f0
    .BYTE $0f, $00, $f0
    .BYTE $0f, $00, $f0
    .BYTE $0f, $00, $f0
    .BYTE $0f, $00, $f0
    .BYTE $ff, $ff, $ff


greyGem:
    .BYTE $00, $00, $00
    .BYTE $00, $aa, $00
    .BYTE $a0, $aa, $0a
    .BYTE $a0, $aa, $0a
    .BYTE $a0, $aa, $0a
    .BYTE $a0, $aa, $0a
    .BYTE $00, $aa, $00
    .BYTE $00, $00, $00
greyMask:
    .BYTE $ff, $ff, $ff
    .BYTE $ff, $00, $ff
    .BYTE $0f, $00, $f0
    .BYTE $0f, $00, $f0
    .BYTE $0f, $00, $f0
    .BYTE $0f, $00, $f0
    .BYTE $ff, $00, $ff
    .BYTE $ff, $ff, $ff


orangeGem:
    .BYTE $00, $00, $00
    .BYTE $00, $99, $00
    .BYTE $90, $99, $09
    .BYTE $90, $99, $09
    .BYTE $90, $99, $09
    .BYTE $90, $99, $09
    .BYTE $00, $99, $00
    .BYTE $00, $00, $00
orangeMask:
    .BYTE $ff, $ff, $ff
    .BYTE $ff, $00, $ff
    .BYTE $0f, $00, $f0
    .BYTE $0f, $00, $f0
    .BYTE $0f, $00, $f0
    .BYTE $0f, $00, $f0
    .BYTE $ff, $00, $ff
    .BYTE $ff, $ff, $ff


specialGem:
    .BYTE $00, $00, $00
    .BYTE $00, $00, $00
    .BYTE $00, $ff, $00
    .BYTE $f0, $ff, $0f
    .BYTE $f0, $ff, $0f
    .BYTE $00, $ff, $00
    .BYTE $00, $00, $00
    .BYTE $00, $00, $00
specialMask:
    .BYTE $ff, $ff, $ff
    .BYTE $ff, $ff, $ff
    .BYTE $ff, $00, $ff
    .BYTE $0f, $00, $f0
    .BYTE $0f, $00, $f0
    .BYTE $ff, $00, $ff
    .BYTE $ff, $ff, $ff
    .BYTE $ff, $ff, $ff


selectGem:
    .BYTE $ff, $ff, $ff
    .BYTE $0f, $00, $f0
    .BYTE $0f, $00, $f0
    .BYTE $0f, $00, $f0
    .BYTE $0f, $00, $f0
    .BYTE $0f, $00, $f0
    .BYTE $0f, $00, $f0
    .BYTE $ff, $ff, $ff
selectMask:
    .BYTE $00, $00, $00
    .BYTE $f0, $ff, $0f
    .BYTE $f0, $ff, $0f
    .BYTE $f0, $ff, $0f
    .BYTE $f0, $ff, $0f
    .BYTE $f0, $ff, $0f
    .BYTE $f0, $ff, $0f
    .BYTE $00, $00, $00