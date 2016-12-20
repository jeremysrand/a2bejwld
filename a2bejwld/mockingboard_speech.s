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
