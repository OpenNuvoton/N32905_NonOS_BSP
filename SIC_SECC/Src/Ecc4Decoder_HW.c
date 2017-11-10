//#include "Platform.h"
#include "wblib.h"
#define FALSE 	0
#define TRUE 		1
#define GetBit(nValue, bit)	(((nValue)>>(bit))&1)
/*
unsigned char g_ucSyndromeF1Lookup[1024];
unsigned char g_ucSyndromeF2Lookup[1024], g_ucSyndromeF3Lookup[1024];
unsigned char g_ucSyndromeF4Lookup[1024], g_ucSyndromeF5Lookup[1024];
unsigned char g_ucSyndromeF6Lookup[1024], g_ucSyndromeF7Lookup[1024];
*/
#include "Ecc4DecoderSyndromeFxLookup.h"
/*
unsigned char  g_au8CorrectLookupR0[1024], g_au8CorrectLookupR1[1024];
unsigned char  g_au8CorrectLookupR2[1024], g_au8CorrectLookupR3[1024];
unsigned short int  g_au16CorrectLookupR4[1024], g_au16CorrectLookupR5[1024];
unsigned short int  g_au16CorrectLookupR6[1024], g_au16CorrectLookupR7[1024];
unsigned short int  g_au16CorrectInitLookupR0[1024], g_au16CorrectInitLookupR1[1024];
unsigned short int  g_au16CorrectInitLookupR2[1024], g_au16CorrectInitLookupR3[1024];
unsigned short int  g_au16CorrectInitLookupR4[1024], g_au16CorrectInitLookupR5[1024];
unsigned short int  g_au16CorrectInitLookupR6[1024], g_au16CorrectInitLookupR7[1024];
*/
#include "Ecc4DecoderCorrectLookup.h"
/*
unsigned short int  g_au16RiBmLookup[1024];
*/
#include "Ecc4DecoderRiBmLookup.h"

extern unsigned int g_p[8];
//extern void Ecc4Encoder(unsigned char *pacEncodeData, unsigned char *pacEc4ParityByte, bool bDecode);

// input - p_in[8] : each element contain 10 bits
// output - s[8]: each element contain 10 bits
void syndrome(unsigned int *p_in, unsigned int *s);
// input - s[8]: each element contain 10 bits
// output- L[5]: each element contain 10 bits
// output- o[4]: each element contain 10 bits
void riBM(unsigned int *s, unsigned int *L, unsigned int *o);
// input - L[5]: each element contain 10 bits
// input - o[4]: each element contain 10 bits
// output- err_addr[4]: each element contain 10 bits
// output- err_data[4]: each element contain 8  bits
unsigned int correct(unsigned int *L, unsigned int *o, unsigned int *err_addr, unsigned char *err_data);

unsigned int Ecc4Decoder(unsigned char *pacEncodeData, unsigned char *pacEcc4ParityByte)
{
	unsigned int i;
	unsigned int s[8];
	unsigned int L[5];
	unsigned int o[4];
	unsigned int nErrCount;
	unsigned int auiErrorAddr[4];
	unsigned char acCorrectData[4];	
	Ecc4Encoder( pacEncodeData, pacEcc4ParityByte, TRUE );
	for( i = 0; i < 8; i ++ )
	{
		if ( g_p[i] != 0 )
		{
			syndrome(g_p, s);
			riBM(s, L, o);
			nErrCount = correct(L, o, &auiErrorAddr[0], &acCorrectData[0]);
			if ( nErrCount && (nErrCount <= 4) )
			{
				for( i = 0; i < nErrCount; i ++ )
					pacEncodeData[auiErrorAddr[i]] ^= acCorrectData[i];
			}
			return nErrCount;
		}
	}
	return 0;
}

