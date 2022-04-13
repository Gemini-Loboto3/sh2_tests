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

var dword_1D81D18,				0x1D81D18
var vsync_timer,				0x24A4C40
var hvsync,						0x24A4C6C

var adxt_time_mode,				0x1D7FBC0
var adxt_tsvr_enter_cnt,		0x1D7FBC4
var adxf_ldpt_buf,				0x1D81C70
var adxf_ldpt_rdsct,			0x1D81C74
var adxm_init_level,			0x1D81CC8
var adxm_save_tprm,				0x1D81CCC
var nPriority,					0x1D81CDC
var adxm_lock_level,			0x1D81CE0
var adxm_goto_border_flag,		0x1D81CE4
var adxm_safe_cnt,				0x1D81CE8
var adxm_vsync_cnt,				0x1D81CEC
var adxm_mwidle_cnt,			0x1D81CF0
var adxm_mwidle_sleep_cb,		0x1D81CF8
var adxm_safe_loop,				0x1D81D00
var adxm_safe_exit,				0x1D81D04
var adxm_vsync_loop,			0x1D81D08
var adxm_vsync_exit,			0x1D81D0C
var adxm_mwidle_loop,			0x1D81D10
var adxm_mwidle_exit,			0x1D81D14
var adxwin_init_flag,			0x1D81D40
var adxwin_thread_kill_flag,	0x1D81D44
var adxt_init_cnt,				0x1D81D48
var adxt_svr_main_id,			0x1D81D50
var adxt_vsync_cnt,				0x1D81D54

var adxm_mwidle_thrdhn,			0x24A4C5C
var adxm_vsync_thrdhn,			0x24A4C68
var adxm_safe_thrdhn,			0x24A4C80

var pDInput,					0x9321A8
var pRenderTarget,				0xAC6C58

; ---------------------------------------
; - FUNCTIONS							-
; ---------------------------------------
section .text
func SVM_ExecSvrMwIdle,			0x56AD20
func ADXM_WaitVsync,			0x55FFC0

func ADXT_SetupThrd,			0x55FD70

func WndProcedure@16,			0x406450
