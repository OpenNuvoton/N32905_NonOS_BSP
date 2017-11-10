;	.ARM						;ARM core, also can be .code 32
;	.align						;align to word(4 Bytes) boundary
;	.extern g_asWireEnc
;	.global Ecc4Encoder
;	.global g_p
;	.bss
;g_p:	.WORD	0, 0, 0, 0, 0, 0, 0, 0
	
	AREA WIREENC, CODE, READONLY	
	IMPORT	g_asWireEnc
	GLOBAL	Ecc4Encoder
	GLOBAL	g_p	
	ALIGN 4
g_p
	DCD	0, 0, 0, 0, 0, 0, 0, 0

;	.text
;.macro  _ECC4ENCODE
	ALIGN 	4
	MACRO	
$M0start _ECC4ENCODE
										; g_x = g_p[7]^(((unsigned int)(*pacEncodeData++))<<2); // = { x[7:0], p[71], p[70] }
	LDRB   R8, [R12], #1
	EOR    R8, R7, R8, LSL #2
	;
										; g_uEnc1 = g_asWireEnc[g_x].enc1;
										; g_uEnc2 = g_asWireEnc[g_x].enc2;
										; g_uEnc3 = g_asWireEnc[g_x].enc3;
										; g_uEnc4 = g_asWireEnc[g_x].enc4;
	;MVN    R9, #0
	;AND    R8, R8, R9, LSR #(32-10)
	;ADD    R9, R11, R8, LSL #3
	MOV    R9, R8, LSL #(32-10)
	ADD    R9, R11, R9, LSR #(32-13)
	
	LDMIA  R9, {R9, R10}				; R9: g_asWireEnc[g_x].enc2, g_asWireEnc[g_x].enc1
										; R10:g_asWireEnc[g_x].enc4, g_asWireEnc[g_x].enc3
	;
	EOR    R7, R6, R9					; g_p[7] = g_p[6]^g_uEnc1;
	EOR    R6, R5, R9, LSR #16			; g_p[6] = g_p[5]^g_uEnc2;
	EOR    R5, R4, R10					; g_p[5] = g_p[4]^g_uEnc3;
	EOR    R4, R3, R10, LSR #16			; g_p[4] = g_p[3]^g_uEnc4;
	EOR    R3, R2, R10					; g_p[3] = g_p[2]^g_uEnc3;
	EOR    R2, R1, R9, LSR #16			; g_p[2] = g_p[1]^g_uEnc2;
	EOR    R1, R0, R9					; g_p[1] = g_p[0]^g_uEnc1;
	MOV    R0, R8						; g_p[0] = g_x;
;.endm
	MEND
	
;.macro  _ECC4ENCODE_EN G_P0, NEWREG
	ALIGN 4
	MACRO	
$M1start _ECC4ENCODE_EN $G_P0, $NEWREG
									; g_x = g_p[7]^(((unsigned int)(*pacEncodeData++))<<2); // = { x[7:0], p[71], p[70] }
	LDRB   $NEWREG, [R12], #1
	EOR    $NEWREG, R7, $NEWREG, LSL #2
	;
										; g_uEnc1 = g_asWireEnc[g_x].enc1;
										; g_uEnc2 = g_asWireEnc[g_x].enc2;
										; g_uEnc3 = g_asWireEnc[g_x].enc3;
										; g_uEnc4 = g_asWireEnc[g_x].enc4;
	;MVN    R9, #0
	;AND    R8, R8, R9, LSR #(32-10)
	;ADD    R9, R11, R8, LSL #3
	MOV    R9, $NEWREG, LSL #(32-10)
	ADD    R9, R11, R9, LSR #(32-13)
	
	LDMIA  R9, {R9, R10}				; R9: g_asWireEnc[g_x].enc2, g_asWireEnc[g_x].enc1
										; R10:g_asWireEnc[g_x].enc4, g_asWireEnc[g_x].enc3
	;
	EOR    R7, R6, R9					; g_p[7] = g_p[6]^g_uEnc1;
	EOR    R6, R5, R9, LSR #16			; g_p[6] = g_p[5]^g_uEnc2;
	EOR    R5, R4, R10					; g_p[5] = g_p[4]^g_uEnc3;
	EOR    R4, R3, R10, LSR #16			; g_p[4] = g_p[3]^g_uEnc4;
	EOR    R3, R2, R10					; g_p[3] = g_p[2]^g_uEnc3;
	EOR    R2, R1, R9, LSR #16			; g_p[2] = g_p[1]^g_uEnc2;
	EOR    R1, $G_P0, R9				; g_p[1] = g_p[0]^g_uEnc1;
	;MOV   $G_P0, $NEWREG				; g_p[0] = g_x;
;.endm
	MEND
