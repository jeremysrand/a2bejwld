;
;  text.s
;  a2bejwld
;
;  Created by Jeremy Rand on 2020-01-22.
;  Copyright © 2020 Jeremy Rand. All rights reserved.
;

.export _videomode
.export _cout

.include "apple2.inc"

.CODE

.proc _cout
    ; This is a thin wrapper around the standard Apple // cout routine.  It switches
    ; the ROM back in, calls cout and then switches the 16k RAM space back in as
    ; expected by the cc65 environment.  Because the standard library routines with
    ; the apple2 cc65 target converts all lowercase to upper case, I don't want to use
    ; printf(), etc.  Instead, I have written my own print routines and at its root is
    ; this cout wrapper.
    
    bit     $C082
    jsr     $FDED
    bit     $C080
    rts
.endproc


.proc _videomode
    ; I grabbed this code from the cc65 library and modified it to work on the 6502
    ; (vs the 65c02 it was written for).  I am trying to get this game to work on an
    ; unenhanced //e which means it must be compiled for the apple2 (vs the apple2enh)
    ; cc65 target.  When that is selected, the videomode() function, among other things
    ; is no longer available.
    ;
    ; So, this comes from the apple2enh library and is slightly modified to avoid 65c02
    ; instructions.

    ; Get and save current videomode flag
    bit     RD80VID
    php

    ; If we are in 80 column mode then the 80 column firmware is
    ; known to be active so we can just print the ctrl-char code
    ; (even if this only means staying in the current videomode)
    bpl     :+
    jsr     _cout
    clc
    bcc     done

    ; If we are in 40 column mode and want to set 40 column mode
    ; then we explicitly do nothing as we neither know about the
    ; current state of the 80 column firmware nor want to fix it
:   cmp     #$11            ; Ctrl-char code for 40 cols
    beq     done
    ; If we are in 40 column mode and want to set 80 column mode
    ; then we first presume the 80 column firmware being already
    ; active and print the ctrl-char code (this causes a garbage
    ; char to be printed on the screen if isn't already active)
    jsr     _cout

    ; If we successfully switched to 80 column mode then the 80
    ; column firmware was in fact already active and we're done
    bit     RD80VID
    bmi     done

    ; The 80 column firmware isn't already active so we need to
    ; initialize it - causing the screen to be cleared and thus
    ; the garbage char printed above to be erased (but for some
    ; reason the cursor horizontal position not to be zeroed)
    lda #0
    sta     CH
        
    ; Initializing the 80 column firmware needs the ROM switched
    ; in, otherwise it would copy the F8 ROM to the LC (@ $CEF4)
    bit     $C082

    ; Initialize 80 column firmware
    jsr     $C300           ; PR#3
        
    ; Switch in LC bank 2 for R/O
    bit     $C080

    ; Return ctrl-char code for setting previous
    ; videomode using the saved videomode flag
done:
    lda     #$11            ; Ctrl-char code for 40 cols
    plp
    bpl     :+
    lda     #$12            ; Ctrl-char code for 80 cols
:   rts                     ; X was preserved all the way
.endproc