void syndrome(unsigned int *p_in, unsigned int *s)
{
	unsigned int _s[8], f[8], clock, Xorf, i;

/*

      {f7, f6, f5, f4, f3, f2, f1, f0}               <= p_in[79:0];
      {_s7, _s6, _s5, _s4, _s3, _s2, _s1, _s0}       <= {8{10'h0}};
*/
	for(i = 0; i<= 7; i++)
	{
		f[i] = p_in[i];
		_s[i] = 0;
	}

	for(clock = 0; clock < 10; clock++)
	{
/*		_s0 <= {_s0[8:0],{f1[9]^f1[8]^f1[4]^f1[1]^f2[7]^f2[6]^f2[3]^f2[0]^f3[9]^f3[7]^f3[6]^
                   f3[5]^f3[3]^f3[2]^f3[1]^f3[0]^f4[8]^f4[7]^f4[4]^f4[3]^f4[1]^f5[6]^f5[4]^f5[2]^
                   f5[1]^f5[0]^f6[9]^f6[8]^f6[7]^f6[5]^f6[4]^f6[2]^f6[0]^f7[9]^f7[3]^f7[1]^f7[0]}};
		_s1<=  {_s1[8:0],{f1[8]^f1[7]^f1[3]^f1[0]^f2[8]^f2[4]^f2[1]^f3[9]^f3[8]^f3[7]^f3[5]^
                   f3[3]^f3[2]^f3[0]^f4[9]^f4[7]^f4[6]^f4[3]^f4[0]^f5[9]^f5[7]^f5[5]^f5[4]^f5[3]^
                   f5[2]^f5[1]^f6[9]^f6[8]^f6[5]^f6[4]^f6[2]^f7[6]^f7[4]^f7[2]^f7[1]^f7[0]}};
		_s2<=  {_s2[8:0],{f1[9]^f1[7]^f1[2]^f2[9]^f2[2]^f3[9]^f3[7]^f3[5]^f3[2]^f3[0]^f4[9]^
                   f4[5]^f4[2]^f5[9]^f5[8]^f5[7]^f5[5]^f5[3]^f5[2]^f5[0]^f6[9]^f6[8]^f6[5]^f6[2]^
                   f7[9]^f7[7]^f7[6]^f7[5]^f7[3]^f7[2]^f7[1]^f7[0]}};
		_s3<=  {_s3[8:0],{f1[8]^f1[6]^f1[1]^f2[7]^f2[0]^f3[9]^f3[7]^f3[2]^f4[8]^f4[1]^f5[8]^
                   f5[7]^f5[3]^f5[0]^f6[9]^f6[2]^f7[9]^f7[8]^f7[4]^f7[1]}};
		_s4<=  {_s4[8:0],{f1[7]^f1[5]^f1[0]^f2[8]^f3[9]^f3[4]^f4[7]^f5[8]^f5[3]^f6[6]^f7[7]^f7[2]}};
		_s5<=  {_s5[8:0],{f1[9]^f1[4]^f2[6]^f3[6]^f3[1]^f4[3]^f5[8]^f5[5]^f5[3]^f6[0]^f7[5]^
                   f7[2]^f7[0]}};
		_s6<=  {_s6[8:0],{f1[8]^f1[3]^f2[4]^f3[8]^f3[5]^f3[3]^f4[9]^f4[6]^f5[8]^f5[5]^f5[3]^
                   f5[0]^f6[4]^f6[1]^f7[8]^f7[3]^f7[2]^f7[0]}};
		_s7<=  {_s7[8:0],{f1[7]^f1[2]^f2[2]^f3[5]^f3[2]^f3[0]^f4[5]^f4[2]^f5[8]^f5[3]^f5[2]^
                   f5[0]^f6[8]^f6[2]^f7[6]^f7[5]^f7[2]^f7[1]^f7[0]}}; */
		Xorf = 
			g_ucSyndromeF1Lookup[f[1]]^
			g_ucSyndromeF2Lookup[f[2]]^
			g_ucSyndromeF3Lookup[f[3]]^
			g_ucSyndromeF4Lookup[f[4]]^
			g_ucSyndromeF5Lookup[f[5]]^
			g_ucSyndromeF6Lookup[f[6]]^
			g_ucSyndromeF7Lookup[f[7]];
		_s[0] = (_s[0]<<1)|((Xorf>>0)&1);
		_s[1] = (_s[1]<<1)|((Xorf>>1)&1);
		_s[2] = (_s[2]<<1)|((Xorf>>2)&1);
		_s[3] = (_s[3]<<1)|((Xorf>>3)&1);
		_s[4] = (_s[4]<<1)|((Xorf>>4)&1);
		_s[5] = (_s[5]<<1)|((Xorf>>5)&1);
		_s[6] = (_s[6]<<1)|((Xorf>>6)&1);
		_s[7] = (_s[7]<<1)|((Xorf>>7)&1);

/*		f1<= #1  {f1[8:0],(f1[9]^f1[6])};
		f2<= #1  {f2[8:0],(f2[9]^f2[6])};
		f3<= #1  {f3[8:0],(f3[9]^f3[6])};
		f4<= #1  {f4[8:0],(f4[9]^f4[6])};
		f5<= #1  {f5[8:0],(f5[9]^f5[6])};
		f6<= #1  {f6[8:0],(f6[9]^f6[6])};
		f7<= #1  {f7[8:0],(f7[9]^f7[6])};*/
		f[1] = ((f[1]<<1)|(((f[1]>>9)^(f[1]>>6))&1))&0x3ff;
		f[2] = ((f[2]<<1)|(((f[2]>>9)^(f[2]>>6))&1))&0x3ff;
		f[3] = ((f[3]<<1)|(((f[3]>>9)^(f[3]>>6))&1))&0x3ff;
		f[4] = ((f[4]<<1)|(((f[4]>>9)^(f[4]>>6))&1))&0x3ff;
		f[5] = ((f[5]<<1)|(((f[5]>>9)^(f[5]>>6))&1))&0x3ff;
		f[6] = ((f[6]<<1)|(((f[6]>>9)^(f[6]>>6))&1))&0x3ff;
		f[7] = ((f[7]<<1)|(((f[7]>>9)^(f[7]>>6))&1))&0x3ff;
	}

/*	
  assign s0  = _s0  ^ f0;
  assign s1  = _s1  ^ f0;
  assign s2  = _s2  ^ f0;
  assign s3  = _s3  ^ f0;
  assign s4  = _s4  ^ f0;
  assign s5  = _s5  ^ f0;
  assign s6  = _s6  ^ f0;
  assign s7  = _s7  ^ f0;
 */
	s[0] = (_s[0]^f[0])&0x3ff;
	s[1] = (_s[1]^f[0])&0x3ff;
	s[2] = (_s[2]^f[0])&0x3ff;
	s[3] = (_s[3]^f[0])&0x3ff;
	s[4] = (_s[4]^f[0])&0x3ff;
	s[5] = (_s[5]^f[0])&0x3ff;
	s[6] = (_s[6]^f[0])&0x3ff;
	s[7] = (_s[7]^f[0])&0x3ff;
}

