#include "stdafx.h"

void
MPC_decoder::RESET_Synthesis ( void )
{
    Reset_V ();
}

void
MPC_decoder::RESET_Y ( void )
{
    memset ( Y_L, 0, sizeof Y_L );
    memset ( Y_R, 0, sizeof Y_R );
}

void
MPC_decoder::RESET_Globals ( void )
{
    Reset_BitstreamDecode ();

    DecodedFrames  = 0;
    StreamVersion  = 0;
    MS_used        = 0;

    memset ( Y_L             , 0, sizeof Y_L              );
    memset ( Y_R             , 0, sizeof Y_R              );
    memset ( SCF_Index_L     , 0, sizeof SCF_Index_L      );
    memset ( SCF_Index_R     , 0, sizeof SCF_Index_R      );
    memset ( Res_L           , 0, sizeof Res_L            );
    memset ( Res_R           , 0, sizeof Res_R            );
    memset ( SCFI_L          , 0, sizeof SCFI_L           );
    memset ( SCFI_R          , 0, sizeof SCFI_R           );
    memset ( DSCF_Flag_L     , 0, sizeof DSCF_Flag_L      );
    memset ( DSCF_Flag_R     , 0, sizeof DSCF_Flag_R      );
    memset ( DSCF_Reference_L, 0, sizeof DSCF_Reference_L );
    memset ( DSCF_Reference_R, 0, sizeof DSCF_Reference_R );
    memset ( Q               , 0, sizeof Q                );
    memset ( MS_Flag         , 0, sizeof MS_Flag          );
}

unsigned int
MPC_decoder::decode_internal ( MPC_SAMPLE_FORMAT *buffer )
{
	unsigned int output_frame_length = FrameLength;

	unsigned int  FrameBitCnt = 0;

	if ( DecodedFrames >= OverallFrames )
		return (unsigned int)(-1);                           // end of file -> abort decoding

	// read jump-info for validity check of frame
	FwdJumpInfo  = Bitstream_read (20);
	SeekTable [DecodedFrames] = 20 + FwdJumpInfo;       // ...

	ActDecodePos = (Zaehler << 5) + pos;

	// decode data and check for validity of frame
	FrameBitCnt = BitsRead ();
	switch ( StreamVersion ) {
	case 0x04:
	case 0x05:
	case 0x06:
		Lese_Bitstrom_SV6 ();
		break;
	case 0x07:
	case 0x17:
		Lese_Bitstrom_SV7 ();
		break;
	default:
		return (unsigned int)(-1);
	}
	FrameWasValid = BitsRead() - FrameBitCnt == FwdJumpInfo;

	// synthesize signal
	Requantisierung ( Max_Band );

	//if ( EQ_activated && PluginSettings.EQbyMPC )
	//    perform_EQ ();

	Synthese_Filter_float ( buffer );


	DecodedFrames++;

	// cut off first SynthDelay zero-samples
	if ( DecodedFrames == OverallFrames  &&  StreamVersion >= 6 )
	{        // reconstruct exact filelength
		int  mod_block   = Bitstream_read (11);
		int  FilterDecay;

		if (mod_block == 0) mod_block = 1152;                    // Encoder bugfix
		FilterDecay = (mod_block + SynthDelay) % FrameLength;

		// additional FilterDecay samples are needed for decay of synthesis filter
		if ( SynthDelay + mod_block >= FrameLength ) {

			// **********************************************************************
			// Rhoades 4/16/2002
			// Commented out are blocks of code which cause gapless playback to fail.
			// Temporary fix...
			// **********************************************************************

			if ( ! TrueGaplessPresent ) {
				RESET_Y ();
			} else {
				//if ( FrameLength != LastValidSamples ) {
					Bitstream_read (20);
					Lese_Bitstrom_SV7 ();
					Requantisierung ( Max_Band );
					//FilterDecay = LastValidSamples;
				//}
				//else {
					//FilterDecay = 0;
				//}
			}

			Synthese_Filter_float ( buffer + 2304 );

			output_frame_length = FrameLength + FilterDecay;
		}
		else {                              // there are only FilterDecay samples needed for this frame
			output_frame_length = FilterDecay;
		}
	}

	if ( samples_to_skip ) {
		if (output_frame_length < samples_to_skip)
		{
			samples_to_skip -= output_frame_length;
			output_frame_length = 0;
		}
		else
		{
			output_frame_length -= samples_to_skip;
			memmove ( buffer, buffer + samples_to_skip * 2, output_frame_length * 2 * sizeof (MPC_SAMPLE_FORMAT) );
			samples_to_skip = 0;
		}
	}

	return output_frame_length;

}

unsigned int MPC_decoder::Decode ( MPC_SAMPLE_FORMAT *buffer, unsigned int * vbr_update_acc, unsigned int * vbr_update_bits )
{
    for(;;)
    {
        const int MaxBrokenFrames = 0; // PluginSettings.MaxBrokenFrames

        unsigned int RING = Zaehler;
        int vbr_ring = (RING << 5) + pos;

        unsigned int valid_samples = decode_internal ( buffer );

        if ( valid_samples == (unsigned int)(-1) ) return 0;

        /**************** ERROR CONCEALMENT *****************/
        if ( FrameWasValid == 0 ) {                                   // error occurred in bitstream
            return (unsigned int)(-1);
        } else
		{
			if (vbr_update_acc && vbr_update_bits)
			{
				(*vbr_update_acc) ++;
				vbr_ring = (Zaehler << 5) + pos - vbr_ring;
				if (vbr_ring < 0) vbr_ring += 524288;
				(*vbr_update_bits) += vbr_ring;
			}

		}
		UpdateBuffer ( RING );

		if (valid_samples > 0) return valid_samples;
	}
}

