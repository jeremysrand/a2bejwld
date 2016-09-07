;
;  vbl.s
;  a2bejwld
;
;  Created by Jeremy Rand on 2016-07-22.
;  Copyright © 2016 Jeremy Rand. All rights reserved.
;



    .export _vblWait, _vblWait2c, _vblInit2gs
    .export _vblIRQCallback


    .include "apple2.inc"


RDVBLBAR      :=  $C019


.proc _vblInit2gs
    lda #$fe
    sta compType
.endproc


.proc _vblWait
    lda   compType
@L1:
    cmp   RDVBLBAR
    bpl   @L1
@L2:
    cmp   RDVBLBAR
    bmi   @L2

    rts
.endproc


.proc _vblIRQCallback
    stz vbl2cByte
    rts
.endproc


.proc _vblWait2c
    lda #$ff
    sta vbl2cByte

@L1:
    lda vbl2cByte
    bne @L1

    rts
.endproc


.data

compType:
    .BYTE   $7e

vbl2cByte:
    .BYTE   $00