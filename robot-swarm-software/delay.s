.equ __P24FJ32GA002,1	    ;required "boiler-plate" (BP)
.include "p24Fxxxx.inc"	    ;BP
#include "xc.inc"	    ;BP
.text			    ;BP (put the following data in ROM(program memory))
    
;This is a library; this it cannot contain a _main function: the C file will define main().
;However, we wiil need a .global statement to make available ASM function to C code.
;All functions utilized out of this file will need to have a leading underscore (_) and be included
;in a comment delimited list below

.global _write_0, _write_1, _wait_50us, _wait_1ms
    
_wait_50us:
			;2 cycles for call execution
    repeat  #793	;1 cycle for load and prep
    nop			;793 + 1 cycles for nop execution
    return		;3 cycles for return execution

_write_0:
    			;2 cycles low
    inc	    LATA	;1 cycle high
    repeat  #3		;1 cycle for load and prep
    nop			;3 + 1 cycles for nop execution
    
    clr	    LATA	;1 cycle low
    repeat  #6		;1 cycle low
    nop			;6 + 1 cycles low execution
    return		;3 cycles low execution
    
_write_1:
			;2 cycles low
    inc	    LATA	;1 cycle high
    repeat  #8		;1 cycle high
    nop			;8 + 1 cycles high
    
    clr	    LATA	;1 cycle low
    repeat  #1		;1 cycle low
    nop			;1 + 1 cycles low
    return		;3 cycles low
    
_wait_1ms:
    repeat  #15993	;1 cycle for load and prep
    nop			;15993 + 1 cycles for nop execution
    return		;3 cycles for return execution
    

