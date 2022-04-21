; macro to define variables
%macro var 2
	global _%1
	_%1: equ %2
%endmacro

; macro to define functions
%macro func 2
	global _%1
	_%1:
	jmp[.a]
	.a: dd %2
%endmacro

; macro to define c++ function jumpers
%macro funp 2
	global _%1
	_%1: dd %2
%endmacro

; ---------------------------------------
; - VARIABLES							-
; ---------------------------------------
section .data
var hWnd,						0x93216C
var pDInput,					0x9321A8
var pRenderTarget,				0xAC6C58

var dword_1D81D18,				0x1D81D18

; ---------------------------------------
; - FUNCTIONS							-
; ---------------------------------------
section .text
func WndProcedure@16,			0x406450
