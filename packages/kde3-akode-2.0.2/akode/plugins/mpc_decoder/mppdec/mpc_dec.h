#ifndef _mpp_dec_h_
#define _mpp_dec_h_


#ifdef WIN32
#define MPC_LITTLE_ENDIAN
#endif


#include <assert.h>

#include "mpc_math.h"


class MPC_reader {
public:
    virtual size_t read ( void *ptr, size_t size ) = 0;
    virtual bool seek ( int offset, int origin ) = 0;
    virtual long tell () = 0;
    virtual long get_size () = 0;
    virtual bool canseek() = 0;
};

class MPC_decoder {
public:
    MPC_decoder(MPC_reader *r, double scale_factor = 1.0);
    ~MPC_decoder();

    bool Initialize(const class StreamInfo *si);
    void ScaleOutput(double factor);
    unsigned int Decode(MPC_SAMPLE_FORMAT *buffer, unsigned int * vbr_update_acc = 0, unsigned int * vbr_update_bits = 0);
    bool SeekSeconds(double seconds);
    bool SeekSample(MPC_INT64 sample);

public:
    enum
    {
            FrameLength = (36 * 32),               // samples per frame
            SynthDelay = 481,
            DecodeBufferLength = 4 * FrameLength
    };
private:
    enum
    {
        EQ_TAP = 13,                        // length of FIR filter for EQ
        DELAY = ((EQ_TAP + 1) / 2),         // delay of FIR
        FIR_BANDS = 4,                      // number of subbands to be FIR filtered
        MEMSIZE = 16384,                    // overall buffer size
        MEMSIZE2 = (MEMSIZE/2),             // size of one buffer
        MEMMASK = (MEMSIZE-1),
        V_MEM = 2304
    };
    void SetStreamInfo ( const class StreamInfo *si );
    void UpdateBuffer ( unsigned int RING );
    void RESET_Synthesis ( void );
    void RESET_Globals ( void );

    unsigned int BitsRead ( void );
    unsigned int decode_internal ( MPC_SAMPLE_FORMAT *buffer );
    void RESET_Y ( void );
    void Lese_Bitstrom_SV6 ( void );
    void Lese_Bitstrom_SV7 ( void );

    void Requantisierung ( const int Last_Band );

    unsigned int samples_to_skip;

    typedef struct {
        int  L [36];
        int  R [36];
    } QuantTyp;

    typedef struct {
        unsigned int  Code;
        unsigned int  Length;
        int           Value;
    } HuffmanTyp;

    static int HuffmanTyp_cmpfn ( const void* p1, const void* p2 );

    MPC_reader *m_reader;

    unsigned int  dword;                      // actually decoded 32bit-word
    unsigned int  pos;                        // bit-position within dword
    unsigned int  Speicher [MEMSIZE];         // read-buffer
    unsigned int  Zaehler;                    // actual index within read-buffer

    unsigned int  FwdJumpInfo;
    unsigned int  ActDecodePos;
    unsigned int  FrameWasValid;

    unsigned int  DecodedFrames;
    unsigned int  OverallFrames;
    int           SampleRate;                 // Sample frequency

    unsigned int  StreamVersion;              // version of bitstream
    unsigned int  MS_used;                    // MS-coding used ?
    int           Max_Band;
    unsigned int  MPCHeaderPos;               // AB: needed to support ID3v2
    //unsigned int  OverallFrames;
    //unsigned int  DecodedFrames;
    unsigned int  LastValidSamples;
    unsigned int  TrueGaplessPresent;

    unsigned int  EQ_activated;

    unsigned int  WordsRead;                  // counts amount of decoded dwords
    unsigned short* SeekTable;

  /*static*/ unsigned int  __r1;
  /*static*/ unsigned int  __r2;


    int           SCF_Index_L [32] [3];
    int           SCF_Index_R [32] [3];       // holds scalefactor-indices
    QuantTyp      Q [32];                     // holds quantized samples
    int           Res_L [32];
    int           Res_R [32];                 // holds the chosen quantizer for each subband
    int           DSCF_Flag_L [32];
    int           DSCF_Flag_R [32];           // differential SCF used?
    int           SCFI_L [32];
    int           SCFI_R [32];                // describes order of transmitted SCF
    int           DSCF_Reference_L [32];
    int           DSCF_Reference_R [32];      // holds last frames SCF
    int           MS_Flag[32];                // MS used?


