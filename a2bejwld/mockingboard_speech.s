;
;  speech.s
;  mocktest
;
;  Created by Jeremy Rand on 2016-09-29.
;  Copyright © 2016 Jeremy Rand. All rights reserved.
;


    .export _mockingBoardSpeechInit, _mockingBoardSpeechShutdown, _mockingBoardSpeakPriv
    .export _mockingBoardSpeechData, _mockingBoardSpeechLen
    .export _mockingBoardSpeechBusy, _mockingBoardSpeechPlaying
    .export _getMockingBoardSlot
    .interruptor mock_irq


TMPPTR  := $FB           ; Temporary pointer used in interrupt handler
IRQL    := $03FE         ; Interrupt vector, low byte
IRQH    := $03FF         ; Interrupt vector, high byte
BASE    := $40           ; First speech chip
DURPHON := BASE          ; Register 0 of speech chip
INFLECT := BASE+$01      ; Register 1 of speech chip
RATEINF := BASE+$02      ; Register 2 of speech chip
CTTRAMP := BASE+$03      ; Register 3 of speech chip
FILFREQ := BASE+$04      ; Register 4 of speech chip
DDRB    := $02
DDRA    := $03
PCR     := $8C           ; Peripheral control register, 6522
IFR     := $8D           ; Interrupt flag register, 6522
IER     := $8E


.DATA
_mockingBoardSpeechData:    .byte   $00, $00
_mockingBoardSpeechLen:     .byte   $00, $00
_outptr:                    .byte   $00, $00
_endptr:                    .byte   $00, $00
_mockingBoardSpeechBusy:    .byte   $00
_mockingBoardSpeechPlaying: .byte   $00

mock_irq:      .byte   $60
               .lobytes _mockInterrupt
               .hibytes _mockInterrupt


.CODE

writeChip:
    sta $C000,X
    rts

readChip:
    lda $C000,X
    rts
    
    
;license:MIT
; By Andrew Roughan
;   in the style of 4am for Total Replay
;   ported into a2bejewld by jrand
;
; Mockingboard support functions
;

;------------------------------------------------------------------------------
; GetMockingboardSlot
; detect Mockingboard card by searching for 6522 timers across all slots 7->1
; access 6522 timers with deterministic cycle counts
;
;   based on prior art in Mockingboard Developers Toolkit
;   with optimisation from deater/french touch
;   also takes into account FastChip //e clock difference
;
; in:    none
;        accelerators should be off
; out:
;        if card was found, A = #$?n where n is the slot number of the card, otherwise #$00
;                           and    bit 6 = 0 if Mockingboard Sound I found
;                               or bit 6 = 1 if Mockingboard Sound II or "A" found
;                           and    bit 7 = 1 if Mockingboard Sound/Speech I or "C" found
;        flags clobbered
;        zp $80-$82 clobbered
;           (jrand - and then restored because I am chicken and afraid of breaking cc65 runtime)
;        X/Y clobbered
;------------------------------------------------------------------------------
.proc _getMockingBoardSlot
             php
             lda   $80
             sta   zp80Backup
             lda   $81
             sta   zp81Backup
             lda   $82
             sta   zp82Backup
             lda   $3fe
             sta   irq1Backup
             lda   $3ff
             sta   irq2Backup
             
             lda   #$00
             sta   $80
             sta   $82                  ; type
             ldx   #$C1
@slotLoop:
             stx   $81
             ldy   #$04                  ; 6522 #1 $Cx04
             jsr   @timercheck
             beq   @foundI
             inx
             cpx   #$C8
             bne   @slotLoop
             ldx   #00                   ; not found
             jmp   @cleanup
             
@foundI:     ; sound I or better
             ldy   #$84                  ; 6522 #2 $Cx84
             jsr   @timercheck
             beq   @foundII
             
             ldy   #$0C
             sty   @mb_smc3 + 1
             iny
             sty   @mb_smc8 + 1
             iny
             sty   @mb_smc7 + 1
             sty   @mb_smc11 + 1
             
             .BYTE $2C                   ; Hide next 2 bytes using a BIT opcode
@foundII:    ; stereo
             ror   $82
             
             lda   $81
             sta   @mb_smc1 + 2
             sta   @mb_smc2 + 2
             sta   @mb_smc3 + 2
             sta   @mb_smc4 + 2
             sta   @mb_smc5 + 2
             sta   @mb_smc6 + 2
             sta   @mb_smc7 + 2
             sta   @mb_smc8 + 2
             sta   @mb_smc9 + 2
             sta   @mb_smc10 + 2
             sta   @mb_smc11 + 2
             sta   @mb_smc12 + 2
             sta   @mb_smc13 + 2
             
             ; detect speech chip
             
             sei
             lda   #<@mb_irq
             sta   $3fe
             lda   #>@mb_irq
             sta   $3ff
             
             lda   #0
@mb_smc1:
             sta   $c403
@mb_smc2:
             sta   $c402
             lda   #$0c
@mb_smc3:
             sta   $c48c
             lda   #$80
@mb_smc4:
             sta   $c443
             lda   #$c0
@mb_smc5:
             sta   $c440
             lda   #$70
@mb_smc6:
             sta   $c443
             lda   #$82
@mb_smc7:
             sta   $c48e
             
             ldx   #0
             ldy   #0
             sec
             cli
             
@wait_irq:
             lda   $80
             bne   @got_irq
             iny
             bne   @wait_irq
             inx
             bne   @wait_irq
             clc
             
@got_irq:

             sei
             ror   $82
             
             lda   $81
             and   #7
             ora   $82
             tax
             