void
MPC_decoder::Requantisierung ( const int Last_Band )
{
    int     Band;
    int     n;
    MPC_SAMPLE_FORMAT facL;
    MPC_SAMPLE_FORMAT facR;
    MPC_SAMPLE_FORMAT templ;
    MPC_SAMPLE_FORMAT tempr;
    MPC_SAMPLE_FORMAT* YL;
    MPC_SAMPLE_FORMAT* YR;
    int*    L;
    int*    R;

#ifdef MPC_FIXED_POINT
#if MPC_FIXED_POINT_FRACTPART == 14
#define MPC_MULTIPLY_SCF(CcVal, SCF_idx) \
	MPC_MULTIPLY_EX(CcVal,SCF[SCF_idx],SCF_shift[SCF_idx])
#else

#error FIXME, Cc table is in 18.14 format

#endif
#else
#define MPC_MULTIPLY_SCF(CcVal, SCF_idx) \
	MPC_MULTIPLY(CcVal,SCF[SCF_idx])
#endif
    // requantization and scaling of subband-samples
    for ( Band = 0; Band <= Last_Band; Band++ ) {   // setting pointers
        YL = Y_L [0] + Band;
        YR = Y_R [0] + Band;
        L  = Q [Band].L;
        R  = Q [Band].R;
        /************************** MS-coded **************************/
        if ( MS_Flag [Band] ) {
            if ( Res_L [Band] ) {
                if ( Res_R [Band] ) {    // M!=0, S!=0
                    facL = MPC_MULTIPLY_SCF( Cc[Res_L[Band]] , (unsigned char)SCF_Index_L[Band][0]);
                    facR = MPC_MULTIPLY_SCF( Cc[Res_R[Band]] , (unsigned char)SCF_Index_R[Band][0]);
                    for ( n = 0; n < 12; n++, YL += 32, YR += 32 ) {
                        *YL   = (templ = MPC_MULTIPLY_FLOAT_INT(facL,*L++))+(tempr = MPC_MULTIPLY_FLOAT_INT(facR,*R++));
                        *YR   = templ - tempr;
                    }
                    facL = MPC_MULTIPLY_SCF( Cc[Res_L[Band]] , (unsigned char)SCF_Index_L[Band][1]);
                    facR = MPC_MULTIPLY_SCF( Cc[Res_R[Band]] , (unsigned char)SCF_Index_R[Band][1]);
                    for ( ; n < 24; n++, YL += 32, YR += 32 ) {
                        *YL   = (templ = MPC_MULTIPLY_FLOAT_INT(facL,*L++))+(tempr = MPC_MULTIPLY_FLOAT_INT(facR,*R++));
                        *YR   = templ - tempr;
                    }
                    facL = MPC_MULTIPLY_SCF( Cc[Res_L[Band]] , (unsigned char)SCF_Index_L[Band][2]);
                    facR = MPC_MULTIPLY_SCF( Cc[Res_R[Band]] , (unsigned char)SCF_Index_R[Band][2]);
                    for ( ; n < 36; n++, YL += 32, YR += 32 ) {
                        *YL   = (templ = MPC_MULTIPLY_FLOAT_INT(facL,*L++))+(tempr = MPC_MULTIPLY_FLOAT_INT(facR,*R++));
                        *YR   = templ - tempr;
                    }
                } else {    // M!=0, S==0
                    facL = MPC_MULTIPLY_SCF( Cc[Res_L[Band]] , (unsigned char)SCF_Index_L[Band][0]);
                    for ( n = 0; n < 12; n++, YL += 32, YR += 32 ) {
						*YR = *YL = MPC_MULTIPLY_FLOAT_INT(facL,*L++);
                    }
                    facL = MPC_MULTIPLY_SCF( Cc[Res_L[Band]] , (unsigned char)SCF_Index_L[Band][1]);
                    for ( ; n < 24; n++, YL += 32, YR += 32 ) {
                        *YR = *YL = MPC_MULTIPLY_FLOAT_INT(facL,*L++);
                    }
                    facL = MPC_MULTIPLY_SCF( Cc[Res_L[Band]] , (unsigned char)SCF_Index_L[Band][2]);
                    for ( ; n < 36; n++, YL += 32, YR += 32 ) {
                        *YR = *YL = MPC_MULTIPLY_FLOAT_INT(facL,*L++);
                    }
                }
            } else {
                if (Res_R[Band])    // M==0, S!=0
                {
                    facR = MPC_MULTIPLY_SCF( Cc[Res_R[Band]] , (unsigned char)SCF_Index_R[Band][0]);
                    for ( n = 0; n < 12; n++, YL += 32, YR += 32 ) {
                        *YR = - (*YL = MPC_MULTIPLY_FLOAT_INT(facR,*(R++)));
                    }
                    facR = MPC_MULTIPLY_SCF( Cc[Res_R[Band]] , (unsigned char)SCF_Index_R[Band][1]);
                    for ( ; n < 24; n++, YL += 32, YR += 32 ) {
                        *YR = - (*YL = MPC_MULTIPLY_FLOAT_INT(facR,*(R++)));
                    }
                    facR = MPC_MULTIPLY_SCF( Cc[Res_R[Band]] , (unsigned char)SCF_Index_R[Band][2]);
                    for ( ; n < 36; n++, YL += 32, YR += 32 ) {
                        *YR = - (*YL = MPC_MULTIPLY_FLOAT_INT(facR,*(R++)));
                    }
                } else {    // M==0, S==0
                    for ( n = 0; n < 36; n++, YL += 32, YR += 32 ) {
                        *YR = *YL = 0;
                    }
                }
            }
        }
        /************************** LR-coded **************************/
        else {
            if ( Res_L [Band] ) {
                if ( Res_R [Band] ) {    // L!=0, R!=0
                    facL = MPC_MULTIPLY_SCF( Cc[Res_L[Band]] , (unsigned char)SCF_Index_L[Band][0]);
                    facR = MPC_MULTIPLY_SCF( Cc[Res_R[Band]] , (unsigned char)SCF_Index_R[Band][0]);
                    for (n = 0; n < 12; n++, YL += 32, YR += 32 ) {
                        *YL = MPC_MULTIPLY_FLOAT_INT(facL,*L++);
                        *YR = MPC_MULTIPLY_FLOAT_INT(facR,*R++);
                    }
                    facL = MPC_MULTIPLY_SCF( Cc[Res_L[Band]] , (unsigned char)SCF_Index_L[Band][1]);
                    facR = MPC_MULTIPLY_SCF( Cc[Res_R[Band]] , (unsigned char)SCF_Index_R[Band][1]);
                    for (; n < 24; n++, YL += 32, YR += 32 ) {
                        *YL = MPC_MULTIPLY_FLOAT_INT(facL,*L++);
                        *YR = MPC_MULTIPLY_FLOAT_INT(facR,*R++);
                    }
                    facL = MPC_MULTIPLY_SCF( Cc[Res_L[Band]] , (unsigned char)SCF_Index_L[Band][2]);
                    facR = MPC_MULTIPLY_SCF( Cc[Res_R[Band]] , (unsigned char)SCF_Index_R[Band][2]);
                    for (; n < 36; n++, YL += 32, YR += 32 ) {
                        *YL = MPC_MULTIPLY_FLOAT_INT(facL,*L++);
                        *YR = MPC_MULTIPLY_FLOAT_INT(facR,*R++);
                    }
                } else {     // L!=0, R==0
                    facL = MPC_MULTIPLY_SCF( Cc[Res_L[Band]] , (unsigned char)SCF_Index_L[Band][0]);
                    for ( n = 0; n < 12; n++, YL += 32, YR += 32 ) {
                        *YL = MPC_MULTIPLY_FLOAT_INT(facL,*L++);
                        *YR = 0;
                    }
                    facL = MPC_MULTIPLY_SCF( Cc[Res_L[Band]] , (unsigned char)SCF_Index_L[Band][1]);
                    for ( ; n < 24; n++, YL += 32, YR += 32 ) {
                        *YL = MPC_MULTIPLY_FLOAT_INT(facL,*L++);
                        *YR = 0;
                    }
                    facL = MPC_MULTIPLY_SCF( Cc[Res_L[Band]] , (unsigned char)SCF_Index_L[Band][2]);
                    for ( ; n < 36; n++, YL += 32, YR += 32 ) {
                        *YL = MPC_MULTIPLY_FLOAT_INT(facL,*L++);
                        *YR = 0;
                    }
                }
            }
            else {
                if ( Res_R [Band] ) {    // L==0, R!=0
                    facR = MPC_MULTIPLY_SCF( Cc[Res_R[Band]] , (unsigned char)SCF_Index_R[Band][0]);
                    for ( n = 0; n < 12; n++, YL += 32, YR += 32 ) {
                        *YL = 0;
                        *YR = MPC_MULTIPLY_FLOAT_INT(facR,*R++);
                    }
                    facR = MPC_MULTIPLY_SCF( Cc[Res_R[Band]] , (unsigned char)SCF_Index_R[Band][1]);
                    for ( ; n < 24; n++, YL += 32, YR += 32 ) {
                        *YL = 0;
                        *YR = MPC_MULTIPLY_FLOAT_INT(facR,*R++);
                    }
                    facR = MPC_MULTIPLY_SCF( Cc[Res_R[Band]] , (unsigned char)SCF_Index_R[Band][2]);
                    for ( ; n < 36; n++, YL += 32, YR += 32 ) {
                        *YL = 0;
                        *YR = MPC_MULTIPLY_FLOAT_INT(facR,*R++);
                    }
                } else {    // L==0, R==0
                    for ( n = 0; n < 36; n++, YL += 32, YR += 32 ) {
                        *YR = *YL = 0;
                    }
                }
            }
        }
    }
}

