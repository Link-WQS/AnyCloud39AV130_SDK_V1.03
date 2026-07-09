#define LEN_HINT         512
#define LEN_OPTION_SHORT 512
#define LEN_SERIAL       8
#define LEN_EXT          32
#define LEN_FILENAME     128
#define AMR_HEADER       "#!AMR\n"

#define MAXWAVESIZE 4294967040LU
#define ASSIC_0 '0'

#ifdef WIN32
#pragma pack(push, 1)
#endif

typedef struct
#ifdef __GNUC__
__attribute__((packed))
#endif
{
    char     szRiffID[4];  // 'R','I','F','F'
    uint32_t dwRiffSize;
    char     szRiffFormat[4]; // 'W','A','V','E'
}RIFF_HEADER;
static RIFF_HEADER riff_hdr = {{'R','I','F','F'}, 0, {'W','A','V','E'}};

typedef struct
#ifdef __GNUC__
__attribute__((packed))
#endif
{
   char  id[4]; // identifier, e.g. "fmt " or "data"
   uint32_t len;  // remaining chunk length after header
} CHUNK_HEADER;// CHUNK 8-byte header

#define CHUNK_SIZE(ck) (sizeof(CHUNK_HEADER) + (ck)->hdr.len)

typedef struct
#ifdef __GNUC__
__attribute__((packed))
#endif
{
    uint16_t  wFormatTag; //编码方式，一般为 0x0001
    uint16_t  wChannels;
    uint32_t  dwSamplesPerSec;
    uint32_t  dwAvgBytesPerSec;
    uint16_t  wBlockAlign;
    uint16_t  wBitsPerSample;
    uint16_t  wExtraSize; // The count in bytes of the extra size
}WAVE_FORMAT_EX;

typedef struct
#ifdef __GNUC__
__attribute__((packed))
#endif
{
    CHUNK_HEADER   hdr; // id = "fmt "
    WAVE_FORMAT_EX wavFormat;
}FMT_CK;
typedef struct
#ifdef __GNUC__
__attribute__((packed))
#endif
{
    CHUNK_HEADER   hdr; // id = "fact"
    uint32_t       dwFileSize; // file time in number of samples
}FACT_CK;
typedef struct
#ifdef __GNUC__
__attribute__((packed))
#endif
{
    CHUNK_HEADER   hdr; // id = "data"
}DATA_CK;

typedef struct
#ifdef __GNUC__
__attribute__((packed))
#endif
{
    FMT_CK fmt_ck;
    uint16_t wSamplesPerBlock;
}ADPCM_FMT_CK;

#ifdef WIN32
#pragma pack(pop)
#endif


static FMT_CK  fmt_ck  = {{{'f', 'm', 't', ' '}, 0}};
static FACT_CK fact_ck = {{{'f', 'a', 'c', 't'}, 0}};
static DATA_CK data_ck = {{{'d', 'a', 't', 'a'}, 0}};


typedef enum
{
    _WAVE_TYPE_UNKNOWN = 0000 ,
    _WAVE_LPCM ,
    _WAVE_ALAW ,
    _WAVE_ULAW ,
    _WAVE_DVI_ADPCM
}T_WAVE_TYPE;

typedef struct audio_files
{
    T_WAVE_TYPE fileType;
    uint32_t samplerate;
    uint16_t bits_per_sample;
    uint16_t channels;
    uint16_t block_align; // for dvi adpcm
    uint16_t samples_per_block; // for dvi adpcm
    uint32_t data_len;
} audio_files;

int32_t write_wav_header(audio_files *aufile, FILE *outFile);
uint32_t min( uint32_t u1 , uint32_t u2 );