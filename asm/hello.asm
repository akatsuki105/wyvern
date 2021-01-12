	SECTION	"Start",ROM0[$100]		; start vector, followed by header data applied by rgbfix.exe
	nop
	jp	start

    SECTION "Example",ROM0[$150]		; code starts here

start:
	di					; disable interrupts
	ld	sp,$E000			; setup stack

.wait_vbl					; wait for vblank to properly disable lcd
	ld	a,[$FF44]	
	cp	$90
	jr	nz,.wait_vbl

.the_end
	halt					; save battery
;	nop					    ; nop after halt is mandatory but rgbasm takes care of it :)
	jr	.the_end			; endless loop

    INCLUDE "decompress.asm"
