#include "stdafx.h"

/* C O N S T A N T S */
// bits per sample for chosen quantizer
const unsigned int  Res_bit [18] = {
    0,  0,  0,  0,  0,  0,  0,  0,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16
};

// coefficients for requantization
// 65536/step bzw. 65536/(2*D+1)



#define _(X) MAKE_MPC_SAMPLE_EX(X,14)

const MPC_SAMPLE_FORMAT  __Cc [1 + 18] = {
      _(111.285962475327f),                                        // 32768/2/255*sqrt(3)
    _(65536.000000000000f), _(21845.333333333332f), _(13107.200000000001f), _(9362.285714285713f),
    _(7281.777777777777f),  _(4369.066666666666f),  _(2114.064516129032f), _(1040.253968253968f),
     _(516.031496062992f),  _(257.003921568627f),   _(128.250489236790f),   _(64.062561094819f),
       _(32.015632633121f),    _(16.003907203907f),     _(8.000976681723f),    _(4.000244155527f),
        _(2.000061037018f),     _(1.000015259021f)
};

#undef _

// offset for requantization
// 2*D+1 = steps of quantizer
const int  __Dc [1 + 18] = {
      2,
      0,     1,     2,     3,     4,     7,    15,    31,    63,
    127,   255,   511,  1023,  2047,  4095,  8191, 16383, 32767
};

static unsigned find_shift(double fval)
{
	MPC_INT64 val = (MPC_INT64)fval;
	if (val<0) val = -val;
	unsigned ptr = 0;
	while(val) {val>>=1;ptr++;}

	return ptr > 31 ? 0 : 31 - ptr;
}

/* F U N C T I O N S */

#define SET_SCF(N,X) SCF[N] = MAKE_MPC_SAMPLE_EX(X,SCF_shift[N] = find_shift(X));

void
MPC_decoder::ScaleOutput ( double factor )
{
#ifndef MPC_FIXED_POINT
	factor *= 1.0 / (double)(1<<(MPC_FIXED_POINT_SHIFT-1));
#else
	factor *= 1.0 / (double)(1<<(16 - MPC_FIXED_POINT_SHIFT));
#endif
    int     n;
    double  f1 = factor;
    double  f2 = factor;

    // handles +1.58...-98.41 dB, where's scf[n] / scf[n-1] = 1.20050805774840750476
	
	SET_SCF(1,factor);

	f1 *=   0.83298066476582673961;
	f2 *= 1/0.83298066476582673961;

    for ( n = 1; n <= 128; n++ ) {
		SET_SCF((unsigned char)(1+n),f1);
		SET_SCF((unsigned char)(1-n),f2);
        f1 *=   0.83298066476582673961;
        f2 *= 1/0.83298066476582673961;
    }
}

void
MPC_decoder::Quantisierungsmodes ( void )            // conversion: index -> quantizer (bitstream reading)
{                                       // conversion: quantizer -> index (bitstream writing)
    int  Band = 0;
    int  i;

    do {
        Q_bit [Band] = 4;
        for ( i = 0; i < 16-1; i++ )
            Q_res [Band] [i] = i;
        Q_res [Band][i] = 17;
        Band++;
    } while ( Band < 11 );

    do {
        Q_bit [Band] = 3;
        for ( i = 0; i < 8-1; i++ )
            Q_res [Band] [i] = i;
        Q_res [Band] [i] = 17;
        Band++;
    } while ( Band < 23 );

    do {
        Q_bit [Band] = 2;
        for ( i = 0; i < 4-1; i++ )
            Q_res [Band] [i] = i;
        Q_res [Band] [i] = 17;
        Band++;
    } while ( Band < 32 );
}

void
MPC_decoder::initialisiere_Quantisierungstabellen ( double scale_factor )
{
    Quantisierungsmodes ();
    ScaleOutput ( scale_factor );
}