unsigned int _10BitXor(unsigned int x )
{
	unsigned int nXor, count;

	nXor = x&1;
	for( count = 1; count < 10; count++ )
	{
		x>>=1;
		nXor = nXor ^ x;
	}
	return nXor&1;
}

void riBM(unsigned int *s, unsigned int *L, unsigned int *oo)
{
	unsigned int count, clock;
	unsigned int o[8];			// 10 bits for each element
	unsigned int fmulo[9];		// 10 bits for each element
	unsigned int fmult[8];		// 10 bits for each element
	unsigned int fmull[5];		// 10 bits for each element
	unsigned int fmulb[4];		// 10 bits for each element
	unsigned int B_val[9];		// 10 bits for each element
	unsigned int theta[8];		// 10 bits for each element
	unsigned int gamma, delta;	// 10 bits for each element
	unsigned int K_val;			// 4 bits 
	unsigned int enb1, enb2;
	unsigned int i;

	{
/*
      gamma <= 10'h200;
      K_val <= 4'h0; 
	
      B_val0                           <= 10'h004;
      {B_val4, B_val3, B_val2, B_val1} <= {4{10'h0}};
      {theta3, theta2, theta1, theta0} <= {s3, s2, s1, s0};
      {theta7, theta6, theta5, theta4} <= {s7, s6, s5, s4};
	
      L0                               <= 10'h004;
      {L4, L3, L2, L1}                 <= {4{10'h0}};
      {o7, o6, o5, o4, o3, o2, o1, o0} <= {s7, s6, s5, s4, s3, s2, s1, s0};
*/
		gamma = 0x200; 
		K_val = 0x0; 

		B_val[0] = 0x4;
		B_val[1] = 0;
		B_val[2] = 0;
		B_val[3] = 0;
		B_val[4] = 0;
		L[0] = 0x4;
		L[1] = 0;
		L[2] = 0;
		L[3] = 0;
		L[4] = 0; 

		for( i = 0; i <= 7; i ++)
			o[i] = theta[i] = s[i];
		
	}
	for(clock = count = 0; clock < 88; clock++)
	{
		{
/*
  wire       enb1 = riBM_en & (|count);
  wire       enb2 = riBM_en & ~(|count);
*/
			enb1 = (count!=0);
			enb2 = !enb1;
		}
		{
/*
		if(enb1)
			begin
			  L0 <= {L0[8:0], ^(gamma&fmull0)};
			  L1 <= {L1[8:0], ^((gamma&fmull1)^(delta&fmulb0))};
			  L2 <= {L2[8:0], ^((gamma&fmull2)^(delta&fmulb1))};
			  L3 <= {L3[8:0], ^((gamma&fmull3)^(delta&fmulb2))};
			  L4 <= {L4[8:0], ^((gamma&fmull4)^(delta&fmulb3))};
			  o0 <= {o0[8:0], ^((gamma&fmulo1)^(delta&fmult0))};
			  o1 <= {o1[8:0], ^((gamma&fmulo2)^(delta&fmult1))};
			  o2 <= {o2[8:0], ^((gamma&fmulo3)^(delta&fmult2))};
			  o3 <= {o3[8:0], ^((gamma&fmulo4)^(delta&fmult3))};
			  o4 <= {o4[8:0], ^((gamma&fmulo5)^(delta&fmult4))};
			  o5 <= {o5[8:0], ^((gamma&fmulo6)^(delta&fmult5))};
			  o6 <= {o6[8:0], ^((gamma&fmulo7)^(delta&fmult6))};
			  o7 <= {o7[8:0], ^(delta&fmult7)};
			end
*/
			if ( enb1 )
			{
				
				L[0] = (L[0]<<1)+_10BitXor((gamma&fmull[0]));
				L[1] = (L[1]<<1)+_10BitXor((gamma&fmull[1])^(delta&fmulb[0]));
				L[2] = (L[2]<<1)+_10BitXor((gamma&fmull[2])^(delta&fmulb[1]));
				L[3] = (L[3]<<1)+_10BitXor((gamma&fmull[3])^(delta&fmulb[2]));
				L[4] = (L[4]<<1)+_10BitXor((gamma&fmull[4])^(delta&fmulb[3]));

				o[0] = (o[0]<<1)+_10BitXor((gamma&fmulo[1])^(delta&fmult[0]));
				o[1] = (o[1]<<1)+_10BitXor((gamma&fmulo[2])^(delta&fmult[1]));
				o[2] = (o[2]<<1)+_10BitXor((gamma&fmulo[3])^(delta&fmult[2]));
				o[3] = (o[3]<<1)+_10BitXor((gamma&fmulo[4])^(delta&fmult[3]));
				o[4] = (o[4]<<1)+_10BitXor((gamma&fmulo[5])^(delta&fmult[4]));
				o[5] = (o[5]<<1)+_10BitXor((gamma&fmulo[6])^(delta&fmult[5]));
				o[6] = (o[6]<<1)+_10BitXor((gamma&fmulo[7])^(delta&fmult[6]));
				o[7] = (o[7]<<1)+_10BitXor((delta&fmult[7]));

			}
		}
		{
/*
			if(enb2)
				begin
				  if(|o0[9:0] && (~K_val[3]))
				  begin
					{B_val4, B_val3, B_val2, B_val1, B_val0} <= {L4, L3, L2, L1, L0};
					{theta3, theta2, theta1, theta0}         <= {o4, o3, o2, o1};
					{theta7, theta6, theta5, theta4}         <= {10'h0, o7, o6, o5};
				  end
				  else
					{B_val4, B_val3, B_val2, B_val1, B_val0} <= {B_val3, B_val2, B_val1, B_val0, 10'h0};
				end
*/
/*
			 if(enb2)
				begin
				  {fmull4, fmull3, fmull2, fmull1, fmull0} <= {L4, L3, L2, L1, L0};
				  {fmulb3, fmulb2, fmulb1, fmulb0}         <= {B_val3, B_val2, B_val1, B_val0};
				  {fmulo3, fmulo2, fmulo1}                 <= {o3, o2, o1};
				  {fmulo7, fmulo6, fmulo5, fmulo4}         <= {o7, o6, o5, o4};
				  {fmult3, fmult2, fmult1, fmult0}         <= {theta3, theta2, theta1, theta0};
				  {fmult7, fmult6, fmult5, fmult4}         <= {theta7, theta6, theta5, theta4};
				  delta <= {(o0[9]^o0[2]), (o0[7]^o0[3]^o0[0]), (o0[8]^o0[4]^o0[1]),
							o0[5], o0[6], o0[7], o0[8], o0[9], (o0[7]^o0[0]), (o0[8]^o0[1])};
				end
			 else if(enb1) // Step of riBM.1: Compute new L(x) and o(x) according to step riBM.1
				begin
				  fmull0 <= {fmull0[8:0], (fmull0[9]^fmull0[6])};
				  fmull1 <= {fmull1[8:0], (fmull1[9]^fmull1[6])};
				  fmull2 <= {fmull2[8:0], (fmull2[9]^fmull2[6])};
				  fmull3 <= {fmull3[8:0], (fmull3[9]^fmull3[6])};
				  fmull4 <= {fmull4[8:0], (fmull4[9]^fmull4[6])};

				  fmulb0 <= {fmulb0[8:0], (fmulb0[9]^fmulb0[6])};
				  fmulb1 <= {fmulb1[8:0], (fmulb1[9]^fmulb1[6])};
				  fmulb2 <= {fmulb2[8:0], (fmulb2[9]^fmulb2[6])};
				  fmulb3 <= {fmulb3[8:0], (fmulb3[9]^fmulb3[6])};

				  fmulo1 <= {fmulo1[8:0], (fmulo1[9]^fmulo1[6])};
				  fmulo2 <= {fmulo2[8:0], (fmulo2[9]^fmulo2[6])};
				  fmulo3 <= {fmulo3[8:0], (fmulo3[9]^fmulo3[6])};
				  fmulo4 <= {fmulo4[8:0], (fmulo4[9]^fmulo4[6])};
				  fmulo5 <= {fmulo5[8:0], (fmulo5[9]^fmulo5[6])};
				  fmulo6 <= {fmulo6[8:0], (fmulo6[9]^fmulo6[6])};
				  fmulo7 <= {fmulo7[8:0], (fmulo7[9]^fmulo7[6])};

				  fmult0 <= {fmult0[8:0], (fmult0[9]^fmult0[6])};
				  fmult1 <= {fmult1[8:0], (fmult1[9]^fmult1[6])};
				  fmult2 <= {fmult2[8:0], (fmult2[9]^fmult2[6])};
				  fmult3 <= {fmult3[8:0], (fmult3[9]^fmult3[6])};
				  fmult4 <= {fmult4[8:0], (fmult4[9]^fmult4[6])};
				  fmult5 <= {fmult5[8:0], (fmult5[9]^fmult5[6])};
				  fmult6 <= {fmult6[8:0], (fmult6[9]^fmult6[6])};
				  fmult7 <= {fmult7[8:0], (fmult7[9]^fmult7[6])};
				end
*/
			if ( enb2 )
			{
				for( i = 0; i <=4; i++ )
					fmull[i] = L[i];
				for( i =0; i <=3; i++ )
					fmulb[i] = B_val[i];
				for( i = 1; i <=7; i++ )
					fmulo[i] = o[i];
				for( i = 0; i <= 7; i ++ )
					fmult[i] = theta[i];

				o[0] &= 0x3ff;
				delta = g_au16RiBmLookup[o[0]];

				if ( o[0] && (!(K_val&0x8)) )
				{
					for( i = 0; i <=4; i++ )
						B_val[i] = L[i];
					for( i = 0; i <= 6; i++ )
						theta[i] = o[i+1];
					theta[7] = 0;
				}
				else
				{
					for( i = 4; i >= 1; i-- )
						B_val[i] = B_val[i-1];
					B_val[0] = 0;
				}
			}
			else if ( enb1 )
			{
				for( i = 0; i <= 4; i++ )
					fmull[i] = (fmull[i]<<1)|(((fmull[i]>>9)^(fmull[i]>>6))&1);

				for( i = 0; i <= 3; i++ )
					fmulb[i] = (fmulb[i]<<1)|(((fmulb[i]>>9)^(fmulb[i]>>6))&1);

				for( i = 1; i <= 7; i++ )
					fmulo[i] = (fmulo[i]<<1)|(((fmulo[i]>>9)^(fmulo[i]>>6))&1);

				for( i = 0; i <= 7; i++ )
					fmult[i] = (fmult[i]<<1)|(((fmult[i]>>9)^(fmult[i]>>6))&1);
			}
		}
		{
/*
			if(riBM_en && (count==4'ha))
				begin
				  if(|delta && (~K_val[3]))
				  begin
					gamma      <= delta;
					K_val[3:0] <= ~K_val[3:0]; // K = -K - 1
				  end
				  else K_val <= K_val + 1'b1;
				end
*/
/*
			if(~riBM_en || (count==4'ha))
			  count <= 4'h0;
			else if(riBM_en)
			  count <= count + 1'b1;
*/
			if ( count == 0xa )
			{
				if ( delta && (!(K_val&(1<<3))) )
				{
					gamma = delta;
					K_val = (~K_val)&0xf;
				}
				else
					K_val ++;
				count = 0;
			}
			else
				count ++;
		}
	}
	for( i = 0; i <= 3; i ++ )
		oo[i] = o[i]&0x3ff;
	

	for( i = 0; i <= 4; i++ )
		L[i] &= 0x3ff;
}