@cleanup:
             lda   zp80Backup
             sta   $80
             lda   zp81Backup
             sta   $81
             lda   zp82Backup
             sta   $82
             lda   irq1Backup
             sta   $3fe
             lda   irq2Backup
             sta   $3ff
             txa
             ldx   #$00
             plp
             rts

@timercheck:
             sec
             lda   ($80),y               ; read 6522 timer low byte
             sbc   ($80),y               ; second time
             cmp   #5                    ; looking for (-)8 cycles between reads
             beq   :+
             cmp   #6                    ; FastChip //e clock is different
:            rts

@mb_irq:
             lda   #2
@mb_smc8:
             sta   $c48d
             lda   #0
@mb_smc9:
             sta   $c440
             lda   #$70
@mb_smc10:
             sta   $c443
             sta   $80
             lda   #2
@mb_smc11:
             sta   $c48e
             lda   #$ff
@mb_smc12:
             sta   $c403
             lda   #7
@mb_smc13:
             sta   $c402
             lda   $45
             rti

; Locals
zp80Backup: .BYTE $00
zp81Backup: .BYTE $00
zp82Backup: .BYTE $00
irq1Backup: .BYTE $00
irq2Backup: .BYTE $00
.endproc


.proc _mockingBoardSpeechInit
    sei

; The accumulator has the slot number of the mockingboard.
; Turn that into the address of the slot and set the address
; in the read and write functions.
    and #$7
    ora #$c0
    sta writeChip+2
    sta readChip+2

; Write a jump instruction at mock_irq to turn on our handler
    lda #$4c
    sta mock_irq

    cli
    rts
.endproc


.proc _mockingBoardSpeechShutdown
    sei

; Write a RTS instruction at mock_irq to disable our handler
    lda #$60
    sta mock_irq

    cli
    rts
.endproc


.proc _mockingBoardSpeakPriv
    sei
    lda #$00
    ldx #DDRA
    jsr writeChip
    ldx #DDRB
    jsr writeChip

; Get the starting address of the data and store in the work pointer
    lda _mockingBoardSpeechData+1
    sta _outptr+1
    lda _mockingBoardSpeechData
    sta _outptr

; Calculate the end address from the start address and the length
    lda _mockingBoardSpeechLen+1
    clc
    adc _mockingBoardSpeechData+1
    sta _endptr+1
    lda _mockingBoardSpeechLen
    clc
    adc _mockingBoardSpeechData
    bcc @L2
    inc _endptr+1
@L2:
    sta _endptr

; Set the busy flag
    lda #$FF
    sta _mockingBoardSpeechBusy

; Set peripheral control register to recognize the signal from the
; speech chip.
    lda #$0C
    ldx #PCR
    jsr writeChip

; Raise control bit in register 3
    lda #$80
    ldx #CTTRAMP
    jsr writeChip

; Set transitioned inflection mode in register 0
    lda #$C0
    ldx #DURPHON
    jsr writeChip

; Lower control bit
    lda #$70
    ldx #CTTRAMP
    jsr writeChip

; Enable 6522 interrupts
    lda #$82
    ldx #IER
    jsr writeChip

    cli
    rts
.endproc


.proc _mockInterrupt
; If we have a 6522 interrupt, jump to L4.
    ldx #IFR
    jsr readChip
    bmi @L4

; Otherwise clear the carry to indicate we didn't handle the interrupt
; and return to the caller.
    clc
    rts

@L4:
; Clear the interrupt flag
    lda #$02
    ldx #IFR
    jsr writeChip

; Check for end of data file.  If not the end, jump to L1
    lda _outptr+1
    cmp _endptr+1
    bcc @L1
    bne @L5
    lda _outptr
    cmp _endptr
    bcc @L1

@L5:

; If at the end, turn everything off.  Store a pause phoneme.
    lda #$00
    ldx #DURPHON
    jsr writeChip

; Zero amplitude
    lda #$70
    ldx #CTTRAMP
    jsr writeChip

; Clear busy and playing flags
    lda #$00
    sta _mockingBoardSpeechBusy
    sta _mockingBoardSpeechPlaying

; Clear interrupt enable in 6522
    lda #$02
    ldx #IER
    jsr writeChip
    lda #$FF
    ldx #DDRA
    jsr writeChip
    lda #$07
    ldx #DDRB
    jsr writeChip

@L2:
; Set the carry flag to indicate we handled the interrupt and return to the caller.
    sec
    rts

@L1:

; Set the speach playing flag
    lda #$ff
    sta _mockingBoardSpeechPlaying

; Save the value of the tmp pointer on the stack
    lda TMPPTR
    pha
    lda TMPPTR+1
    pha

; Move the _outptr into the tmp pointer
    lda _outptr
    sta TMPPTR
    lda _outptr+1
    sta TMPPTR+1

; Init registers
    ldy #$00
    ldx #FILFREQ

@L6:
; Get the next data
    lda (TMPPTR),Y

; Store in the speech chip
    jsr writeChip

; Next data
    inc TMPPTR
    bne @L3
    inc TMPPTR+1

@L3:
; Go to next register
    dex

; If we are not done the last register, then loop back to L6
    cpx #BASE-1
    bne @L6

; We are done writing so move the tmp pointer back into _outptr
    lda TMPPTR
    sta _outptr
    lda TMPPTR+1
    sta _outptr+1

; Restore the tmp pointer from the stack
    pla
    sta TMPPTR+1
    pla
    sta TMPPTR

; Finish the interrupt handler
    jmp @L2
.endproc