/****************************************** SV 6 ******************************************/
void
MPC_decoder::Lese_Bitstrom_SV6 ( void )
{
    int n,k;
    int Max_used_Band=0;
    HuffmanTyp *Table;
    const HuffmanTyp *x1;
    const HuffmanTyp *x2;
    int *L;
    int *R;
    int *ResL = Res_L;
    int *ResR = Res_R;

    /************************ HEADER **************************/
    ResL = Res_L;
    ResR = Res_R;
    for (n=0; n<=Max_Band; ++n, ++ResL, ++ResR)
    {
        if      (n<11)           Table = Region_A;
        else if (n>=11 && n<=22) Table = Region_B;
        else /*if (n>=23)*/      Table = Region_C;

        *ResL = Q_res[n][Huffman_Decode(Table)];
        if (MS_used)      MS_Flag[n] = Bitstream_read(1);
        *ResR = Q_res[n][Huffman_Decode(Table)];

        // only perform the following procedure up to the maximum non-zero subband
        if (*ResL || *ResR) Max_used_Band = n;
    }

    /************************* SCFI-Bundle *****************************/
    ResL = Res_L;
    ResR = Res_R;
    for (n=0; n<=Max_used_Band; ++n, ++ResL, ++ResR) {
        if (*ResL) SCFI_Bundle_read(SCFI_Bundle, &SCFI_L[n], &DSCF_Flag_L[n]);
        if (*ResR) SCFI_Bundle_read(SCFI_Bundle, &SCFI_R[n], &DSCF_Flag_R[n]);
    }

    /***************************** SCFI ********************************/
    ResL = Res_L;
    ResR = Res_R;
    L    = SCF_Index_L[0];
    R    = SCF_Index_R[0];
    for (n=0; n<=Max_used_Band; ++n, ++ResL, ++ResR, L+=3, R+=3)
    {
        if (*ResL)
        {
            /*********** DSCF ************/
            if (DSCF_Flag_L[n]==1)
            {
                L[2] = DSCF_Reference_L[n];
                switch (SCFI_L[n])
                {
                case 3:
                    L[0] = L[2] + Huffman_Decode_fast(DSCF_Entropie);
                    L[1] = L[0];
                    L[2] = L[1];
                    break;
                case 1:
                    L[0] = L[2] + Huffman_Decode_fast(DSCF_Entropie);
                    L[1] = L[0] + Huffman_Decode_fast(DSCF_Entropie);
                    L[2] = L[1];
                    break;
                case 2:
                    L[0] = L[2] + Huffman_Decode_fast(DSCF_Entropie);
                    L[1] = L[0];
                    L[2] = L[1] + Huffman_Decode_fast(DSCF_Entropie);
                    break;
                case 0:
                    L[0] = L[2] + Huffman_Decode_fast(DSCF_Entropie);
                    L[1] = L[0] + Huffman_Decode_fast(DSCF_Entropie);
                    L[2] = L[1] + Huffman_Decode_fast(DSCF_Entropie);
                    break;
                default:
                    return;
                    break;
                }
            }
            /************ SCF ************/
            else
            {
                switch (SCFI_L[n])
                {
                case 3:
                    L[0] = Bitstream_read(6);
                    L[1] = L[0];
                    L[2] = L[1];
                    break;
                case 1:
                    L[0] = Bitstream_read(6);
                    L[1] = Bitstream_read(6);
                    L[2] = L[1];
                    break;
                case 2:
                    L[0] = Bitstream_read(6);
                    L[1] = L[0];
                    L[2] = Bitstream_read(6);
                    break;
                case 0:
                    L[0] = Bitstream_read(6);
                    L[1] = Bitstream_read(6);
                    L[2] = Bitstream_read(6);
                    break;
                default:
                    return;
                    break;
                }
            }
            // update Reference for DSCF
            DSCF_Reference_L[n] = L[2];
        }
        if (*ResR)
        {
            R[2] = DSCF_Reference_R[n];
            /*********** DSCF ************/
            if (DSCF_Flag_R[n]==1)
            {
                switch (SCFI_R[n])
                {
                case 3:
                    R[0] = R[2] + Huffman_Decode_fast(DSCF_Entropie);
                    R[1] = R[0];
                    R[2] = R[1];
                    break;
                case 1:
                    R[0] = R[2] + Huffman_Decode_fast(DSCF_Entropie);
                    R[1] = R[0] + Huffman_Decode_fast(DSCF_Entropie);
                    R[2] = R[1];
                    break;
                case 2:
                    R[0] = R[2] + Huffman_Decode_fast(DSCF_Entropie);
                    R[1] = R[0];
                    R[2] = R[1] + Huffman_Decode_fast(DSCF_Entropie);
                    break;
                case 0:
                    R[0] = R[2] + Huffman_Decode_fast(DSCF_Entropie);
                    R[1] = R[0] + Huffman_Decode_fast(DSCF_Entropie);
                    R[2] = R[1] + Huffman_Decode_fast(DSCF_Entropie);
                    break;
                default:
                    return;
                    break;
                }
            }
            /************ SCF ************/
            else
            {
                switch (SCFI_R[n])
                {
                case 3:
                    R[0] = Bitstream_read(6);
                    R[1] = R[0];
                    R[2] = R[1];
                    break;
                case 1:
                    R[0] = Bitstream_read(6);
                    R[1] = Bitstream_read(6);
                    R[2] = R[1];
                    break;
                case 2:
                    R[0] = Bitstream_read(6);
                    R[1] = R[0];
                    R[2] = Bitstream_read(6);
                    break;
                case 0:
                    R[0] = Bitstream_read(6);
                    R[1] = Bitstream_read(6);
                    R[2] = Bitstream_read(6);
                    break;
                default:
                    return;
                    break;
                }
            }
            // update Reference for DSCF
            DSCF_Reference_R[n] = R[2];
        }
    }

    /**************************** Samples ****************************/
    ResL = Res_L;
    ResR = Res_R;
    for (n=0; n<=Max_used_Band; ++n, ++ResL, ++ResR)
    {
        // setting pointers
        x1 = SampleHuff[*ResL];
        x2 = SampleHuff[*ResR];
        L = Q[n].L;
        R = Q[n].R;

        if (x1!=NULL || x2!=NULL)
            for (k=0; k<36; ++k)
            {
                if (x1 != NULL) *L++ = Huffman_Decode_fast (x1);
                if (x2 != NULL) *R++ = Huffman_Decode_fast (x2);
            }

        if (*ResL>7 || *ResR>7)
            for (k=0; k<36; ++k)
            {
                if (*ResL>7) *L++ = (int)Bitstream_read(Res_bit[*ResL]) - Dc[*ResL];
                if (*ResR>7) *R++ = (int)Bitstream_read(Res_bit[*ResR]) - Dc[*ResR];
            }
    }
}