unsigned int _2BitXor(unsigned int x )
{
	unsigned int nXor;

	nXor = x&1;
	nXor = nXor ^ (x>>1);
	return nXor&1;
}

// x: 10 bits
// y: 10 bits
// return z: 10 bits
unsigned int galoisdiv(unsigned int x, unsigned int y)
{
	unsigned int count;				// 5  bits
	unsigned int state;
	unsigned int z;					// 10 bits
	unsigned int div0, div1, _div1;	// 10 bits
	unsigned int div2, divy, _divy;	// 11 bits
	unsigned int clock, nBit10, nBit9, nBit7, nBit5, nBit2, nBit1;
/*
  reg  [10:0] div2, divy;
  reg   [9:0] div0, div1;
  reg         state;
  reg   [4:0] count;
  wire [10:0] _divy;
  wire  [9:0] _div1;
*/
/* init
      state           <= 1'b0;
      {z, div1, div0} <= {x, 10'h0, 10'h0};
      //{divy, div2}    <= {2{11'h0}};
      div2            <= 11'h049;
      divy            <= {y, 1'b0};
*/
	count = 0;
	state = 0;
	z = x;
	div0 = div1 = 0;
	div2 = 0x49;
	divy = y <<1;
	// Need 18 clock to complete
	for( clock = 0; clock < 18; clock++ )
	{
/*
  assign _divy = {divy[9:6], divy[9]^divy[5]^divy[2], divy[4:3], divy[9], divy[1],
                  (divy[10]^divy[7]), (divy[9]^divy[2])};
  assign _div1 = {div1[8:0], (div1[9]^div1[6])};
*/
		nBit10= GetBit(divy,10);
		nBit9 = GetBit(divy,9);
		nBit7 = GetBit(divy,7);
		nBit5 = GetBit(divy,5);
		nBit2 = GetBit(divy,2);
		nBit1 = GetBit(divy,1);
		_divy = ((divy&0x3d8)<<1) | ((nBit9^nBit5^nBit2)<<6) | (nBit9<<3) | (nBit1<<2) | ((nBit10^nBit7)<<1) | (nBit9^nBit2);
		_div1 = ((div1<<1)|(((div1>>9)^(div1>>6))&1))&0x3ff;
/*
      if(~state)
		  begin
			if(_divy[0])
				begin
				  state <= ~state;
				  divy  <= _divy ^ div2;
				  div1  <= z;
				  div2  <= _divy;
				end
			else {div1, divy} <= {_div1, _divy};
		  end
      else
		  begin
			if(~_divy[0])
				begin
				  {div1, divy} <= {_div1, _divy};
				  if(count==1)
					  begin
						z     <= div0 ^ _div1;
						div0  <= z;
						state <= ~state;
					  end
				end
			else
				begin
				  divy <= _divy ^ div2;
				  div1 <= _div1 ^ z;
				  if (count==1)
					  begin
						z     <= div0 ^ _div1 ^ z;
						div0  <= z;
						state <= ~state;
					  end
				end
		  end
*/
/*
    begin
      if(~state)  count <= count + 1'b1;
      else        count <= count - 1'b1;
    end
*/
		if (!state)
		{
			if ( _divy&1 )
			{
				  state = !state;
				  divy  = _divy ^ div2;
				  div1  = z;
				  div2  = _divy;
			}
			else
			{
				div1 = _div1;
				divy = _divy;
			}

			count ++;
		}
		else
		{
			if ( !(_divy&1) )
			{
				div1 = _div1;
				divy = _divy;
				if(count==1)
				{
					unsigned int z_temp;

					z_temp = z;
					z     = div0 ^ _div1;
					div0  = z_temp;
					state = !state;
				}
			}
			else
			{
				divy = _divy ^ div2;
				div1 = _div1 ^ z;
				if (count==1)
				{
					unsigned int z_temp;

					z_temp = z;
					z = div0 ^ _div1 ^ z_temp;
					div0 = z_temp;
					state = !state;
				}
			}
			count --;
		}
	}
	return z;
}

