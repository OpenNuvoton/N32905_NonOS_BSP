/****************************************************************
 *                                                              *
 * Copyright (c) Nuvoton Technology Corp. All rights reserved.  *
 *                                                              *
 ****************************************************************/
 
#ifndef	__SPU_H__
#define __SPU_H__

#include "w55fa93_reg.h"
#include "wbtypes.h"
#include "wblib.h"
#include "DrvSPU.h"


//==================================================================================
//Constant Definition
//==================================================================================
#define	MIXED_PCM_BUFFER_NUMBER	4  				// m_u16MixedPcmBufferSize = m_u16SpuBufferSize*MIXED_PCM_BUFFER_NUMBER

#define MIXER_BUSY			0x0001
#define	MIXER_STEREO		0x0002
#define MIXER_SPU_RAMP_ZERO	0x0008

#define	MIXER_CH_BUSY	0x0001
#define	MIXER_CH_STEREO	0x0002
#define	MIXER_CH_DECAY	0x0004
#define MIXER_CH_PAUSE	0x0008

#define MIXER_CH_FILL_RAW_BUF_AT_CRITICAL	0x0010	// 1-indicate has filled raw buffer when raw buffer ptr is rawbufsize-1

#define S_SPU_PARCON_MODE			BIT16
#define S_SPU_PARCON_STEREO			0xFFFFFFFE
#define S_SPU_PARCON_MONO			0x00000001

#define SPU_ENABLE()	outpw(REG_SPU_CTRL, inpw(REG_SPU_CTRL) | 0x01)	
#define SPU_DISABLE()	outpw(REG_SPU_CTRL, inpw(REG_SPU_CTRL) & ~0x01)	
#define SPU_RESET()		outpw(REG_SPU_CTRL, inpw(REG_SPU_CTRL) | 0x00010000); outpw(REG_SPU_CTRL, inpw(REG_SPU_CTRL) & ~0x00010000)
#define	SPU_ISENABLED()	(inpw(REG_SPU_CTRL) | 0x01)	


#define SPU_CTRL    REG_SPU_CTRL			// SPU control and status register
#define DAC_PAR     REG_SPU_DAC_PAR			// DAC parameter register
#define DAC_VOL     REG_SPU_DAC_VOL			// Sub block reset control
#define EQGain0     REG_SPU_EQGain0			// Equalizer bands 08 - 01 gain control
#define EQGain1     REG_SPU_EQGain1			// Equalizer bands 10 - 09 and DC gain control
#define CH_EN       REG_SPU_CH_EN			// Channel enable register
#define CH_IRQ      REG_SPU_CH_IRQ			// Channel iterrupt request flag register
#define CH_PAUSE    REG_SPU_CH_PAUSE		// Channel PAUSE register
#define CH_CTRL     REG_SPU_CH_CTRL			// Channel control register
#define S_ADDR      REG_SPU_S_ADDR			// Source start (base) address register
#define M_ADDR      REG_SPU_M_ADDR			// Threshold address register
#define E_ADDR      REG_SPU_E_ADDR			// End start address register
#define TONE_PULSE  REG_SPU_TONE_PULSE		// Tone Pulse control register
#define TONE_AMP    REG_SPU_TONE_AMP		// Tone Amplitude control register
#define CH_PAR_1    REG_SPU_CH_PAR_1		// Channel parameter 1 register
#define CH_PAR_2    REG_SPU_CH_PAR_2		// Channel parameter 2 register
#define CH_EVENT    REG_SPU_CH_EVENT		// DMA down counter register
#define CUR_ADDR    REG_SPU_CUR_ADDR		// DMA down counter register
#define LP_ADDR     REG_SPU_LP_ADDR			// DMA down counter register
#define PA_ADDR     REG_SPU_PA_ADDR			// Pause Address for mono/stereo PCM16 
#define P_BYTES     REG_SPU_P_BYTES			// Chanel loop paly byte conuts

//=================================================================================
//struct declare
//=================================================================================
typedef union tagFF16_16
{
	UINT32 m_u32Value;
	struct
	{
		UINT16 m_u16Fraction;
		UINT16 m_u16Integer;
	} m_sFF16;
} U_FF16_16;

// ioctls
#define SPU_IOCTL_SET_BASE_ADDRESS 	0
#define SPU_IOCTL_SET_TH1_ADDRESS	1
#define SPU_IOCTL_SET_TH2_ADDRESS	2
#define SPU_IOCTL_SET_VOLUME		3
#define SPU_IOCTL_SET_MONO			4
#define SPU_IOCTL_GET_FRAG_SIZE		5
#define SPU_IOCTL_SET_STEREO		6

#define FRAG_SIZE		(32 * 1024)
#define HALF_FRAG_SIZE	(FRAG_SIZE/2)
extern UINT8	*_pucPlayAudioBuff;

VOID spuDacOn(UINT8 level);
VOID spuDacOff(VOID);
VOID spuStartPlay(PFN_DRVSPU_CB_FUNC *fnCallBack, UINT8 *data);
VOID spuStopPlay(VOID);
VOID spuIoctl(UINT32 cmd, UINT32 arg0, UINT32 arg1);
VOID spuOpen(UINT32 u32SampleRate);
VOID spuClose (VOID);
VOID spuEqOpen (E_DRVSPU_EQ_BAND eEqBand, E_DRVSPU_EQ_GAIN eEqGain);
VOID spuEqClose (VOID);

#endif