/****************************************** SV 7 ******************************************/
void
MPC_decoder::Lese_Bitstrom_SV7 ( void )
{
    // these arrays hold decoding results for bundled quantizers (3- and 5-step)
    /*static*/ int idx30[] = { -1, 0, 1,-1, 0, 1,-1, 0, 1,-1, 0, 1,-1, 0, 1,-1, 0, 1,-1, 0, 1,-1, 0, 1,-1, 0, 1};
    /*static*/ int idx31[] = { -1,-1,-1, 0, 0, 0, 1, 1, 1,-1,-1,-1, 0, 0, 0, 1, 1, 1,-1,-1,-1, 0, 0, 0, 1, 1, 1};
    /*static*/ int idx32[] = { -1,-1,-1,-1,-1,-1,-1,-1,-1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    /*static*/ int idx50[] = { -2,-1, 0, 1, 2,-2,-1, 0, 1, 2,-2,-1, 0, 1, 2,-2,-1, 0, 1, 2,-2,-1, 0, 1, 2};
    /*static*/ int idx51[] = { -2,-2,-2,-2,-2,-1,-1,-1,-1,-1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2};

    int n,k;
    int Max_used_Band=0;
    const HuffmanTyp *Table;
    int idx;
    int *L   ,*R;
    int *ResL,*ResR;
    unsigned int tmp;

    /***************************** Header *****************************/
    ResL  = Res_L;
    ResR  = Res_R;

    // first subband
    *ResL = Bitstream_read(4);
    *ResR = Bitstream_read(4);
    if (MS_used && !(*ResL==0 && *ResR==0)) MS_Flag[0] = Bitstream_read(1);

    // consecutive subbands
    ++ResL; ++ResR; // increase pointers
    for (n=1; n<=Max_Band; ++n, ++ResL, ++ResR)
    {
        idx   = Huffman_Decode_fast(HuffHdr);
        *ResL = (idx!=4) ? *(ResL-1) + idx : Bitstream_read(4);

        idx   = Huffman_Decode_fast(HuffHdr);
        *ResR = (idx!=4) ? *(ResR-1) + idx : Bitstream_read(4);

        if (MS_used && !(*ResL==0 && *ResR==0)) MS_Flag[n] = Bitstream_read(1);

        // only perform following procedures up to the maximum non-zero subband
        if (*ResL!=0 || *ResR!=0) Max_used_Band = n;
    }
    /****************************** SCFI ******************************/
    L     = SCFI_L;
    R     = SCFI_R;
    ResL  = Res_L;
    ResR  = Res_R;
    for (n=0; n<=Max_used_Band; ++n, ++L, ++R, ++ResL, ++ResR) {
        if (*ResL) *L = Huffman_Decode_faster(HuffSCFI);
        if (*ResR) *R = Huffman_Decode_faster(HuffSCFI);
    }

    /**************************** SCF/DSCF ****************************/
    ResL  = Res_L;
    ResR  = Res_R;
    L     = SCF_Index_L[0];
    R     = SCF_Index_R[0];
    for (n=0; n<=Max_used_Band; ++n, ++ResL, ++ResR, L+=3, R+=3) {
        if (*ResL)
        {
            L[2] = DSCF_Reference_L[n];
            switch (SCFI_L[n])
            {
                case 1:
                    idx  = Huffman_Decode_fast(HuffDSCF);
                    L[0] = (idx!=8) ? L[2] + idx : Bitstream_read(6);
                    idx  = Huffman_Decode_fast(HuffDSCF);
                    L[1] = (idx!=8) ? L[0] + idx : Bitstream_read(6);
                    L[2] = L[1];
                    break;
                case 3:
                    idx  = Huffman_Decode_fast(HuffDSCF);
                    L[0] = (idx!=8) ? L[2] + idx : Bitstream_read(6);
                    L[1] = L[0];
                    L[2] = L[1];
                    break;
                case 2:
                    idx  = Huffman_Decode_fast(HuffDSCF);
                    L[0] = (idx!=8) ? L[2] + idx : Bitstream_read(6);
                    L[1] = L[0];
                    idx  = Huffman_Decode_fast(HuffDSCF);
                    L[2] = (idx!=8) ? L[1] + idx : Bitstream_read(6);
                    break;
                case 0:
                    idx  = Huffman_Decode_fast(HuffDSCF);
                    L[0] = (idx!=8) ? L[2] + idx : Bitstream_read(6);
                    idx  = Huffman_Decode_fast(HuffDSCF);
                    L[1] = (idx!=8) ? L[0] + idx : Bitstream_read(6);
                    idx  = Huffman_Decode_fast(HuffDSCF);
                    L[2] = (idx!=8) ? L[1] + idx : Bitstream_read(6);
                    break;
                default:
                    return;
                    break;
            }
            // update Reference for DSCF
            DSCF_Reference_L[n] = L[2];
        }
        if (*ResR)
        {
            R[2] = DSCF_Reference_R[n];
            switch (SCFI_R[n])
            {
                case 1:
                    idx  = Huffman_Decode_fast(HuffDSCF);
                    R[0] = (idx!=8) ? R[2] + idx : Bitstream_read(6);
                    idx  = Huffman_Decode_fast(HuffDSCF);
                    R[1] = (idx!=8) ? R[0] + idx : Bitstream_read(6);
                    R[2] = R[1];
                    break;
                case 3:
                    idx  = Huffman_Decode_fast(HuffDSCF);
                    R[0] = (idx!=8) ? R[2] + idx : Bitstream_read(6);
                    R[1] = R[0];
                    R[2] = R[1];
                    break;
                case 2:
                    idx  = Huffman_Decode_fast(HuffDSCF);
                    R[0] = (idx!=8) ? R[2] + idx : Bitstream_read(6);
                    R[1] = R[0];
                    idx  = Huffman_Decode_fast(HuffDSCF);
                    R[2] = (idx!=8) ? R[1] + idx : Bitstream_read(6);
                    break;
                case 0:
                    idx  = Huffman_Decode_fast(HuffDSCF);
                    R[0] = (idx!=8) ? R[2] + idx : Bitstream_read(6);
                    idx  = Huffman_Decode_fast(HuffDSCF);
                    R[1] = (idx!=8) ? R[0] + idx : Bitstream_read(6);
                    idx  = Huffman_Decode_fast(HuffDSCF);
                    R[2] = (idx!=8) ? R[1] + idx : Bitstream_read(6);
                    break;
                default:
                    return;
                    break;
            }
            // update Reference for DSCF
            DSCF_Reference_R[n] = R[2];
        }
    }
    /***************************** Samples ****************************/
    ResL = Res_L;
    ResR = Res_R;
    L    = Q[0].L;
    R    = Q[0].R;
    for (n=0; n<=Max_used_Band; ++n, ++ResL, ++ResR, L+=36, R+=36)
    {
        /************** links **************/
        switch (*ResL)
        {
            case  -2: case  -3: case  -4: case  -5: case  -6: case  -7: case  -8: case  -9:
            case -10: case -11: case -12: case -13: case -14: case -15: case -16: case -17:
                L += 36;
                break;
            case -1:
                for (k=0; k<36; k++ ) {
                    tmp  = random_int ();
                    *L++ = ((tmp >> 24) & 0xFF) + ((tmp >> 16) & 0xFF) + ((tmp >>  8) & 0xFF) + ((tmp >>  0) & 0xFF) - 510;
                }
                break;
            case 0:
                L += 36;// increase pointer
                break;
            case 1:
                Table = HuffQ[Bitstream_read(1)][1];
                for (k=0; k<12; ++k)
                {
                    idx = Huffman_Decode_fast(Table);
                    *L++ = idx30[idx];
                    *L++ = idx31[idx];
                    *L++ = idx32[idx];
                }
                break;
            case 2:
                Table = HuffQ[Bitstream_read(1)][2];
                for (k=0; k<18; ++k)
                {
                    idx = Huffman_Decode_fast(Table);
                    *L++ = idx50[idx];
                    *L++ = idx51[idx];
                }
                break;
            case 3:
            case 4:
                Table = HuffQ[Bitstream_read(1)][*ResL];
                for (k=0; k<36; ++k)
                    *L++ = Huffman_Decode_faster(Table);
                break;
            case 5:
                Table = HuffQ[Bitstream_read(1)][*ResL];
                for (k=0; k<36; ++k)
                    *L++ = Huffman_Decode_fast(Table);
                break;
            case 6:
            case 7:
                Table = HuffQ[Bitstream_read(1)][*ResL];
                for (k=0; k<36; ++k)
                    *L++ = Huffman_Decode(Table);
                break;
            case 8: case 9: case 10: case 11: case 12: case 13: case 14: case 15: case 16: case 17:
                tmp = Dc[*ResL];
                for (k=0; k<36; ++k)
                    *L++ = (int)Bitstream_read(Res_bit[*ResL]) - tmp;
                break;
            default:
                return;
        }
        /************** rechts **************/
        switch (*ResR)
        {
            case  -2: case  -3: case  -4: case  -5: case  -6: case  -7: case  -8: case  -9:
            case -10: case -11: case -12: case -13: case -14: case -15: case -16: case -17:
                R += 36;
                break;
            case -1:
                for (k=0; k<36; k++ ) {
                    tmp  = random_int ();
                    *R++ = ((tmp >> 24) & 0xFF) + ((tmp >> 16) & 0xFF) + ((tmp >>  8) & 0xFF) + ((tmp >>  0) & 0xFF) - 510;
                }
                break;
            case 0:
                R += 36;// increase pointer
                break;
            case 1:
                Table = HuffQ[Bitstream_read(1)][1];
                for (k=0; k<12; ++k)
                {
                    idx = Huffman_Decode_fast(Table);
                    *R++ = idx30[idx];
                    *R++ = idx31[idx];
                    *R++ = idx32[idx];
                }
                break;
            case 2:
                Table = HuffQ[Bitstream_read(1)][2];
                for (k=0; k<18; ++k)
                {
                    idx = Huffman_Decode_fast(Table);
                    *R++ = idx50[idx];
                    *R++ = idx51[idx];
                }
                break;
            case 3:
            case 4:
                Table = HuffQ[Bitstream_read(1)][*ResR];
                for (k=0; k<36; ++k)
                    *R++ = Huffman_Decode_faster(Table);
                break;
            case 5:
                Table = HuffQ[Bitstream_read(1)][*ResR];
                for (k=0; k<36; ++k)
                    *R++ = Huffman_Decode_fast(Table);
                break;
            case 6:
            case 7:
                Table = HuffQ[Bitstream_read(1)][*ResR];
                for (k=0; k<36; ++k)
                    *R++ = Huffman_Decode(Table);
                break;
            case 8: case 9: case 10: case 11: case 12: case 13: case 14: case 15: case 16: case 17:
                tmp = Dc[*ResR];
                for (k=0; k<36; ++k)
                    *R++ = (int)Bitstream_read(Res_bit[*ResR]) - tmp;
                break;
            default:
                return;
        }
    }
}

MPC_decoder::~MPC_decoder ()
{
  if (SeekTable) free ( SeekTable );
}

MPC_decoder::MPC_decoder ( MPC_reader *r, double scale_factor )
{
  m_reader = r;

  HuffQ[0][0] = 0;
  HuffQ[1][0] = 0;
  HuffQ[0][1] = HuffQ1[0];
  HuffQ[1][1] = HuffQ1[1];
  HuffQ[0][2] = HuffQ2[0];
  HuffQ[1][2] = HuffQ2[1];
  HuffQ[0][3] = HuffQ3[0];
  HuffQ[1][3] = HuffQ3[1];
  HuffQ[0][4] = HuffQ4[0];
  HuffQ[1][4] = HuffQ4[1];
  HuffQ[0][5] = HuffQ5[0];
  HuffQ[1][5] = HuffQ5[1];
  HuffQ[0][6] = HuffQ6[0];
  HuffQ[1][6] = HuffQ6[1];
  HuffQ[0][7] = HuffQ7[0];
  HuffQ[1][7] = HuffQ7[1];

  SampleHuff[0] = NULL;
  SampleHuff[1] = Entropie_1;
  SampleHuff[2] = Entropie_2;
  SampleHuff[3] = Entropie_3;
  SampleHuff[4] = Entropie_4;
  SampleHuff[5] = Entropie_5;
  SampleHuff[6] = Entropie_6;
  SampleHuff[7] = Entropie_7;
  SampleHuff[8] = NULL;
  SampleHuff[9] = NULL;
  SampleHuff[10] = NULL;
  SampleHuff[11] = NULL;
  SampleHuff[12] = NULL;
  SampleHuff[13] = NULL;
  SampleHuff[14] = NULL;
  SampleHuff[15] = NULL;
  SampleHuff[16] = NULL;
  SampleHuff[17] = NULL;

  SeekTable = NULL;
  EQ_activated = 0;
  MPCHeaderPos = 0;
  StreamVersion = 0;
  MS_used = 0;
  FwdJumpInfo = 0;
  ActDecodePos = 0;
  FrameWasValid = 0;
  OverallFrames = 0;
  DecodedFrames = 0;
  LastValidSamples = 0;
  TrueGaplessPresent = 0;
  WordsRead = 0;
  Max_Band = 0;
  SampleRate = 0;
//  clips = 0;
  __r1 = 1;
  __r2 = 1;

  dword = 0;
  pos = 0;
  Zaehler = 0;
  WordsRead = 0;
  Max_Band = 0;

  initialisiere_Quantisierungstabellen (scale_factor);
  Huffman_SV6_Decoder ();
  Huffman_SV7_Decoder ();
}

void MPC_decoder::SetStreamInfo ( const StreamInfo *si )
{
	RESET_Synthesis ();
	RESET_Globals ();

  StreamVersion      = si->simple.StreamVersion;
  MS_used            = si->simple.MS;
  Max_Band           = si->simple.MaxBand;
  OverallFrames      = si->simple.Frames;
  MPCHeaderPos       = si->simple.HeaderPosition;
  LastValidSamples   = si->simple.LastFrameSamples;
  TrueGaplessPresent = si->simple.IsTrueGapless;
  SampleRate         = (int)si->simple.SampleFreq;

  if ( SeekTable != NULL ) free ( SeekTable );
  SeekTable = (unsigned short *)calloc ( sizeof(unsigned short), OverallFrames+64 );

  samples_to_skip = SynthDelay;
}

bool MPC_decoder::Initialize(const StreamInfo * info)
{
	SetStreamInfo(info);

  // AB: setting position to the beginning of the data-bitstream
  switch ( StreamVersion ) {
  case 0x04: f_seek ( 4 + MPCHeaderPos, SEEK_SET); pos = 16; break;  // Geht auch über eine der Helperfunktionen
  case 0x05:
  case 0x06: f_seek ( 8 + MPCHeaderPos, SEEK_SET); pos =  0; break;
  case 0x07:
  case 0x17: /*f_seek ( 24 + MPCHeaderPos, SEEK_SET);*/ pos =  8; break;
  default: return false;
  }

  // AB: fill buffer and initialize decoder
  f_read_dword ( Speicher, MEMSIZE );
  dword = Speicher [Zaehler = 0];

  return true;
}

//---------------------------------------------------------------
// will seek from the beginning of the file to the desired
// position in ms (given by seek_needed)
//---------------------------------------------------------------

void
MPC_decoder::Helper1 ( unsigned long bitpos )
{
    f_seek ( (bitpos>>5) * 4 + MPCHeaderPos, SEEK_SET );
    f_read_dword ( Speicher, 2 );
    dword = Speicher [ Zaehler = 0];
    pos   = bitpos & 31;
}

void
MPC_decoder::Helper2 ( unsigned long bitpos )
{
    f_seek ( (bitpos>>5) * 4 + MPCHeaderPos, SEEK_SET );
    f_read_dword ( Speicher, MEMSIZE );
    dword = Speicher [ Zaehler = 0];
    pos   = bitpos & 31;
}

void
MPC_decoder::Helper3 ( unsigned long bitpos, unsigned long* buffoffs )
{
    pos      = bitpos & 31;
    bitpos >>= 5;
    if ( (unsigned long)(bitpos - *buffoffs) >= MEMSIZE-2 ) {
        *buffoffs = bitpos;
        f_seek ( bitpos * 4L + MPCHeaderPos, SEEK_SET );
        f_read_dword ( Speicher, MEMSIZE );
    }
    dword = Speicher [ Zaehler = bitpos - *buffoffs ];
}

static unsigned int get_initial_fpos(unsigned int StreamVersion)
{
    unsigned int fpos = 0;
    switch ( StreamVersion ) {                                                  // setting position to the beginning of the data-bitstream
    case  0x04: fpos =  48; break;
    case  0x05:
    case  0x06: fpos =  64; break;
    case  0x07:
    case  0x17: fpos = 200; break;
    }
	return fpos;
}

bool MPC_decoder::SeekSeconds ( double seconds )
{
    return SeekSample((MPC_INT64)(seconds * (double)SampleRate + 0.5));
}


bool MPC_decoder::SeekSample(MPC_INT64 destsample)
{
    unsigned long  fpos;
    unsigned int   fwd;

	fwd = (unsigned) (destsample / FrameLength);
	samples_to_skip = SynthDelay + (unsigned)(destsample % FrameLength);

    memset ( Y_L             , 0, sizeof Y_L              );
    memset ( Y_R             , 0, sizeof Y_R              );
    memset ( SCF_Index_L     , 0, sizeof SCF_Index_L      );
    memset ( SCF_Index_R     , 0, sizeof SCF_Index_R      );
    memset ( Res_L           , 0, sizeof Res_L            );
    memset ( Res_R           , 0, sizeof Res_R            );
    memset ( SCFI_L          , 0, sizeof SCFI_L           );
    memset ( SCFI_R          , 0, sizeof SCFI_R           );
    memset ( DSCF_Flag_L     , 0, sizeof DSCF_Flag_L      );
    memset ( DSCF_Flag_R     , 0, sizeof DSCF_Flag_R      );
    memset ( DSCF_Reference_L, 0, sizeof DSCF_Reference_L );
    memset ( DSCF_Reference_R, 0, sizeof DSCF_Reference_R );
    memset ( Q               , 0, sizeof Q                );
    memset ( MS_Flag         , 0, sizeof MS_Flag          );

    RESET_Synthesis ();                                                         // resetting synthesis filter to avoid "clicks"

    fwd           = fwd < OverallFrames  ?  fwd  :  OverallFrames;              // prevent from desired position out of allowed range
    DecodedFrames = 0;                                                          // reset number of decoded frames

    fpos = get_initial_fpos(StreamVersion);
    if (fpos == 0) return false;

#if 1
    {
        unsigned long buffoffs = 0x80000000;
        for ( ; DecodedFrames + 1024 < fwd; DecodedFrames++ ) {                   // proceed until 32 frames before (!!) desired position (allows to scan the scalefactors)
            if ( SeekTable [DecodedFrames] == 0 ) {
                Helper3 ( fpos, &buffoffs );
                fpos += SeekTable [DecodedFrames] = 20 + Bitstream_read (20);   // calculate desired pos (in Bits)
            } else {
                fpos += SeekTable [DecodedFrames];
            }
        }
    }
#endif
    Helper2 ( fpos );

    for ( ; DecodedFrames < fwd; DecodedFrames++ ) {                            // read the last 32 frames before the desired position to scan the scalefactors (artifactless jumping)
        unsigned int   FrameBitCnt;
        unsigned int   RING;
        RING         = Zaehler;
        FwdJumpInfo  = Bitstream_read (20);                                     // read jump-info
        SeekTable [DecodedFrames] = 20 + FwdJumpInfo;
        ActDecodePos = (Zaehler << 5) + pos;
        FrameBitCnt  = BitsRead ();                                             // scanning the scalefactors and check for validity of frame
        if (StreamVersion >= 7)  Lese_Bitstrom_SV7 ();
        else Lese_Bitstrom_SV6 ();
        if ( BitsRead() - FrameBitCnt != FwdJumpInfo ) {
//            Box ("Bug in perform_jump");
            return false;
        }
        if ( (RING ^ Zaehler) & MEMSIZE2 )                                      // update buffer
            f_read_dword ( Speicher + (RING & MEMSIZE2),  MEMSIZE2 );
    }

//    LastBitsRead = BitsRead ();
  //  LastFrame = DecodedFrames;

    return true;
}

void MPC_decoder::UpdateBuffer ( unsigned int RING )
{
    if ( (RING ^ Zaehler) & MEMSIZE2 )
        f_read_dword ( Speicher + (RING & MEMSIZE2), MEMSIZE2 );      // update buffer
}

static unsigned long swap32(unsigned long val)
{
	const unsigned char * src = (const unsigned char*)&val;
	return (unsigned long)src[0] | ((unsigned long)src[1] << 8) | ((unsigned long)src[2] << 16) | ((unsigned long)src[3] << 24);
}

int MPC_decoder::f_read_dword( unsigned int * ptr, unsigned int count)
{
	count = f_read(ptr,count << 2) >> 2;
#ifndef MPC_LITTLE_ENDIAN
	unsigned int n;
	for(n=0;n<count;n++)
		ptr[n] = swap32(ptr[n]);
#endif
	return count;
}