;=====================Subroutine====================
; Ecc4Encoder
; R0: unsigned char *pacEncodeData
; R1: unsigned char *pacEc4ParityByte
; R2: BOOL bDecode
;===================================================
;Ecc4Encoder:
Ecc4Encoder
	STMDB  R13!, {R1-R12,R14}

	; R0: g_p0
	; R1: g_p1
	; R2: g_p2
	; R3: g_p3
	; R4: g_p4
	; R5: g_p5
	; R6: g_p6
	; R7: g_p7
	; R8: g_x
	; R9: 
	; R10:
	; R11: g_asWireEnc
	; R12: pacEncodeData
	; R14: loop count
	LDR    R11, =g_asWireEnc
	MOV    R12, R0
	MOV    R14, #512
	;
										; for(i = 0; i < 8; i++ )
										;	g_p[i] = 0;
	MOV    R0, #0
	MOV    R1, #0
	MOV    R2, #0
	MOV    R3, #0
	MOV    R4, #0
	MOV    R5, #0
	MOV    R6, #0
	MOV    R7, #0
	;
;_Ecc4EncoderLoop:
_Ecc4EncoderLoop
;/*										; g_x = g_p[7]^(((unsigned int)(*pacEncodeData++))<<2); // = { x[7:0], p[71], p[70] }
;	LDRB   R8, [R12], #1
;	EOR    R8, R7, R8, LSL #2
	;
										; g_uEnc1 = g_asWireEnc[g_x].enc1;
										; g_uEnc2 = g_asWireEnc[g_x].enc2;
										; g_uEnc3 = g_asWireEnc[g_x].enc3;
										; g_uEnc4 = g_asWireEnc[g_x].enc4;
;	MVN    R9, #0
;	AND    R8, R8, R9, LSR #(32-10)
;	ADD    R9, R11, R8, LSL #3
;	LDMIA  R9, {R9, R10}				; R9: g_asWireEnc[g_x].enc2, g_asWireEnc[g_x].enc1
										; R10:g_asWireEnc[g_x].enc4, g_asWireEnc[g_x].enc3
	;
;	EOR    R7, R6, R9					; g_p[7] = g_p[6]^g_uEnc1;
;	EOR    R6, R5, R9, LSR #16			; g_p[6] = g_p[5]^g_uEnc2;
;	EOR    R5, R4, R10					; g_p[5] = g_p[4]^g_uEnc3;
;	EOR    R4, R3, R10, LSR #16			; g_p[4] = g_p[3]^g_uEnc4;
;	EOR    R3, R2, R10					; g_p[3] = g_p[2]^g_uEnc3;
;	EOR    R2, R1, R9, LSR #16			; g_p[2] = g_p[1]^g_uEnc2;
;	EOR    R1, R0, R9					; g_p[1] = g_p[0]^g_uEnc1;
;	MOV    R0, R8						; g_p[0] = g_x;
	;*/
	_ECC4ENCODE_EN R0, R8
	_ECC4ENCODE_EN R8, R0
	_ECC4ENCODE_EN R0, R8
	_ECC4ENCODE_EN R8, R0
	_ECC4ENCODE_EN R0, R8
	_ECC4ENCODE_EN R8, R0
	_ECC4ENCODE_EN R0, R8
	_ECC4ENCODE_EN R8, R0
	;
	SUBS   R14, R14, #8
	BNE    _Ecc4EncoderLoop	
	;
	; R0: g_p0
	; R1: g_p1
	; R2: g_p2
	; R3: g_p3
	; R4: g_p4
	; R5: g_p5
	; R6: g_p6
	; R7: g_p7
	; R8: pacEc4ParityByte
	; R9: 0x3ff
	; R10, R11, R12, R14: free register
	LDR    R8, [R13, #0]
	;
	LDR    R9, [R13, #4]
	CMP    R9, #1
	BNE    _ENCODE_GEN_PARITY
;_DECODE_CHK_PARITY:
_DECODE_CHK_PARITY
	MVN    R9, #0
	MOV    R9, R9, LSR #(32-10)
	;								; g_p[0] = (g_p[0]^(pacEc4ParityByte[0] | (pacEc4ParityByte[1]<<8)))&0x3ff;
	LDRB   R10, [R8], #1			; R10 = pacEc4ParityByte[0]
	LDRB   R11, [R8], #1			; R11 = pacEc4ParityByte[1]
	ORR    R10, R10, R11, LSL #8
	EOR    R0, R0, R10
	AND    R0, R0, R9
	;								; g_p[1] = (g_p[1]^((pacEc4ParityByte[1]>>2) | (pacEc4ParityByte[2]<<6)))&0x3ff;
	LDRB   R10, [R8], #1			; R10 = pacEc4ParityByte[2]
	MOV    R11, R11, LSR #2
	ORR    R11, R11, R10, LSL #6
	EOR    R1, R1, R11
	AND    R1, R1, R9
	;								; g_p[2] = (g_p[2]^((pacEc4ParityByte[2]>>4) | (pacEc4ParityByte[3]<<4)))&0x3ff;
	LDRB   R11, [R8], #1			; R11 = pacEc4ParityByte[3]
	MOV    R10, R10, LSR #4
	ORR    R10, R10, R11, LSL #4
	EOR    R2, R2, R10
	AND    R2, R2, R9
	;								; g_p[3] = (g_p[3]^((pacEc4ParityByte[3]>>6) | (pacEc4ParityByte[4]<<2)))&0x3ff;
	LDRB   R10, [R8], #1			; R10 = pacEc4ParityByte[4]
	MOV    R11, R11, LSR #6
	ORR    R11, R11, R10, LSL #2
	EOR    R3, R3, R11
	AND    R3, R3, R9
	;								; g_p[4] = (g_p[4]^(pacEc4ParityByte[5] | (pacEc4ParityByte[6]<<8)))&0x3ff;
	LDRB   R10, [R8], #1			; R10 = pacEc4ParityByte[5]
	LDRB   R11, [R8], #1			; R11 = pacEc4ParityByte[6] 
	ORR    R10, R10, R11, LSL #8
	EOR    R4, R4, R10
	AND    R4, R4, R9
	;								; g_p[5] = (g_p[5]^((pacEc4ParityByte[6]>>2) | (pacEc4ParityByte[7]<<6)))&0x3ff;
	LDRB   R10, [R8], #1			; R10 = pacEc4ParityByte[7]
	MOV    R11, R11, LSR #2
	ORR    R11, R11, R10, LSL #6
	EOR    R5, R5, R11
	AND    R5, R5, R9
	;								; g_p[6] = (g_p[6]^((pacEc4ParityByte[7]>>4) | (pacEc4ParityByte[8]<<4)))&0x3ff;
	LDRB   R11, [R8], #1			; R11 = pacEc4ParityByte[8]
	MOV    R10, R10, LSR #4
	ORR    R10, R10, R11, LSL #4
	EOR    R6, R6, R10
	AND    R6, R6, R9
	;								; g_p[7] = (g_p[7]^((pacEc4ParityByte[8]>>6) | (pacEc4ParityByte[9]<<2)))&0x3ff;
	LDRB   R10, [R8, #0]			; R10 = pacEc4ParityByte[9]
	MOV    R11, R11, LSR #6
	ORR    R11, R11, R10, LSL #2
	EOR    R7, R7, R11
	AND    R7, R7, R9
	;
	LDR    R8, =g_p
	STMIA  R8, {R0-R7}
	;
	LDMIA  R13!, {R1-R12,PC}
	;
;_ENCODE_GEN_PARITY:
_ENCODE_GEN_PARITY
	;								; pacEc4ParityByte[0] = (unsigned char)g_p[0];
	STRB   R0, [R8], #1
	;								; pacEc4ParityByte[1] = (unsigned char)(((g_p[0]>>8)&3)|(g_p[1]<<2));
	MOV    R0, R0, LSR #8
	AND    R0, R0, #3
	ORR    R0, R0, R1, LSL #2
	STRB   R0, [R8], #1
	;								; pacEc4ParityByte[2] = (unsigned char)(((g_p[1]>>6)&0xf)|(g_p[2]<<4));
	MOV    R1, R1, LSR #6
	AND    R1, R1, #0xf
	ORR    R1, R1, R2, LSL #4
	STRB   R1, [R8], #1
	;								; pacEc4ParityByte[3] = (unsigned char)(((g_p[2]>>4)&0x3f)|(g_p[3]<<6));
	MOV    R2, R2, LSR #4
	AND    R2, R2, #0x3f
	ORR    R2, R2, R3, LSL #6
	STRB   R2, [R8], #1
	;								; pacEc4ParityByte[4] = (unsigned char)(g_p[3]>>2);
	MOV    R3, R3, LSR #2
	STRB   R3, [R8], #1
	;								; pacEc4ParityByte[5] = (unsigned char)g_p[4];
	STRB   R4, [R8], #1
	;								; pacEc4ParityByte[6] = (unsigned char)(((g_p[4]>>8)&3)|(g_p[5]<<2));
	MOV    R4, R4, LSR #8
	AND    R4, R4, #3
	ORR    R4, R4, R5, LSL #2
	STRB   R4, [R8], #1
	;								; pacEc4ParityByte[7] = (unsigned char)(((g_p[5]>>6)&0xf)|(g_p[6]<<4));
	MOV    R5, R5, LSR #6
	AND    R5, R5, #0xf
	ORR    R5, R5, R6, LSL #4
	STRB   R5, [R8], #1
	;								; pacEc4ParityByte[8] = (unsigned char)(((g_p[6]>>4)&0x3f)|(g_p[7]<<6));
	MOV    R6, R6, LSR #4
	AND    R6, R6, #0x3f
	ORR    R6, R6, R7, LSL #6
	STRB   R6, [R8], #1
	;								; pacEc4ParityByte[9] = (unsigned char)(g_p[7]>>2);
	MOV    R7, R7, LSR #2
	STRB   R7, [R8, #0]
	;
	LDMIA  R13!, {R1-R12,PC}

;	.end
	END
	