    HuffmanTyp    HuffHdr  [10];
    HuffmanTyp    HuffSCFI [ 4];
    HuffmanTyp    HuffDSCF [16];
    HuffmanTyp*   HuffQ [2] [8];

    HuffmanTyp    HuffQ1 [2] [3*3*3];
    HuffmanTyp    HuffQ2 [2] [5*5];
    HuffmanTyp    HuffQ3 [2] [ 7];
    HuffmanTyp    HuffQ4 [2] [ 9];
    HuffmanTyp    HuffQ5 [2] [15];
    HuffmanTyp    HuffQ6 [2] [31];
    HuffmanTyp    HuffQ7 [2] [63];

    const HuffmanTyp* SampleHuff [18];
    HuffmanTyp    SCFI_Bundle   [ 8];
    HuffmanTyp    DSCF_Entropie [13];
    HuffmanTyp    Region_A [16];
    HuffmanTyp    Region_B [ 8];
    HuffmanTyp    Region_C [ 4];

    HuffmanTyp    Entropie_1 [ 3];
    HuffmanTyp    Entropie_2 [ 5];
    HuffmanTyp    Entropie_3 [ 7];
    HuffmanTyp    Entropie_4 [ 9];
    HuffmanTyp    Entropie_5 [15];
    HuffmanTyp    Entropie_6 [31];
    HuffmanTyp    Entropie_7 [63];

    MPC_SAMPLE_FORMAT V_L [V_MEM + 960];
    MPC_SAMPLE_FORMAT V_R [V_MEM + 960];
    MPC_SAMPLE_FORMAT Y_L [36] [32];
    MPC_SAMPLE_FORMAT Y_R [36] [32];

    MPC_SAMPLE_FORMAT SCF [256];                 // holds adapted scalefactors (for clipping prevention)
#ifdef MPC_FIXED_POINT
    unsigned char SCF_shift[256];
#endif
    unsigned int  Q_bit [32];                 // holds amount of bits to save chosen quantizer (SV6)
    unsigned int  Q_res [32] [16];            // holds conversion: index -> quantizer (SV6)

private: // functions
    void Reset_BitstreamDecode ( void );
    unsigned int Bitstream_read ( const unsigned int );
    int Huffman_Decode ( const HuffmanTyp* );         // works with maximum lengths up to 14
    int Huffman_Decode_fast ( const HuffmanTyp* );    // works with maximum lengths up to 10
    int Huffman_Decode_faster ( const HuffmanTyp* );  // works with maximum lengths up to  5
    void SCFI_Bundle_read ( const HuffmanTyp*, int*, int* );

    void Reset_V ( void );
    void Synthese_Filter_float ( MPC_SAMPLE_FORMAT * dst );
    unsigned int random_int ( void );

    void EQSet ( int on, char data[10], int preamp );
    void Helper1 ( unsigned long bitpos );
    void Helper2 ( unsigned long bitpos );
    void Helper3 ( unsigned long bitpos, unsigned long* buffoffs );
    void Huffman_SV6_Encoder ( void );
    void Huffman_SV6_Decoder ( void );
    void Huffman_SV7_Encoder ( void );
    void Huffman_SV7_Decoder ( void );

    void initialisiere_Quantisierungstabellen ( double );
    void Quantisierungsmodes ( void );        // conversion: index -> quantizer (bitstream reading)
    void Resort_HuffTables ( const unsigned int elements, HuffmanTyp* Table, const int offset );
private:
    inline int f_read ( void *ptr, size_t size) { return m_reader->read (ptr, size); };
    int f_read_dword( unsigned int * ptr, unsigned int count);
    inline bool f_seek ( int offset, int origin) { return m_reader->seek (offset, origin); };
};

#include "streaminfo.h"

#endif
