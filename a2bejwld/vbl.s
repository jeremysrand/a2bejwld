;
;  vbl.s
;  a2bejwld
;
;  Created by Jeremy Rand on 2016-07-22.
;  Copyright © 2016 Jeremy Rand. All rights reserved.
;



    .export _vblWait, _vblWait2c, _vblInit2gs


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


.proc _vblWait2c
    ; TODO - write a routine for VBL detection on //c
    rts
.endproc


.data

compType:
    .BYTE   $7e