unsigned int correct(unsigned int *L, unsigned int *o, unsigned int *err_addr, unsigned char *err_data)
{
	unsigned int cor_addr;				// 10 bits
	unsigned int err_cnt;				// 3  bits
	unsigned int r0, r1, r2, r3, r4, r5, r6,r7;	// 10 bits
	unsigned galoisdiv_e;
	{
/* init
	
      r0[9] <= L1[9]^L1[8]^L1[5]^L1[4]^L1[2]^L1[1]^L1[0];
      r0[8] <= L1[9]^L1[8]^L1[7]^L1[6]^L1[4]^L1[3]^L1[1]^L1[0];
      r0[7] <= L1[9]^L1[8]^L1[7]^L1[5]^L1[3]^L1[2]^L1[0];
      r0[6] <= L1[9]^L1[8]^L1[7]^L1[4]^L1[2]^L1[1];
      r0[5] <= L1[8]^L1[7]^L1[6]^L1[3]^L1[1]^L1[0];
      r0[4] <= L1[9]^L1[7]^L1[5]^L1[2]^L1[0];
      r0[3] <= L1[9]^L1[8]^L1[4]^L1[1];
      r0[2] <= L1[8]^L1[7]^L1[3]^L1[0];
      r0[1] <= L1[9]^L1[7]^L1[2];
      r0[0] <= L1[8]^L1[6]^L1[1];

      r1[9] <= L2[8]^L2[7]^L2[6]^L2[3]^L2[2]^L2[0];
      r1[8] <= L2[9]^L2[7]^L2[5]^L2[2]^L2[1];
      r1[7] <= L2[8]^L2[6]^L2[4]^L2[1]^L2[0];
      r1[6] <= L2[9]^L2[7]^L2[6]^L2[5]^L2[3]^L2[0];
      r1[5] <= L2[9]^L2[8]^L2[5]^L2[4]^L2[2];
      r1[4] <= L2[8]^L2[7]^L2[4]^L2[3]^L2[1];
      r1[3] <= L2[7]^L2[6]^L2[3]^L2[2]^L2[0];
      r1[2] <= L2[9]^L2[5]^L2[2]^L2[1];
      r1[1] <= L2[8]^L2[4]^L2[1]^L2[0];
      r1[0] <= L2[9]^L2[7]^L2[6]^L2[3]^L2[0];

      r2[9] <= L3[5]^L3[4]^L3[3]^L3[0];
      r2[8] <= L3[9]^L3[6]^L3[4]^L3[3]^L3[2];
      r2[7] <= L3[8]^L3[5]^L3[3]^L3[2]^L3[1];
      r2[6] <= L3[7]^L3[4]^L3[2]^L3[1]^L3[0];
      r2[5] <= L3[9]^L3[3]^L3[1]^L3[0];
      r2[4] <= L3[9]^L3[8]^L3[6]^L3[2]^L3[0];
      r2[3] <= L3[9]^L3[8]^L3[7]^L3[6]^L3[5]^L3[1];
      r2[2] <= L3[8]^L3[7]^L3[6]^L3[5]^L3[4]^L3[0];
      r2[1] <= L3[9]^L3[7]^L3[5]^L3[4]^L3[3];
      r2[0] <= L3[8]^L3[6]^L3[4]^L3[3]^L3[2];

      r3[9] <= L4[8]^L4[5]^L4[4]^L4[3]^L4[2]^L4[1];
      r3[8] <= L4[7]^L4[4]^L4[3]^L4[2]^L4[1]^L4[0];
      r3[7] <= L4[9]^L4[3]^L4[2]^L4[1]^L4[0];
      r3[6] <= L4[9]^L4[8]^L4[6]^L4[2]^L4[1]^L4[0];
      r3[5] <= L4[9]^L4[8]^L4[7]^L4[6]^L4[5]^L4[1]^L4[0];
      r3[4] <= L4[9]^L4[8]^L4[7]^L4[5]^L4[4]^L4[0];
      r3[3] <= L4[9]^L4[8]^L4[7]^L4[4]^L4[3];
      r3[2] <= L4[8]^L4[7]^L4[6]^L4[3]^L4[2];
      r3[1] <= L4[7]^L4[6]^L4[5]^L4[2]^L4[1];
      r3[0] <= L4[6]^L4[5]^L4[4]^L4[1]^L4[0];

      r4[9] <= o0[9]^o0[5]^o0[3];
      r4[8] <= o0[8]^o0[4]^o0[2];
      r4[7] <= o0[7]^o0[3]^o0[1];
      r4[6] <= o0[6]^o0[2]^o0[0];
      r4[5] <= o0[9]^o0[6]^o0[5]^o0[1];
      r4[4] <= o0[8]^o0[5]^o0[4]^o0[0];
      r4[3] <= o0[9]^o0[7]^o0[6]^o0[4]^o0[3];
      r4[2] <= o0[8]^o0[6]^o0[5]^o0[3]^o0[2];
      r4[1] <= o0[7]^o0[5]^o0[4]^o0[2]^o0[1];
      r4[0] <= o0[6]^o0[4]^o0[3]^o0[1]^o0[0];

      r5[9] <= o1[8]^o1[7]^o1[6]^o1[5]^o1[3]^o1[2]^o1[1];
      r5[8] <= o1[7]^o1[6]^o1[5]^o1[4]^o1[2]^o1[1]^o1[0];
      r5[7] <= o1[9]^o1[5]^o1[4]^o1[3]^o1[1]^o1[0];
      r5[6] <= o1[9]^o1[8]^o1[6]^o1[4]^o1[3]^o1[2]^o1[0];
      r5[5] <= o1[9]^o1[8]^o1[7]^o1[6]^o1[5]^o1[3]^o1[2]^o1[1];
      r5[4] <= o1[8]^o1[7]^o1[6]^o1[5]^o1[4]^o1[2]^o1[1]^o1[0];
      r5[3] <= o1[9]^o1[7]^o1[5]^o1[4]^o1[3]^o1[1]^o1[0];
      r5[2] <= o1[9]^o1[8]^o1[4]^o1[3]^o1[2]^o1[0];
      r5[1] <= o1[9]^o1[8]^o1[7]^o1[6]^o1[3]^o1[2]^o1[1];
      r5[0] <= o1[8]^o1[7]^o1[6]^o1[5]^o1[2]^o1[1]^o1[0];

      r6[9] <= o2[9]^o2[5]^o2[4]^o2[2];
      r6[8] <= o2[8]^o2[4]^o2[3]^o2[1];
      r6[7] <= o2[7]^o2[3]^o2[2]^o2[0];
      r6[6] <= o2[9]^o2[2]^o2[1];
      r6[5] <= o2[8]^o2[1]^o2[0];
      r6[4] <= o2[9]^o2[7]^o2[6]^o2[0];
      r6[3] <= o2[9]^o2[8]^o2[5];
      r6[2] <= o2[8]^o2[7]^o2[4];
      r6[1] <= o2[7]^o2[6]^o2[3];
      r6[0] <= o2[6]^o2[5]^o2[2];

      r7[9] <= o3[8]^o3[7]^o3[6]^o3[4];
      r7[8] <= o3[7]^o3[6]^o3[5]^o3[3];
      r7[7] <= o3[6]^o3[5]^o3[4]^o3[2];
      r7[6] <= o3[5]^o3[4]^o3[3]^o3[1];
      r7[5] <= o3[4]^o3[3]^o3[2]^o3[0];
      r7[4] <= o3[9]^o3[6]^o3[3]^o3[2]^o3[1];
      r7[3] <= o3[8]^o3[5]^o3[2]^o3[1]^o3[0];
      r7[2] <= o3[9]^o3[7]^o3[6]^o3[4]^o3[1]^o3[0];
      r7[1] <= o3[9]^o3[8]^o3[5]^o3[3]^o3[0];
      r7[0] <= o3[9]^o3[8]^o3[7]^o3[6]^o3[4]^o3[2];
*/
		
		r0 = g_au16CorrectInitLookupR0[L[1]];
		r1 = g_au16CorrectInitLookupR1[L[2]];
		r2 = g_au16CorrectInitLookupR2[L[3]];
		r3 = g_au16CorrectInitLookupR3[L[4]];
		r4 = g_au16CorrectInitLookupR4[o[0]];
		r5 = g_au16CorrectInitLookupR5[o[1]];
		r6 = g_au16CorrectInitLookupR6[o[2]];
		r7 = g_au16CorrectInitLookupR7[o[3]];
		cor_addr = err_cnt = 0;
	}
	do
	{
		if ( !(L[0]^r0^r1^r2^r3)  )
		{
			// error find
			galoisdiv_e = galoisdiv( r4^r5^r6^r7, r0^r2 );
/*

      if(^galoisdiv_e[1:0] | (&err_cnt))
        err_cnt <= 3'd7;
      else
        err_cnt <= err_cnt + 1'b1;
  // cor_data
  assign cor_data = galoisdiv_e[9:2];
*/
			if (err_cnt<4)
			{
				err_addr[err_cnt] = cor_addr - 2;
				err_data[err_cnt] = galoisdiv_e>>2;
			}
			if ( _2BitXor(galoisdiv_e) | (err_cnt == 7) )
				err_cnt = 7;
			else
				err_cnt ++;
		}
/*
		if(chk_en)
			begin
			  r0 <= {r0[8:0], r0[9]^r0[6]};
			  r1 <= {r1[7:0], (r1[9]^r1[6]), (r1[8]^r1[5])};
			  r2 <= {r2[6:0], (r2[9]^r2[6]), (r2[8]^r2[5]), (r2[7]^r2[4])};
			  r3 <= {r3[5:0], (r3[9]^r3[6]), (r3[8]^r3[5]), (r3[7]^r3[4]), (r3[6]^r3[3])};
			  r4 <= {(r4[6]^r4[1]), (r4[5]^r4[0]), (r4[9]^r4[6]^r4[4]), (r4[8]^r4[5]^r4[3]),
					 (r4[7]^r4[4]^r4[2]), (r4[6]^r4[3]^r4[1]), (r4[5]^r4[2]^r4[0]),
					 (r4[9]^r4[6]^r4[4]^r4[1]), (r4[8]^r4[5]^r4[3]^r4[0]), (r4[9]^r4[7]^r4[6]^r4[4]^r4[2])};
			  r5 <= {(r5[5]^r5[0]), (r5[9]^r5[6]^r5[4]), (r5[8]^r5[5]^r5[3]), (r5[7]^r5[4]^r5[2]),
					 (r5[6]^r5[3]^r5[1]), (r5[5]^r5[2]^r5[0]), (r5[9]^r5[6]^r5[4]^r5[1]),
					 (r5[8]^r5[5]^r5[3]^r5[0]), (r5[9]^r5[7]^r5[6]^r5[4]^r5[2]),
					 (r5[8]^r5[6]^r5[5]^r5[3]^r5[1])};
			  r6 <= {(r6[9]^r6[6]^r6[4]), (r6[8]^r6[5]^r6[3]), (r6[7]^r6[4]^r6[2]), (r6[6]^r6[3]^r6[1]),
					 (r6[5]^r6[2]^r6[0]), (r6[9]^r6[6]^r6[4]^r6[1]), (r6[8]^r6[5]^r6[3]^r6[0]),
					 (r6[9]^r6[7]^r6[6]^r6[4]^r6[2]), (r6[8]^r6[6]^r6[5]^r6[3]^r6[1]),
					 (r6[7]^r6[5]^r6[4]^r6[2]^r6[0])};
			  r7 <= {(r7[8]^r7[5]^r7[3]), (r7[7]^r7[4]^r7[2]), (r7[6]^r7[3]^r7[1]), (r7[5]^r7[2]^r7[0]),
					 (r7[9]^r7[6]^r7[4]^r7[1]), (r7[8]^r7[5]^r7[3]^r7[0]), (r7[9]^r7[7]^r7[6]^r7[4]^r7[2]),
					 (r7[8]^r7[6]^r7[5]^r7[3]^r7[1]), (r7[7]^r7[5]^r7[4]^r7[2]^r7[0]), (r7[9]^r7[4]^r7[3]^r7[1])};
			end
*/
		if ( cor_addr != (512+1) )//if ( cor_addr != (512+2) )
		{
			r0 = ((r0<<1)|g_au8CorrectLookupR0[r0])&0x3ff;
			r1 = ((r1<<2)|g_au8CorrectLookupR1[r1])&0x3ff;
			r2 = ((r2<<3)|g_au8CorrectLookupR2[r2])&0x3ff;
			r3 = ((r3<<4)|g_au8CorrectLookupR3[r3])&0x3ff;
			r4 = g_au16CorrectLookupR4[r4];
			r5 = g_au16CorrectLookupR5[r5];
			r6 = g_au16CorrectLookupR6[r6];
			r7 = g_au16CorrectLookupR7[r7];
		}
		else
			break;
		cor_addr ++;
	}while(1);

	return err_cnt;
}
