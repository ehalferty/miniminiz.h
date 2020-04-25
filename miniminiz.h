#ifndef MINIMINIZ_HEADER_INCLUDED
#define MINIMINIZ_HEADER_INCLUDED
#include <stdlib.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef unsigned long mmz_ulong;

// mmz_free() internally uses the MMZ_FREE() macro (which by default calls free() unless you've modified the MMZ_MALLOC macro) to release a block allocated from the heap.
void mmz_free(void *p);

#define MMZ_ADLER32_INIT (1)
// mmz_adler32() returns the initial adler-32 value to use when called with ptr==NULL.
mmz_ulong mmz_adler32(mmz_ulong adler, const unsigned char *ptr, size_t buf_len);

#define MMZ_CRC32_INIT (0)
// mmz_crc32() returns the initial CRC-32 value to use when called with ptr==NULL.
mmz_ulong mmz_crc32(mmz_ulong crc, const unsigned char *ptr, size_t buf_len);

// Compression strategies.
enum { MMZ_DEFAULT_STRATEGY = 0, MMZ_FILTERED = 1, MMZ_HUFFMAN_ONLY = 2, MMZ_RLE = 3, MMZ_FIXED = 4 };

// Method
#define MMZ_DEFLATED 8

// Heap allocation callbacks.
// Note that mmz_alloc_func parameter types purpsosely differ from zlib's: items/size is size_t, not unsigned long.
typedef void *(*mmz_alloc_func)(void *opaque, size_t items, size_t size);
typedef void (*mmz_free_func)(void *opaque, void *address);
typedef void *(*mmz_realloc_func)(void *opaque, void *address, size_t items, size_t size);

// Flush values. For typical usage you only need MMZ_NO_FLUSH and MMZ_FINISH. The other values are for advanced use (refer to the zlib docs).
enum { MMZ_NO_FLUSH = 0, MMZ_PARTIAL_FLUSH = 1, MMZ_SYNC_FLUSH = 2, MMZ_FULL_FLUSH = 3, MMZ_FINISH = 4, MMZ_BLOCK = 5 };

// Return status codes. MMZ_PARAM_ERROR is non-standard.
enum { MMZ_OK = 0, MMZ_STREAM_END = 1, MMZ_NEED_DICT = 2, MMZ_ERRNO = -1, MMZ_STREAM_ERROR = -2, MMZ_DATA_ERROR = -3, MMZ_MEM_ERROR = -4, MMZ_BUF_ERROR = -5, MMZ_VERSION_ERROR = -6, MMZ_PARAM_ERROR = -10000 };

// Window bits
#define MMZ_DEFAULT_WINDOW_BITS 15

struct mmz_internal_state;

// Compression/decompression stream struct.
typedef struct mmz_stream_s
{
  const unsigned char *next_in;     // pointer to next byte to read
  unsigned int avail_in;            // number of bytes available at next_in
  mmz_ulong total_in;                // total number of bytes consumed so far

  unsigned char *next_out;          // pointer to next byte to write
  unsigned int avail_out;           // number of bytes that can be written to next_out
  mmz_ulong total_out;               // total number of bytes produced so far

  char *msg;                        // error msg (unused)
  struct mmz_internal_state *state;  // internal state, allocated by zalloc/zfree

  mmz_alloc_func zalloc;             // optional heap allocation function (defaults to malloc)
  mmz_free_func zfree;               // optional heap free function (defaults to free)
  void *opaque;                     // heap alloc function user pointer

  int data_type;                    // data_type (unused)
  mmz_ulong adler;                   // adler32 of the source or uncompressed data
  mmz_ulong reserved;                // not used
} mmz_stream;

typedef mmz_stream *mmz_streamp;

// Returns the version string of miniz.c.
const char *mmz_version(void);

// mmz_deflateInit() initializes a compressor with default options:
// Parameters:
//  pStream must point to an initialized mmz_stream struct.
//  level must be between [MMZ_NO_COMPRESSION, MMZ_BEST_COMPRESSION].
//  level 1 enables a specially optimized compression function that's been optimized purely for performance, not ratio.
//  (This special func. is currently only enabled when MINIZ_USE_UNALIGNED_LOADS_AND_STORES and MINIZ_LITTLE_ENDIAN are defined.)
// Return values:
//  MMZ_OK on success.
//  MMZ_STREAM_ERROR if the stream is bogus.
//  MMZ_PARAM_ERROR if the input parameters are bogus.
//  MMZ_MEM_ERROR on out of memory.
int mmz_deflateInit(mmz_streamp pStream, int level);

// mmz_deflateInit2() is like mmz_deflate(), except with more control:
// Additional parameters:
//   method must be MMZ_DEFLATED
//   window_bits must be MMZ_DEFAULT_WINDOW_BITS (to wrap the deflate stream with zlib header/adler-32 footer) or -MMZ_DEFAULT_WINDOW_BITS (raw deflate/no header or footer)
//   mem_level must be between [1, 9] (it's checked but ignored by miniz.c)
int mmz_deflateInit2(mmz_streamp pStream, int level, int method, int window_bits, int mem_level, int strategy);

// Quickly resets a compressor without having to reallocate anything. Same as calling mmz_deflateEnd() followed by mmz_deflateInit()/mmz_deflateInit2().
int mmz_deflateReset(mmz_streamp pStream);

// mmz_deflate() compresses the input to output, consuming as much of the input and producing as much output as possible.
// Parameters:
//   pStream is the stream to read from and write to. You must initialize/update the next_in, avail_in, next_out, and avail_out members.
//   flush may be MMZ_NO_FLUSH, MMZ_PARTIAL_FLUSH/MMZ_SYNC_FLUSH, MMZ_FULL_FLUSH, or MMZ_FINISH.
// Return values:
//   MMZ_OK on success (when flushing, or if more input is needed but not available, and/or there's more output to be written but the output buffer is full).
//   MMZ_STREAM_END if all input has been consumed and all output bytes have been written. Don't call mmz_deflate() on the stream anymore.
//   MMZ_STREAM_ERROR if the stream is bogus.
//   MMZ_PARAM_ERROR if one of the parameters is invalid.
//   MMZ_BUF_ERROR if no forward progress is possible because the input and/or output buffers are empty. (Fill up the input buffer or free up some output space and try again.)
int mmz_deflate(mmz_streamp pStream, int flush);

// mmz_deflateEnd() deinitializes a compressor:
// Return values:
//  MMZ_OK on success.
//  MMZ_STREAM_ERROR if the stream is bogus.
int mmz_deflateEnd(mmz_streamp pStream);

// mmz_deflateBound() returns a (very) conservative upper bound on the amount of data that could be generated by deflate(), assuming flush is set to only MMZ_NO_FLUSH or MMZ_FINISH.
mmz_ulong mmz_deflateBound(mmz_streamp pStream, mmz_ulong source_len);


// Initializes a decompressor.
int mmz_inflateInit(mmz_streamp pStream);

// mmz_inflateInit2() is like mmz_inflateInit() with an additional option that controls the window size and whether or not the stream has been wrapped with a zlib header/footer:
// window_bits must be MMZ_DEFAULT_WINDOW_BITS (to parse zlib header/footer) or -MMZ_DEFAULT_WINDOW_BITS (raw deflate).
int mmz_inflateInit2(mmz_streamp pStream, int window_bits);

// Decompresses the input stream to the output, consuming only as much of the input as needed, and writing as much to the output as possible.
// Parameters:
//   pStream is the stream to read from and write to. You must initialize/update the next_in, avail_in, next_out, and avail_out members.
//   flush may be MMZ_NO_FLUSH, MMZ_SYNC_FLUSH, or MMZ_FINISH.
//   On the first call, if flush is MMZ_FINISH it's assumed the input and output buffers are both sized large enough to decompress the entire stream in a single call (this is slightly faster).
//   MMZ_FINISH implies that there are no more source bytes available beside what's already in the input buffer, and that the output buffer is large enough to hold the rest of the decompressed data.
// Return values:
//   MMZ_OK on success. Either more input is needed but not available, and/or there's more output to be written but the output buffer is full.
//   MMZ_STREAM_END if all needed input has been consumed and all output bytes have been written. For zlib streams, the adler-32 of the decompressed data has also been verified.
//   MMZ_STREAM_ERROR if the stream is bogus.
//   MMZ_DATA_ERROR if the deflate stream is invalid.
//   MMZ_PARAM_ERROR if one of the parameters is invalid.
//   MMZ_BUF_ERROR if no forward progress is possible because the input buffer is empty but the inflater needs more input to continue, or if the output buffer is not large enough. Call mmz_inflate() again
//   with more input data, or with more room in the output buffer (except when using single call decompression, described above).
int mmz_inflate(mmz_streamp pStream, int flush);

// Deinitializes a decompressor.
int mmz_inflateEnd(mmz_streamp pStream);

// Single-call decompression.
// Returns MMZ_OK on success, or one of the error codes from mmz_inflate() on failure.
int mmz_uncompress(unsigned char *pDest, mmz_ulong *pDest_len, const unsigned char *pSource, mmz_ulong source_len);

// Returns a string description of the specified error code, or NULL if the error code is invalid.
const char *mmz_error(int err);

  typedef unsigned char Byte;
  typedef unsigned int uInt;
  typedef mmz_ulong uLong;
  typedef Byte Bytef;
  typedef uInt uIntf;
  typedef char charf;
  typedef int intf;
  typedef void *voidpf;
  typedef uLong uLongf;
  typedef void *voidp;
  typedef void *const voidpc;

typedef unsigned char mmz_uint8;
typedef signed short mmz_int16;
typedef unsigned short mmz_uint16;
typedef unsigned int mmz_uint32;
typedef unsigned int mmz_uint;
typedef long long mmz_int64;
typedef unsigned long long mmz_uint64;
typedef int mmz_bool;

#define MMZ_FALSE (0)
#define MMZ_TRUE (1)

// An attempt to work around MSVC's spammy "warning C4127: conditional expression is constant" message.
#ifdef _MSC_VER
   #define MMZ_MACRO_END while (0, 0)
#else
   #define MMZ_MACRO_END while (0)
#endif

// ------------------- Low-level Decompression API Definitions

// Decompression flags used by mmz_tinfl_decompress().
// MMZ_TINFL_FLAG_PARSE_ZLIB_HEADER: If set, the input has a valid zlib header and ends with an adler32 checksum (it's a valid zlib stream). Otherwise, the input is a raw deflate stream.
// MMZ_TINFL_FLAG_HAS_MORE_INPUT: If set, there are more input bytes available beyond the end of the supplied input buffer. If clear, the input buffer contains all remaining input.
// MMZ_TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF: If set, the output buffer is large enough to hold the entire decompressed stream. If clear, the output buffer is at least the size of the dictionary (typically 32KB).
// MMZ_TINFL_FLAG_COMPUTE_ADLER32: Force adler-32 checksum computation of the decompressed bytes.
enum
{
  MMZ_TINFL_FLAG_PARSE_ZLIB_HEADER = 1,
  MMZ_TINFL_FLAG_HAS_MORE_INPUT = 2,
  MMZ_TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF = 4,
  MMZ_TINFL_FLAG_COMPUTE_ADLER32 = 8
};

// High level decompression functions:
// mmz_tinfl_decompress_mem_to_heap() decompresses a block in memory to a heap block allocated via malloc().
// On entry:
//  pSrc_buf, src_buf_len: Pointer and size of the Deflate or zlib source data to decompress.
// On return:
//  Function returns a pointer to the decompressed data, or NULL on failure.
//  *pOut_len will be set to the decompressed data's size, which could be larger than src_buf_len on uncompressible data.
//  The caller must call mmz_free() on the returned block when it's no longer needed.
void *mmz_tinfl_decompress_mem_to_heap(const void *pSrc_buf, size_t src_buf_len, size_t *pOut_len, int flags);

// mmz_tinfl_decompress_mem_to_mem() decompresses a block in memory to another block in memory.
// Returns MMZ_TINFL_DECOMPRESS_MEM_TO_MEM_FAILED on failure, or the number of bytes written on success.
#define MMZ_TINFL_DECOMPRESS_MEM_TO_MEM_FAILED ((size_t)(-1))
size_t mmz_tinfl_decompress_mem_to_mem(void *pOut_buf, size_t out_buf_len, const void *pSrc_buf, size_t src_buf_len, int flags);

// mmz_tinfl_decompress_mem_to_callback() decompresses a block in memory to an internal 32KB buffer, and a user provided callback function will be called to flush the buffer.
// Returns 1 on success or 0 on failure.
typedef int (*mmz_tinfl_put_buf_func_ptr)(const void* pBuf, int len, void *pUser);
int mmz_tinfl_decompress_mem_to_callback(const void *pIn_buf, size_t *pIn_buf_size, mmz_tinfl_put_buf_func_ptr pPut_buf_func, void *pPut_buf_user, int flags);

struct mmz_tinfl_decompressor_tag; typedef struct mmz_tinfl_decompressor_tag mmz_tinfl_decompressor;

// Max size of LZ dictionary.
#define MMZ_TINFL_LZ_DICT_SIZE 32768

// Return status.
typedef enum
{
  MMZ_TINFL_STATUS_BAD_PARAM = -3,
  MMZ_TINFL_STATUS_ADLER32_MISMATCH = -2,
  MMZ_TINFL_STATUS_FAILED = -1,
  MMZ_TINFL_STATUS_DONE = 0,
  MMZ_TINFL_STATUS_NEEDS_MORE_INPUT = 1,
  MMZ_TINFL_STATUS_HAS_MORE_OUTPUT = 2
} mmz_tinfl_status;

// Initializes the decompressor to its initial state.
#define mmz_tinfl_init(r) do { (r)->m_state = 0; } MMZ_MACRO_END
#define mmz_tinfl_get_adler32(r) (r)->m_check_adler32

// Main low-level decompressor coroutine function. This is the only function actually needed for decompression. All the other functions are just high-level helpers for improved usability.
// This is a universal API, i.e. it can be used as a building block to build any desired higher level decompression API. In the limit case, it can be called once per every byte input or output.
mmz_tinfl_status mmz_tinfl_decompress(mmz_tinfl_decompressor *r, const mmz_uint8 *pIn_buf_next, size_t *pIn_buf_size, mmz_uint8 *pOut_buf_start, mmz_uint8 *pOut_buf_next, size_t *pOut_buf_size, const mmz_uint32 decomp_flags);

// Internal/private bits follow.
enum
{
  MMZ_TINFL_MAX_HUFF_TABLES = 3, MMZ_TINFL_MAX_HUFF_SYMBOLS_0 = 288, MMZ_TINFL_MAX_HUFF_SYMBOLS_1 = 32, MMZ_TINFL_MAX_HUFF_SYMBOLS_2 = 19,
  MMZ_TINFL_FAST_LOOKUP_BITS = 10, MMZ_TINFL_FAST_LOOKUP_SIZE = 1 << MMZ_TINFL_FAST_LOOKUP_BITS
};

typedef struct
{
  mmz_uint8 m_code_size[TINFL_MAX_HUFF_SYMBOLS_0];
  mmz_int16 m_look_up[TINFL_FAST_LOOKUP_SIZE], m_tree[TINFL_MAX_HUFF_SYMBOLS_0 * 2];
} mmz_tinfl_huff_table;


  #define MMZ_TINFL_USE_64BIT_BITBUF 1


#if MMZ_TINFL_USE_64BIT_BITBUF
  typedef mmz_uint64 mmz_tinfl_bit_buf_t;
  #define MMZ_TINFL_BITBUF_SIZE (64)
#else
  typedef mmz_uint32 mmz_tinfl_bit_buf_t;
  #define MMZ_TINFL_BITBUF_SIZE (32)
#endif

struct mmz_tinfl_decompressor_tag
{
  mmz_uint32 m_state, m_num_bits, m_zhdr0, m_zhdr1, m_z_adler32, m_final, m_type, m_check_adler32, m_dist, m_counter, m_num_extra, m_table_sizes[TINFL_MAX_HUFF_TABLES];
  mmz_tinfl_bit_buf_t m_bit_buf;
  size_t m_dist_from_out_buf_start;
  mmz_tinfl_huff_table m_tables[TINFL_MAX_HUFF_TABLES];
  mmz_uint8 m_raw_header[4], m_len_codes[TINFL_MAX_HUFF_SYMBOLS_0 + MMZ_TINFL_MAX_HUFF_SYMBOLS_1 + 137];
};

// ------------------- Low-level Compression API Definitions

// Set MMZ_TDEFL_LESS_MEMORY to 1 to use less memory (compression will be slightly slower, and raw/dynamic blocks will be output more frequently).
#define MMZ_TDEFL_LESS_MEMORY 0

// mmz_tdefl_init() compression flags logically OR'd together (low 12 bits contain the max. number of probes per dictionary search):
// MMZ_TDEFL_DEFAULT_MAX_PROBES: The compressor defaults to 128 dictionary probes per dictionary search. 0=Huffman only, 1=Huffman+LZ (fastest/crap compression), 4095=Huffman+LZ (slowest/best compression).
enum
{
  MMZ_TDEFL_HUFFMAN_ONLY = 0, MMZ_TDEFL_DEFAULT_MAX_PROBES = 128, MMZ_TDEFL_MAX_PROBES_MASK = 0xFFF
};

// MMZ_TDEFL_WRITE_ZLIB_HEADER: If set, the compressor outputs a zlib header before the deflate data, and the Adler-32 of the source data at the end. Otherwise, you'll get raw deflate data.
// MMZ_TDEFL_COMPUTE_ADLER32: Always compute the adler-32 of the input data (even when not writing zlib headers).
// MMZ_TDEFL_GREEDY_PARSING_FLAG: Set to use faster greedy parsing, instead of more efficient lazy parsing.
// MMZ_TDEFL_NONDETERMINISTIC_PARSING_FLAG: Enable to decrease the compressor's initialization time to the minimum, but the output may vary from run to run given the same input (depending on the contents of memory).
// MMZ_TDEFL_RLE_MATCHES: Only look for RLE matches (matches with a distance of 1)
// MMZ_TDEFL_FILTER_MATCHES: Discards matches <= 5 chars if enabled.
// MMZ_TDEFL_FORCE_ALL_STATIC_BLOCKS: Disable usage of optimized Huffman tables.
// MMZ_TDEFL_FORCE_ALL_RAW_BLOCKS: Only use raw (uncompressed) deflate blocks.
// The low 12 bits are reserved to control the max # of hash probes per dictionary lookup (see MMZ_TDEFL_MAX_PROBES_MASK).
enum
{
  MMZ_TDEFL_WRITE_ZLIB_HEADER             = 0x01000,
  MMZ_TDEFL_COMPUTE_ADLER32               = 0x02000,
  MMZ_TDEFL_GREEDY_PARSING_FLAG           = 0x04000,
  MMZ_TDEFL_NONDETERMINISTIC_PARSING_FLAG = 0x08000,
  MMZ_TDEFL_RLE_MATCHES                   = 0x10000,
  MMZ_TDEFL_FILTER_MATCHES                = 0x20000,
  MMZ_TDEFL_FORCE_ALL_STATIC_BLOCKS       = 0x40000,
  MMZ_TDEFL_FORCE_ALL_RAW_BLOCKS          = 0x80000
};

// High level compression functions:
// mmz_tdefl_compress_mem_to_heap() compresses a block in memory to a heap block allocated via malloc().
// On entry:
//  pSrc_buf, src_buf_len: Pointer and size of source block to compress.
//  flags: The max match finder probes (default is 128) logically OR'd against the above flags. Higher probes are slower but improve compression.
// On return:
//  Function returns a pointer to the compressed data, or NULL on failure.
//  *pOut_len will be set to the compressed data's size, which could be larger than src_buf_len on uncompressible data.
//  The caller must free() the returned block when it's no longer needed.
void *tdefl_compress_mem_to_heap(const void *pSrc_buf, size_t src_buf_len, size_t *pOut_len, int flags);

// mmz_tdefl_compress_mem_to_mem() compresses a block in memory to another block in memory.
// Returns 0 on failure.
size_t mmz_tdefl_compress_mem_to_mem(void *pOut_buf, size_t out_buf_len, const void *pSrc_buf, size_t src_buf_len, int flags);

// Compresses an image to a compressed PNG file in memory.
// On entry:
//  pImage, w, h, and num_chans describe the image to compress. num_chans may be 1, 2, 3, or 4.
//  The image pitch in bytes per scanline will be w*num_chans. The leftmost pixel on the top scanline is stored first in memory.
//  level may range from [0,10], use MMZ_NO_COMPRESSION, MMZ_BEST_SPEED, MMZ_BEST_COMPRESSION, etc. or a decent default is MMZ_DEFAULT_LEVEL
//  If flip is true, the image will be flipped on the Y axis (useful for OpenGL apps).
// On return:
//  Function returns a pointer to the compressed data, or NULL on failure.
//  *pLen_out will be set to the size of the PNG image file.
//  The caller must mmz_free() the returned heap block (which will typically be larger than *pLen_out) when it's no longer needed.
void *tdefl_write_image_to_png_file_in_memory_ex(const void *pImage, int w, int h, int num_chans, size_t *pLen_out, mmz_uint level, mmz_bool flip);
void *tdefl_write_image_to_png_file_in_memory(const void *pImage, int w, int h, int num_chans, size_t *pLen_out);

// Output stream interface. The compressor uses this interface to write compressed data. It'll typically be called MMZ_TDEFL_OUT_BUF_SIZE at a time.
typedef mmz_bool (*mmz_tdefl_put_buf_func_ptr)(const void* pBuf, int len, void *pUser);

// mmz_tdefl_compress_mem_to_output() compresses a block to an output stream. The above helpers use this function internally.
mmz_bool mmz_tdefl_compress_mem_to_output(const void *pBuf, size_t buf_len, mmz_tdefl_put_buf_func_ptr pPut_buf_func, void *pPut_buf_user, int flags);

enum { MMZ_TDEFL_MAX_HUFF_TABLES = 3, MMZ_TDEFL_MAX_HUFF_SYMBOLS_0 = 288, MMZ_TDEFL_MAX_HUFF_SYMBOLS_1 = 32, MMZ_TDEFL_MAX_HUFF_SYMBOLS_2 = 19, MMZ_TDEFL_LZ_DICT_SIZE = 32768, MMZ_TDEFL_LZ_DICT_SIZE_MASK = MMZ_TDEFL_LZ_DICT_SIZE - 1, MMZ_TDEFL_MIN_MATCH_LEN = 3, MMZ_TDEFL_MAX_MATCH_LEN = 258 };

// MMZ_TDEFL_OUT_BUF_SIZE MUST be large enough to hold a single entire compressed output block (using static/fixed Huffman codes).
#if MMZ_TDEFL_LESS_MEMORY
enum { MMZ_TDEFL_LZ_CODE_BUF_SIZE = 24 * 1024, MMZ_TDEFL_OUT_BUF_SIZE = (MMZ_TDEFL_LZ_CODE_BUF_SIZE * 13 ) / 10, MMZ_TDEFL_MAX_HUFF_SYMBOLS = 288, MMZ_TDEFL_LZ_HASH_BITS = 12, MMZ_TDEFL_LEVEL1_HASH_SIZE_MASK = 4095, MMZ_TDEFL_LZ_HASH_SHIFT = (MMZ_TDEFL_LZ_HASH_BITS + 2) / 3, MMZ_TDEFL_LZ_HASH_SIZE = 1 << MMZ_TDEFL_LZ_HASH_BITS };
#else
enum { MMZ_TDEFL_LZ_CODE_BUF_SIZE = 64 * 1024, MMZ_TDEFL_OUT_BUF_SIZE = (MMZ_TDEFL_LZ_CODE_BUF_SIZE * 13 ) / 10, MMZ_TDEFL_MAX_HUFF_SYMBOLS = 288, MMZ_TDEFL_LZ_HASH_BITS = 15, MMZ_TDEFL_LEVEL1_HASH_SIZE_MASK = 4095, MMZ_TDEFL_LZ_HASH_SHIFT = (MMZ_TDEFL_LZ_HASH_BITS + 2) / 3, MMZ_TDEFL_LZ_HASH_SIZE = 1 << MMZ_TDEFL_LZ_HASH_BITS };
#endif

// The low-level tdefl functions below may be used directly if the above helper functions aren't flexible enough. The low-level functions don't make any heap allocations, unlike the above helper functions.
typedef enum
{
  MMZ_TDEFL_STATUS_BAD_PARAM = -2,
  MMZ_TDEFL_STATUS_PUT_BUF_FAILED = -1,
  MMZ_TDEFL_STATUS_OKAY = 0,
  MMZ_TDEFL_STATUS_DONE = 1,
} mmz_tdefl_status;

// Must map to MMZ_NO_FLUSH, MMZ_SYNC_FLUSH, etc. enums
typedef enum
{
  MMZ_TDEFL_NO_FLUSH = 0,
  MMZ_TDEFL_SYNC_FLUSH = 2,
  MMZ_TDEFL_FULL_FLUSH = 3,
  MMZ_TDEFL_FINISH = 4
} mmz_tdefl_flush;

// tdefl's compression state structure.
typedef struct
{
  mmz_tdefl_put_buf_func_ptr m_pPut_buf_func;
  void *m_pPut_buf_user;
  mmz_uint m_flags, m_max_probes[2];
  int m_greedy_parsing;
  mmz_uint m_adler32, m_lookahead_pos, m_lookahead_size, m_dict_size;
  mmz_uint8 *m_pLZ_code_buf, *m_pLZ_flags, *m_pOutput_buf, *m_pOutput_buf_end;
  mmz_uint m_num_flags_left, m_total_lz_bytes, m_lz_code_buf_dict_pos, m_bits_in, m_bit_buffer;
  mmz_uint m_saved_match_dist, m_saved_match_len, m_saved_lit, m_output_flush_ofs, m_output_flush_remaining, m_finished, m_block_index, m_wants_to_finish;
  mmz_tdefl_status m_prev_return_status;
  const void *m_pIn_buf;
  void *m_pOut_buf;
  size_t *m_pIn_buf_size, *m_pOut_buf_size;
  mmz_tdefl_flush m_flush;
  const mmz_uint8 *m_pSrc;
  size_t m_src_buf_left, m_out_buf_ofs;
  mmz_uint8 m_dict[TDEFL_LZ_DICT_SIZE + MMZ_TDEFL_MAX_MATCH_LEN - 1];
  mmz_uint16 m_huff_count[TDEFL_MAX_HUFF_TABLES][TDEFL_MAX_HUFF_SYMBOLS];
  mmz_uint16 m_huff_codes[TDEFL_MAX_HUFF_TABLES][TDEFL_MAX_HUFF_SYMBOLS];
  mmz_uint8 m_huff_code_sizes[TDEFL_MAX_HUFF_TABLES][TDEFL_MAX_HUFF_SYMBOLS];
  mmz_uint8 m_lz_code_buf[TDEFL_LZ_CODE_BUF_SIZE];
  mmz_uint16 m_next[TDEFL_LZ_DICT_SIZE];
  mmz_uint16 m_hash[TDEFL_LZ_HASH_SIZE];
  mmz_uint8 m_output_buf[TDEFL_OUT_BUF_SIZE];
} mmz_tdefl_compressor;

// Initializes the compressor.
// There is no corresponding deinit() function because the tdefl API's do not dynamically allocate memory.
// pBut_buf_func: If NULL, output data will be supplied to the specified callback. In this case, the user should call the mmz_tdefl_compress_buffer() API for compression.
// If pBut_buf_func is NULL the user should always call the mmz_tdefl_compress() API.
// flags: See the above enums (MMZ_TDEFL_HUFFMAN_ONLY, MMZ_TDEFL_WRITE_ZLIB_HEADER, etc.)
mmz_tdefl_status mmz_tdefl_init(mmz_tdefl_compressor *d, mmz_tdefl_put_buf_func_ptr pPut_buf_func, void *pPut_buf_user, int flags);

// Compresses a block of data, consuming as much of the specified input buffer as possible, and writing as much compressed data to the specified output buffer as possible.
mmz_tdefl_status mmz_tdefl_compress(mmz_tdefl_compressor *d, const void *pIn_buf, size_t *pIn_buf_size, void *pOut_buf, size_t *pOut_buf_size, mmz_tdefl_flush flush);

// mmz_tdefl_compress_buffer() is only usable when the mmz_tdefl_init() is called with a non-NULL mmz_tdefl_put_buf_func_ptr.
// mmz_tdefl_compress_buffer() always consumes the entire input buffer.
mmz_tdefl_status mmz_tdefl_compress_buffer(mmz_tdefl_compressor *d, const void *pIn_buf, size_t in_buf_size, mmz_tdefl_flush flush);

mmz_tdefl_status mmz_tdefl_get_prev_return_status(mmz_tdefl_compressor *d);
mmz_uint32 mmz_tdefl_get_adler32(mmz_tdefl_compressor *d);


// Create mmz_tdefl_compress() flags given zlib-style compression parameters.
// level may range from [0,10] (where 10 is absolute max compression, but may be much slower on some files)
// window_bits may be -15 (raw deflate) or 15 (zlib)
// strategy may be either MMZ_DEFAULT_STRATEGY, MMZ_FILTERED, MMZ_HUFFMAN_ONLY, MMZ_RLE, or MMZ_FIXED
mmz_uint mmz_tdefl_create_comp_flags_from_zip_params(int level, int window_bits, int strategy);


#ifdef __cplusplus
}
#endif

#endif // MINIZ_HEADER_INCLUDED

// ------------------- End of Header: Implementation follows. (If you only want the header, define MINIZ_HEADER_FILE_ONLY.)

#ifndef MINIZ_HEADER_FILE_ONLY

typedef unsigned char mmz_validate_uint16[sizeof(mmz_uint16)==2 ? 1 : -1];
typedef unsigned char mmz_validate_uint32[sizeof(mmz_uint32)==4 ? 1 : -1];
typedef unsigned char mmz_validate_uint64[sizeof(mmz_uint64)==8 ? 1 : -1];

#include <string.h>
#include <assert.h>

#define MMZ_ASSERT(x) assert(x)

  #define MMZ_MALLOC(x) malloc(x)
  #define MMZ_FREE(x) free(x)
  #define MMZ_REALLOC(p, x) realloc(p, x)

#define MMZ_MAX(a,b) (((a)>(b))?(a):(b))
#define MMZ_MIN(a,b) (((a)<(b))?(a):(b))
#define MMZ_CLEAR_OBJ(obj) memset(&(obj), 0, sizeof(obj))

  #define MMZ_READ_LE16(p) *((const mmz_uint16 *)(p))
  #define MMZ_READ_LE32(p) *((const mmz_uint32 *)(p))

#ifdef _MSC_VER
  #define MMZ_FORCEINLINE __forceinline
#elif defined(__GNUC__)
  #define MMZ_FORCEINLINE inline __attribute__((__always_inline__))
#else
  #define MMZ_FORCEINLINE inline
#endif

#ifdef __cplusplus
  extern "C" {
#endif

// ------------------- zlib-style API's

mmz_ulong mmz_adler32(mmz_ulong adler, const unsigned char *ptr, size_t buf_len)
{
  mmz_uint32 i, s1 = (mmz_uint32)(adler & 0xffff), s2 = (mmz_uint32)(adler >> 16); size_t block_len = buf_len % 5552;
  if (!ptr) return MMZ_ADLER32_INIT;
  while (buf_len) {
    for (i = 0; i + 7 < block_len; i += 8, ptr += 8) {
      s1 += ptr[0], s2 += s1; s1 += ptr[1], s2 += s1; s1 += ptr[2], s2 += s1; s1 += ptr[3], s2 += s1;
      s1 += ptr[4], s2 += s1; s1 += ptr[5], s2 += s1; s1 += ptr[6], s2 += s1; s1 += ptr[7], s2 += s1;
    }
    for ( ; i < block_len; ++i) s1 += *ptr++, s2 += s1;
    s1 %= 65521U, s2 %= 65521U; buf_len -= block_len; block_len = 5552;
  }
  return (s2 << 16) + s1;
}

// Karl Malbrain's compact CRC-32. See "A compact CCITT crc16 and crc32 C implementation that balances processor cache usage against speed": http://www.geocities.com/malbrain/
mmz_ulong mmz_crc32(mmz_ulong crc, const mmz_uint8 *ptr, size_t buf_len)
{
  static const mmz_uint32 s_crc32[16] = { 0, 0x1db71064, 0x3b6e20c8, 0x26d930ac, 0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
    0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c, 0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c };
  mmz_uint32 crcu32 = (mmz_uint32)crc;
  if (!ptr) return MMZ_CRC32_INIT;
  crcu32 = ~crcu32; while (buf_len--) { mmz_uint8 b = *ptr++; crcu32 = (crcu32 >> 4) ^ s_crc32[(crcu32 & 0xF) ^ (b & 0xF)]; crcu32 = (crcu32 >> 4) ^ s_crc32[(crcu32 & 0xF) ^ (b >> 4)]; }
  return ~crcu32;
}

void mmz_free(void *p)
{
  MMZ_FREE(p);
}

typedef struct
{
  mmz_tinfl_decompressor m_decomp;
  mmz_uint m_dict_ofs, m_dict_avail, m_first_call, m_has_flushed; int m_window_bits;
  mmz_uint8 m_dict[TINFL_LZ_DICT_SIZE];
  mmz_tinfl_status m_last_status;
} mmz_inflate_state;

int mmz_inflateInit2(mmz_streamp pStream, int window_bits)
{
  mmz_inflate_state *pDecomp;
  if (!pStream) return MMZ_STREAM_ERROR;
  if ((window_bits != MMZ_DEFAULT_WINDOW_BITS) && (-window_bits != MMZ_DEFAULT_WINDOW_BITS)) return MMZ_PARAM_ERROR;

  pStream->data_type = 0;
  pStream->adler = 0;
  pStream->msg = NULL;
  pStream->total_in = 0;
  pStream->total_out = 0;
  pStream->reserved = 0;
  if (!pStream->zalloc) pStream->zalloc = def_alloc_func;
  if (!pStream->zfree) pStream->zfree = def_free_func;

  pDecomp = (mmz_inflate_state*)pStream->zalloc(pStream->opaque, 1, sizeof(mmz_inflate_state));
  if (!pDecomp) return MMZ_MEM_ERROR;

  pStream->state = (struct mmz_internal_state *)pDecomp;

  mmz_tinfl_init(&pDecomp->m_decomp);
  pDecomp->m_dict_ofs = 0;
  pDecomp->m_dict_avail = 0;
  pDecomp->m_last_status = MMZ_TINFL_STATUS_NEEDS_MORE_INPUT;
  pDecomp->m_first_call = 1;
  pDecomp->m_has_flushed = 0;
  pDecomp->m_window_bits = window_bits;

  return MMZ_OK;
}

int mmz_inflateInit(mmz_streamp pStream)
{
   return mmz_inflateInit2(pStream, MMZ_DEFAULT_WINDOW_BITS);
}

int mmz_inflate(mmz_streamp pStream, int flush)
{
  mmz_inflate_state* pState;
  mmz_uint n, first_call, decomp_flags = MMZ_TINFL_FLAG_COMPUTE_ADLER32;
  size_t in_bytes, out_bytes, orig_avail_in;
  mmz_tinfl_status status;

  if ((!pStream) || (!pStream->state)) return MMZ_STREAM_ERROR;
  if (flush == MMZ_PARTIAL_FLUSH) flush = MMZ_SYNC_FLUSH;
  if ((flush) && (flush != MMZ_SYNC_FLUSH) && (flush != MMZ_FINISH)) return MMZ_STREAM_ERROR;

  pState = (mmz_inflate_state*)pStream->state;
  if (pState->m_window_bits > 0) decomp_flags |= MMZ_TINFL_FLAG_PARSE_ZLIB_HEADER;
  orig_avail_in = pStream->avail_in;

  first_call = pState->m_first_call; pState->m_first_call = 0;
  if (pState->m_last_status < 0) return MMZ_DATA_ERROR;

  if (pState->m_has_flushed && (flush != MMZ_FINISH)) return MMZ_STREAM_ERROR;
  pState->m_has_flushed |= (flush == MMZ_FINISH);

  if ((flush == MMZ_FINISH) && (first_call))
  {
    // MMZ_FINISH on the first call implies that the input and output buffers are large enough to hold the entire compressed/decompressed file.
    decomp_flags |= MMZ_TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF;
    in_bytes = pStream->avail_in; out_bytes = pStream->avail_out;
    status = mmz_tinfl_decompress(&pState->m_decomp, pStream->next_in, &in_bytes, pStream->next_out, pStream->next_out, &out_bytes, decomp_flags);
    pState->m_last_status = status;
    pStream->next_in += (mmz_uint)in_bytes; pStream->avail_in -= (mmz_uint)in_bytes; pStream->total_in += (mmz_uint)in_bytes;
    pStream->adler = mmz_tinfl_get_adler32(&pState->m_decomp);
    pStream->next_out += (mmz_uint)out_bytes; pStream->avail_out -= (mmz_uint)out_bytes; pStream->total_out += (mmz_uint)out_bytes;

    if (status < 0)
      return MMZ_DATA_ERROR;
    else if (status != MMZ_TINFL_STATUS_DONE)
    {
      pState->m_last_status = MMZ_TINFL_STATUS_FAILED;
      return MMZ_BUF_ERROR;
    }
    return MMZ_STREAM_END;
  }
  // flush != MMZ_FINISH then we must assume there's more input.
  if (flush != MMZ_FINISH) decomp_flags |= MMZ_TINFL_FLAG_HAS_MORE_INPUT;

  if (pState->m_dict_avail)
  {
    n = MMZ_MIN(pState->m_dict_avail, pStream->avail_out);
    memcpy(pStream->next_out, pState->m_dict + pState->m_dict_ofs, n);
    pStream->next_out += n; pStream->avail_out -= n; pStream->total_out += n;
    pState->m_dict_avail -= n; pState->m_dict_ofs = (pState->m_dict_ofs + n) & (TINFL_LZ_DICT_SIZE - 1);
    return ((pState->m_last_status == MMZ_TINFL_STATUS_DONE) && (!pState->m_dict_avail)) ? MMZ_STREAM_END : MMZ_OK;
  }

  for ( ; ; )
  {
    in_bytes = pStream->avail_in;
    out_bytes = MMZ_TINFL_LZ_DICT_SIZE - pState->m_dict_ofs;

    status = mmz_tinfl_decompress(&pState->m_decomp, pStream->next_in, &in_bytes, pState->m_dict, pState->m_dict + pState->m_dict_ofs, &out_bytes, decomp_flags);
    pState->m_last_status = status;

    pStream->next_in += (mmz_uint)in_bytes; pStream->avail_in -= (mmz_uint)in_bytes;
    pStream->total_in += (mmz_uint)in_bytes; pStream->adler = mmz_tinfl_get_adler32(&pState->m_decomp);

    pState->m_dict_avail = (mmz_uint)out_bytes;

    n = MMZ_MIN(pState->m_dict_avail, pStream->avail_out);
    memcpy(pStream->next_out, pState->m_dict + pState->m_dict_ofs, n);
    pStream->next_out += n; pStream->avail_out -= n; pStream->total_out += n;
    pState->m_dict_avail -= n; pState->m_dict_ofs = (pState->m_dict_ofs + n) & (TINFL_LZ_DICT_SIZE - 1);

    if (status < 0)
       return MMZ_DATA_ERROR; // Stream is corrupted (there could be some uncompressed data left in the output dictionary - oh well).
    else if ((status == MMZ_TINFL_STATUS_NEEDS_MORE_INPUT) && (!orig_avail_in))
      return MMZ_BUF_ERROR; // Signal caller that we can't make forward progress without supplying more input or by setting flush to MMZ_FINISH.
    else if (flush == MMZ_FINISH)
    {
       // The output buffer MUST be large to hold the remaining uncompressed data when flush==MMZ_FINISH.
       if (status == MMZ_TINFL_STATUS_DONE)
          return pState->m_dict_avail ? MMZ_BUF_ERROR : MMZ_STREAM_END;
       // status here must be MMZ_TINFL_STATUS_HAS_MORE_OUTPUT, which means there's at least 1 more byte on the way. If there's no more room left in the output buffer then something is wrong.
       else if (!pStream->avail_out)
          return MMZ_BUF_ERROR;
    }
    else if ((status == MMZ_TINFL_STATUS_DONE) || (!pStream->avail_in) || (!pStream->avail_out) || (pState->m_dict_avail))
      break;
  }

  return ((status == MMZ_TINFL_STATUS_DONE) && (!pState->m_dict_avail)) ? MMZ_STREAM_END : MMZ_OK;
}

int mmz_inflateEnd(mmz_streamp pStream)
{
  if (!pStream)
    return MMZ_STREAM_ERROR;
  if (pStream->state)
  {
    pStream->zfree(pStream->opaque, pStream->state);
    pStream->state = NULL;
  }
  return MMZ_OK;
}

int mmz_uncompress(unsigned char *pDest, mmz_ulong *pDest_len, const unsigned char *pSource, mmz_ulong source_len)
{
  mmz_stream stream;
  int status;
  memset(&stream, 0, sizeof(stream));

  // In case mmz_ulong is 64-bits (argh I hate longs).
  if ((source_len | *pDest_len) > 0xFFFFFFFFU) return MMZ_PARAM_ERROR;

  stream.next_in = pSource;
  stream.avail_in = (mmz_uint32)source_len;
  stream.next_out = pDest;
  stream.avail_out = (mmz_uint32)*pDest_len;

  status = mmz_inflateInit(&stream);
  if (status != MMZ_OK)
    return status;

  status = mmz_inflate(&stream, MMZ_FINISH);
  if (status != MMZ_STREAM_END)
  {
    mmz_inflateEnd(&stream);
    return ((status == MMZ_BUF_ERROR) && (!stream.avail_in)) ? MMZ_DATA_ERROR : status;
  }
  *pDest_len = stream.total_out;

  return mmz_inflateEnd(&stream);
}

const char *mmz_error(int err)
{
  static struct { int m_err; const char *m_pDesc; } s_error_descs[] =
  {
    { MMZ_OK, "" }, { MMZ_STREAM_END, "stream end" }, { MMZ_NEED_DICT, "need dictionary" }, { MMZ_ERRNO, "file error" }, { MMZ_STREAM_ERROR, "stream error" },
    { MMZ_DATA_ERROR, "data error" }, { MMZ_MEM_ERROR, "out of memory" }, { MMZ_BUF_ERROR, "buf error" }, { MMZ_VERSION_ERROR, "version error" }, { MMZ_PARAM_ERROR, "parameter error" }
  };
  mmz_uint i; for (i = 0; i < sizeof(s_error_descs) / sizeof(s_error_descs[0]); ++i) if (s_error_descs[i].m_err == err) return s_error_descs[i].m_pDesc;
  return NULL;
}


// ------------------- Low-level Decompression (completely independent from all compression API's)

#define MMZ_TINFL_MEMCPY(d, s, l) memcpy(d, s, l)
#define MMZ_TINFL_MEMSET(p, c, l) memset(p, c, l)

#define MMZ_TINFL_CR_BEGIN switch(r->m_state) { case 0:
#define MMZ_TINFL_CR_RETURN(state_index, result) do { status = result; r->m_state = state_index; goto common_exit; case state_index:; } MMZ_MACRO_END
#define MMZ_TINFL_CR_RETURN_FOREVER(state_index, result) do { for ( ; ; ) { MMZ_TINFL_CR_RETURN(state_index, result); } } MMZ_MACRO_END
#define MMZ_TINFL_CR_FINISH }

// TODO: If the caller has indicated that there's no more input, and we attempt to read beyond the input buf, then something is wrong with the input because the inflator never
// reads ahead more than it needs to. Currently MMZ_TINFL_GET_BYTE() pads the end of the stream with 0's in this scenario.
#define MMZ_TINFL_GET_BYTE(state_index, c) do { \
  if (pIn_buf_cur >= pIn_buf_end) { \
    for ( ; ; ) { \
      if (decomp_flags & MMZ_TINFL_FLAG_HAS_MORE_INPUT) { \
        MMZ_TINFL_CR_RETURN(state_index, MMZ_TINFL_STATUS_NEEDS_MORE_INPUT); \
        if (pIn_buf_cur < pIn_buf_end) { \
          c = *pIn_buf_cur++; \
          break; \
        } \
      } else { \
        c = 0; \
        break; \
      } \
    } \
  } else c = *pIn_buf_cur++; } MMZ_MACRO_END

#define MMZ_TINFL_NEED_BITS(state_index, n) do { mmz_uint c; MMZ_TINFL_GET_BYTE(state_index, c); bit_buf |= (((tinfl_bit_buf_t)c) << num_bits); num_bits += 8; } while (num_bits < (mmz_uint)(n))
#define MMZ_TINFL_SKIP_BITS(state_index, n) do { if (num_bits < (mmz_uint)(n)) { MMZ_TINFL_NEED_BITS(state_index, n); } bit_buf >>= (n); num_bits -= (n); } MMZ_MACRO_END
#define MMZ_TINFL_GET_BITS(state_index, b, n) do { if (num_bits < (mmz_uint)(n)) { MMZ_TINFL_NEED_BITS(state_index, n); } b = bit_buf & ((1 << (n)) - 1); bit_buf >>= (n); num_bits -= (n); } MMZ_MACRO_END

// MMZ_TINFL_HUFF_BITBUF_FILL() is only used rarely, when the number of bytes remaining in the input buffer falls below 2.
// It reads just enough bytes from the input stream that are needed to decode the next Huffman code (and absolutely no more). It works by trying to fully decode a
// Huffman code by using whatever bits are currently present in the bit buffer. If this fails, it reads another byte, and tries again until it succeeds or until the
// bit buffer contains >=15 bits (deflate's max. Huffman code size).
#define MMZ_TINFL_HUFF_BITBUF_FILL(state_index, pHuff) \
  do { \
    temp = (pHuff)->m_look_up[bit_buf & (TINFL_FAST_LOOKUP_SIZE - 1)]; \
    if (temp >= 0) { \
      code_len = temp >> 9; \
      if ((code_len) && (num_bits >= code_len)) \
      break; \
    } else if (num_bits > MMZ_TINFL_FAST_LOOKUP_BITS) { \
       code_len = MMZ_TINFL_FAST_LOOKUP_BITS; \
       do { \
          temp = (pHuff)->m_tree[~temp + ((bit_buf >> code_len++) & 1)]; \
       } while ((temp < 0) && (num_bits >= (code_len + 1))); if (temp >= 0) break; \
    } MMZ_TINFL_GET_BYTE(state_index, c); bit_buf |= (((tinfl_bit_buf_t)c) << num_bits); num_bits += 8; \
  } while (num_bits < 15);

// MMZ_TINFL_HUFF_DECODE() decodes the next Huffman coded symbol. It's more complex than you would initially expect because the zlib API expects the decompressor to never read
// beyond the final byte of the deflate stream. (In other words, when this macro wants to read another byte from the input, it REALLY needs another byte in order to fully
// decode the next Huffman code.) Handling this properly is particularly important on raw deflate (non-zlib) streams, which aren't followed by a byte aligned adler-32.
// The slow path is only executed at the very end of the input buffer.
#define MMZ_TINFL_HUFF_DECODE(state_index, sym, pHuff) do { \
  int temp; mmz_uint code_len, c; \
  if (num_bits < 15) { \
    if ((pIn_buf_end - pIn_buf_cur) < 2) { \
       MMZ_TINFL_HUFF_BITBUF_FILL(state_index, pHuff); \
    } else { \
       bit_buf |= (((tinfl_bit_buf_t)pIn_buf_cur[0]) << num_bits) | (((tinfl_bit_buf_t)pIn_buf_cur[1]) << (num_bits + 8)); pIn_buf_cur += 2; num_bits += 16; \
    } \
  } \
  if ((temp = (pHuff)->m_look_up[bit_buf & (TINFL_FAST_LOOKUP_SIZE - 1)]) >= 0) \
    code_len = temp >> 9, temp &= 511; \
  else { \
    code_len = MMZ_TINFL_FAST_LOOKUP_BITS; do { temp = (pHuff)->m_tree[~temp + ((bit_buf >> code_len++) & 1)]; } while (temp < 0); \
  } sym = temp; bit_buf >>= code_len; num_bits -= code_len; } MMZ_MACRO_END

mmz_tinfl_status mmz_tinfl_decompress(mmz_tinfl_decompressor *r, const mmz_uint8 *pIn_buf_next, size_t *pIn_buf_size, mmz_uint8 *pOut_buf_start, mmz_uint8 *pOut_buf_next, size_t *pOut_buf_size, const mmz_uint32 decomp_flags)
{
  static const int s_length_base[31] = { 3,4,5,6,7,8,9,10,11,13, 15,17,19,23,27,31,35,43,51,59, 67,83,99,115,131,163,195,227,258,0,0 };
  static const int s_length_extra[31]= { 0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0,0,0 };
  static const int s_dist_base[32] = { 1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193, 257,385,513,769,1025,1537,2049,3073,4097,6145,8193,12289,16385,24577,0,0};
  static const int s_dist_extra[32] = { 0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13};
  static const mmz_uint8 s_length_dezigzag[19] = { 16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15 };
  static const int s_min_table_sizes[3] = { 257, 1, 4 };

  mmz_tinfl_status status = MMZ_TINFL_STATUS_FAILED; mmz_uint32 num_bits, dist, counter, num_extra; mmz_tinfl_bit_buf_t bit_buf;
  const mmz_uint8 *pIn_buf_cur = pIn_buf_next, *const pIn_buf_end = pIn_buf_next + *pIn_buf_size;
  mmz_uint8 *pOut_buf_cur = pOut_buf_next, *const pOut_buf_end = pOut_buf_next + *pOut_buf_size;
  size_t out_buf_size_mask = (decomp_flags & MMZ_TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF) ? (size_t)-1 : ((pOut_buf_next - pOut_buf_start) + *pOut_buf_size) - 1, dist_from_out_buf_start;

  // Ensure the output buffer's size is a power of 2, unless the output buffer is large enough to hold the entire output file (in which case it doesn't matter).
  if (((out_buf_size_mask + 1) & out_buf_size_mask) || (pOut_buf_next < pOut_buf_start)) { *pIn_buf_size = *pOut_buf_size = 0; return MMZ_TINFL_STATUS_BAD_PARAM; }

  num_bits = r->m_num_bits; bit_buf = r->m_bit_buf; dist = r->m_dist; counter = r->m_counter; num_extra = r->m_num_extra; dist_from_out_buf_start = r->m_dist_from_out_buf_start;
  MMZ_TINFL_CR_BEGIN

  bit_buf = num_bits = dist = counter = num_extra = r->m_zhdr0 = r->m_zhdr1 = 0; r->m_z_adler32 = r->m_check_adler32 = 1;
  if (decomp_flags & MMZ_TINFL_FLAG_PARSE_ZLIB_HEADER)
  {
    MMZ_TINFL_GET_BYTE(1, r->m_zhdr0); MMZ_TINFL_GET_BYTE(2, r->m_zhdr1);
    counter = (((r->m_zhdr0 * 256 + r->m_zhdr1) % 31 != 0) || (r->m_zhdr1 & 32) || ((r->m_zhdr0 & 15) != 8));
    if (!(decomp_flags & MMZ_TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF)) counter |= (((1U << (8U + (r->m_zhdr0 >> 4))) > 32768U) || ((out_buf_size_mask + 1) < (size_t)(1U << (8U + (r->m_zhdr0 >> 4)))));
    if (counter) { MMZ_TINFL_CR_RETURN_FOREVER(36, MMZ_TINFL_STATUS_FAILED); }
  }

  do
  {
    MMZ_TINFL_GET_BITS(3, r->m_final, 3); r->m_type = r->m_final >> 1;
    if (r->m_type == 0)
    {
      MMZ_TINFL_SKIP_BITS(5, num_bits & 7);
      for (counter = 0; counter < 4; ++counter) { if (num_bits) MMZ_TINFL_GET_BITS(6, r->m_raw_header[counter], 8); else MMZ_TINFL_GET_BYTE(7, r->m_raw_header[counter]); }
      if ((counter = (r->m_raw_header[0] | (r->m_raw_header[1] << 8))) != (mmz_uint)(0xFFFF ^ (r->m_raw_header[2] | (r->m_raw_header[3] << 8)))) { MMZ_TINFL_CR_RETURN_FOREVER(39, MMZ_TINFL_STATUS_FAILED); }
      while ((counter) && (num_bits))
      {
        MMZ_TINFL_GET_BITS(51, dist, 8);
        while (pOut_buf_cur >= pOut_buf_end) { MMZ_TINFL_CR_RETURN(52, MMZ_TINFL_STATUS_HAS_MORE_OUTPUT); }
        *pOut_buf_cur++ = (mmz_uint8)dist;
        counter--;
      }
      while (counter)
      {
        size_t n; while (pOut_buf_cur >= pOut_buf_end) { MMZ_TINFL_CR_RETURN(9, MMZ_TINFL_STATUS_HAS_MORE_OUTPUT); }
        while (pIn_buf_cur >= pIn_buf_end)
        {
          if (decomp_flags & MMZ_TINFL_FLAG_HAS_MORE_INPUT)
          {
            MMZ_TINFL_CR_RETURN(38, MMZ_TINFL_STATUS_NEEDS_MORE_INPUT);
          }
          else
          {
            MMZ_TINFL_CR_RETURN_FOREVER(40, MMZ_TINFL_STATUS_FAILED);
          }
        }
        n = MMZ_MIN(MMZ_MIN((size_t)(pOut_buf_end - pOut_buf_cur), (size_t)(pIn_buf_end - pIn_buf_cur)), counter);
        MMZ_TINFL_MEMCPY(pOut_buf_cur, pIn_buf_cur, n); pIn_buf_cur += n; pOut_buf_cur += n; counter -= (mmz_uint)n;
      }
    }
    else if (r->m_type == 3)
    {
      MMZ_TINFL_CR_RETURN_FOREVER(10, MMZ_TINFL_STATUS_FAILED);
    }
    else
    {
      if (r->m_type == 1)
      {
        mmz_uint8 *p = r->m_tables[0].m_code_size; mmz_uint i;
        r->m_table_sizes[0] = 288; r->m_table_sizes[1] = 32; MMZ_TINFL_MEMSET(r->m_tables[1].m_code_size, 5, 32);
        for ( i = 0; i <= 143; ++i) *p++ = 8; for ( ; i <= 255; ++i) *p++ = 9; for ( ; i <= 279; ++i) *p++ = 7; for ( ; i <= 287; ++i) *p++ = 8;
      }
      else
      {
        for (counter = 0; counter < 3; counter++) { MMZ_TINFL_GET_BITS(11, r->m_table_sizes[counter], "\05\05\04"[counter]); r->m_table_sizes[counter] += s_min_table_sizes[counter]; }
        MMZ_CLEAR_OBJ(r->m_tables[2].m_code_size); for (counter = 0; counter < r->m_table_sizes[2]; counter++) { mmz_uint s; MMZ_TINFL_GET_BITS(14, s, 3); r->m_tables[2].m_code_size[s_length_dezigzag[counter]] = (mmz_uint8)s; }
        r->m_table_sizes[2] = 19;
      }
      for ( ; (int)r->m_type >= 0; r->m_type--)
      {
        int tree_next, tree_cur; mmz_tinfl_huff_table *pTable;
        mmz_uint i, j, used_syms, total, sym_index, next_code[17], total_syms[16]; pTable = &r->m_tables[r->m_type]; MMZ_CLEAR_OBJ(total_syms); MMZ_CLEAR_OBJ(pTable->m_look_up); MMZ_CLEAR_OBJ(pTable->m_tree);
        for (i = 0; i < r->m_table_sizes[r->m_type]; ++i) total_syms[pTable->m_code_size[i]]++;
        used_syms = 0, total = 0; next_code[0] = next_code[1] = 0;
        for (i = 1; i <= 15; ++i) { used_syms += total_syms[i]; next_code[i + 1] = (total = ((total + total_syms[i]) << 1)); }
        if ((65536 != total) && (used_syms > 1))
        {
          MMZ_TINFL_CR_RETURN_FOREVER(35, MMZ_TINFL_STATUS_FAILED);
        }
        for (tree_next = -1, sym_index = 0; sym_index < r->m_table_sizes[r->m_type]; ++sym_index)
        {
          mmz_uint rev_code = 0, l, cur_code, code_size = pTable->m_code_size[sym_index]; if (!code_size) continue;
          cur_code = next_code[code_size]++; for (l = code_size; l > 0; l--, cur_code >>= 1) rev_code = (rev_code << 1) | (cur_code & 1);
          if (code_size <= MMZ_TINFL_FAST_LOOKUP_BITS) { mmz_int16 k = (mmz_int16)((code_size << 9) | sym_index); while (rev_code < MMZ_TINFL_FAST_LOOKUP_SIZE) { pTable->m_look_up[rev_code] = k; rev_code += (1 << code_size); } continue; }
          if (0 == (tree_cur = pTable->m_look_up[rev_code & (TINFL_FAST_LOOKUP_SIZE - 1)])) { pTable->m_look_up[rev_code & (TINFL_FAST_LOOKUP_SIZE - 1)] = (mmz_int16)tree_next; tree_cur = tree_next; tree_next -= 2; }
          rev_code >>= (TINFL_FAST_LOOKUP_BITS - 1);
          for (j = code_size; j > (TINFL_FAST_LOOKUP_BITS + 1); j--)
          {
            tree_cur -= ((rev_code >>= 1) & 1);
            if (!pTable->m_tree[-tree_cur - 1]) { pTable->m_tree[-tree_cur - 1] = (mmz_int16)tree_next; tree_cur = tree_next; tree_next -= 2; } else tree_cur = pTable->m_tree[-tree_cur - 1];
          }
          tree_cur -= ((rev_code >>= 1) & 1); pTable->m_tree[-tree_cur - 1] = (mmz_int16)sym_index;
        }
        if (r->m_type == 2)
        {
          for (counter = 0; counter < (r->m_table_sizes[0] + r->m_table_sizes[1]); )
          {
            mmz_uint s; MMZ_TINFL_HUFF_DECODE(16, dist, &r->m_tables[2]); if (dist < 16) { r->m_len_codes[counter++] = (mmz_uint8)dist; continue; }
            if ((dist == 16) && (!counter))
            {
              MMZ_TINFL_CR_RETURN_FOREVER(17, MMZ_TINFL_STATUS_FAILED);
            }
            num_extra = "\02\03\07"[dist - 16]; MMZ_TINFL_GET_BITS(18, s, num_extra); s += "\03\03\013"[dist - 16];
            MMZ_TINFL_MEMSET(r->m_len_codes + counter, (dist == 16) ? r->m_len_codes[counter - 1] : 0, s); counter += s;
          }
          if ((r->m_table_sizes[0] + r->m_table_sizes[1]) != counter)
          {
            MMZ_TINFL_CR_RETURN_FOREVER(21, MMZ_TINFL_STATUS_FAILED);
          }
          MMZ_TINFL_MEMCPY(r->m_tables[0].m_code_size, r->m_len_codes, r->m_table_sizes[0]); MMZ_TINFL_MEMCPY(r->m_tables[1].m_code_size, r->m_len_codes + r->m_table_sizes[0], r->m_table_sizes[1]);
        }
      }
      for ( ; ; )
      {
        mmz_uint8 *pSrc;
        for ( ; ; )
        {
          if (((pIn_buf_end - pIn_buf_cur) < 4) || ((pOut_buf_end - pOut_buf_cur) < 2))
          {
            MMZ_TINFL_HUFF_DECODE(23, counter, &r->m_tables[0]);
            if (counter >= 256)
              break;
            while (pOut_buf_cur >= pOut_buf_end) { MMZ_TINFL_CR_RETURN(24, MMZ_TINFL_STATUS_HAS_MORE_OUTPUT); }
            *pOut_buf_cur++ = (mmz_uint8)counter;
          }
          else
          {
            int sym2; mmz_uint code_len;
#if MMZ_TINFL_USE_64BIT_BITBUF
            if (num_bits < 30) { bit_buf |= (((tinfl_bit_buf_t)MMZ_READ_LE32(pIn_buf_cur)) << num_bits); pIn_buf_cur += 4; num_bits += 32; }
#else
            if (num_bits < 15) { bit_buf |= (((tinfl_bit_buf_t)MMZ_READ_LE16(pIn_buf_cur)) << num_bits); pIn_buf_cur += 2; num_bits += 16; }
#endif
            if ((sym2 = r->m_tables[0].m_look_up[bit_buf & (TINFL_FAST_LOOKUP_SIZE - 1)]) >= 0)
              code_len = sym2 >> 9;
            else
            {
              code_len = MMZ_TINFL_FAST_LOOKUP_BITS; do { sym2 = r->m_tables[0].m_tree[~sym2 + ((bit_buf >> code_len++) & 1)]; } while (sym2 < 0);
            }
            counter = sym2; bit_buf >>= code_len; num_bits -= code_len;
            if (counter & 256)
              break;

#if !TINFL_USE_64BIT_BITBUF
            if (num_bits < 15) { bit_buf |= (((tinfl_bit_buf_t)MMZ_READ_LE16(pIn_buf_cur)) << num_bits); pIn_buf_cur += 2; num_bits += 16; }
#endif
            if ((sym2 = r->m_tables[0].m_look_up[bit_buf & (TINFL_FAST_LOOKUP_SIZE - 1)]) >= 0)
              code_len = sym2 >> 9;
            else
            {
              code_len = MMZ_TINFL_FAST_LOOKUP_BITS; do { sym2 = r->m_tables[0].m_tree[~sym2 + ((bit_buf >> code_len++) & 1)]; } while (sym2 < 0);
            }
            bit_buf >>= code_len; num_bits -= code_len;

            pOut_buf_cur[0] = (mmz_uint8)counter;
            if (sym2 & 256)
            {
              pOut_buf_cur++;
              counter = sym2;
              break;
            }
            pOut_buf_cur[1] = (mmz_uint8)sym2;
            pOut_buf_cur += 2;
          }
        }
        if ((counter &= 511) == 256) break;

        num_extra = s_length_extra[counter - 257]; counter = s_length_base[counter - 257];
        if (num_extra) { mmz_uint extra_bits; MMZ_TINFL_GET_BITS(25, extra_bits, num_extra); counter += extra_bits; }

        MMZ_TINFL_HUFF_DECODE(26, dist, &r->m_tables[1]);
        num_extra = s_dist_extra[dist]; dist = s_dist_base[dist];
        if (num_extra) { mmz_uint extra_bits; MMZ_TINFL_GET_BITS(27, extra_bits, num_extra); dist += extra_bits; }

        dist_from_out_buf_start = pOut_buf_cur - pOut_buf_start;
        if ((dist > dist_from_out_buf_start) && (decomp_flags & MMZ_TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF))
        {
          MMZ_TINFL_CR_RETURN_FOREVER(37, MMZ_TINFL_STATUS_FAILED);
        }

        pSrc = pOut_buf_start + ((dist_from_out_buf_start - dist) & out_buf_size_mask);

        if ((MMZ_MAX(pOut_buf_cur, pSrc) + counter) > pOut_buf_end)
        {
          while (counter--)
          {
            while (pOut_buf_cur >= pOut_buf_end) { MMZ_TINFL_CR_RETURN(53, MMZ_TINFL_STATUS_HAS_MORE_OUTPUT); }
            *pOut_buf_cur++ = pOut_buf_start[(dist_from_out_buf_start++ - dist) & out_buf_size_mask];
          }
          continue;
        }
        else if ((counter >= 9) && (counter <= dist))
        {
          const mmz_uint8 *pSrc_end = pSrc + (counter & ~7);
          do
          {
            ((mmz_uint32 *)pOut_buf_cur)[0] = ((const mmz_uint32 *)pSrc)[0];
            ((mmz_uint32 *)pOut_buf_cur)[1] = ((const mmz_uint32 *)pSrc)[1];
            pOut_buf_cur += 8;
          } while ((pSrc += 8) < pSrc_end);
          if ((counter &= 7) < 3)
          {
            if (counter)
            {
              pOut_buf_cur[0] = pSrc[0];
              if (counter > 1)
                pOut_buf_cur[1] = pSrc[1];
              pOut_buf_cur += counter;
            }
            continue;
          }
        }
        do
        {
          pOut_buf_cur[0] = pSrc[0];
          pOut_buf_cur[1] = pSrc[1];
          pOut_buf_cur[2] = pSrc[2];
          pOut_buf_cur += 3; pSrc += 3;
        } while ((int)(counter -= 3) > 2);
        if ((int)counter > 0)
        {
          pOut_buf_cur[0] = pSrc[0];
          if ((int)counter > 1)
            pOut_buf_cur[1] = pSrc[1];
          pOut_buf_cur += counter;
        }
      }
    }
  } while (!(r->m_final & 1));
  if (decomp_flags & MMZ_TINFL_FLAG_PARSE_ZLIB_HEADER)
  {
    MMZ_TINFL_SKIP_BITS(32, num_bits & 7); for (counter = 0; counter < 4; ++counter) { mmz_uint s; if (num_bits) MMZ_TINFL_GET_BITS(41, s, 8); else MMZ_TINFL_GET_BYTE(42, s); r->m_z_adler32 = (r->m_z_adler32 << 8) | s; }
  }
  MMZ_TINFL_CR_RETURN_FOREVER(34, MMZ_TINFL_STATUS_DONE);
  MMZ_TINFL_CR_FINISH

common_exit:
  r->m_num_bits = num_bits; r->m_bit_buf = bit_buf; r->m_dist = dist; r->m_counter = counter; r->m_num_extra = num_extra; r->m_dist_from_out_buf_start = dist_from_out_buf_start;
  *pIn_buf_size = pIn_buf_cur - pIn_buf_next; *pOut_buf_size = pOut_buf_cur - pOut_buf_next;
  if ((decomp_flags & (MMZ_TINFL_FLAG_PARSE_ZLIB_HEADER | MMZ_TINFL_FLAG_COMPUTE_ADLER32)) && (status >= 0))
  {
    const mmz_uint8 *ptr = pOut_buf_next; size_t buf_len = *pOut_buf_size;
    mmz_uint32 i, s1 = r->m_check_adler32 & 0xffff, s2 = r->m_check_adler32 >> 16; size_t block_len = buf_len % 5552;
    while (buf_len)
    {
      for (i = 0; i + 7 < block_len; i += 8, ptr += 8)
      {
        s1 += ptr[0], s2 += s1; s1 += ptr[1], s2 += s1; s1 += ptr[2], s2 += s1; s1 += ptr[3], s2 += s1;
        s1 += ptr[4], s2 += s1; s1 += ptr[5], s2 += s1; s1 += ptr[6], s2 += s1; s1 += ptr[7], s2 += s1;
      }
      for ( ; i < block_len; ++i) s1 += *ptr++, s2 += s1;
      s1 %= 65521U, s2 %= 65521U; buf_len -= block_len; block_len = 5552;
    }
    r->m_check_adler32 = (s2 << 16) + s1; if ((status == MMZ_TINFL_STATUS_DONE) && (decomp_flags & MMZ_TINFL_FLAG_PARSE_ZLIB_HEADER) && (r->m_check_adler32 != r->m_z_adler32)) status = MMZ_TINFL_STATUS_ADLER32_MISMATCH;
  }
  return status;
}

// Higher level helper functions.
void *mmz_tinfl_decompress_mem_to_heap(const void *pSrc_buf, size_t src_buf_len, size_t *pOut_len, int flags)
{
  mmz_tinfl_decompressor decomp; void *pBuf = NULL, *pNew_buf; size_t src_buf_ofs = 0, out_buf_capacity = 0;
  *pOut_len = 0;
  mmz_tinfl_init(&decomp);
  for ( ; ; )
  {
    size_t src_buf_size = src_buf_len - src_buf_ofs, dst_buf_size = out_buf_capacity - *pOut_len, new_out_buf_capacity;
    mmz_tinfl_status status = mmz_tinfl_decompress(&decomp, (const mmz_uint8*)pSrc_buf + src_buf_ofs, &src_buf_size, (mmz_uint8*)pBuf, pBuf ? (mmz_uint8*)pBuf + *pOut_len : NULL, &dst_buf_size,
      (flags & ~MMZ_TINFL_FLAG_HAS_MORE_INPUT) | MMZ_TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF);
    if ((status < 0) || (status == MMZ_TINFL_STATUS_NEEDS_MORE_INPUT))
    {
      MMZ_FREE(pBuf); *pOut_len = 0; return NULL;
    }
    src_buf_ofs += src_buf_size;
    *pOut_len += dst_buf_size;
    if (status == MMZ_TINFL_STATUS_DONE) break;
    new_out_buf_capacity = out_buf_capacity * 2; if (new_out_buf_capacity < 128) new_out_buf_capacity = 128;
    pNew_buf = MMZ_REALLOC(pBuf, new_out_buf_capacity);
    if (!pNew_buf)
    {
      MMZ_FREE(pBuf); *pOut_len = 0; return NULL;
    }
    pBuf = pNew_buf; out_buf_capacity = new_out_buf_capacity;
  }
  return pBuf;
}

size_t mmz_tinfl_decompress_mem_to_mem(void *pOut_buf, size_t out_buf_len, const void *pSrc_buf, size_t src_buf_len, int flags)
{
  mmz_tinfl_decompressor decomp; mmz_tinfl_status status; mmz_tinfl_init(&decomp);
  status = mmz_tinfl_decompress(&decomp, (const mmz_uint8*)pSrc_buf, &src_buf_len, (mmz_uint8*)pOut_buf, (mmz_uint8*)pOut_buf, &out_buf_len, (flags & ~MMZ_TINFL_FLAG_HAS_MORE_INPUT) | MMZ_TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF);
  return (status != MMZ_TINFL_STATUS_DONE) ? MMZ_TINFL_DECOMPRESS_MEM_TO_MEM_FAILED : out_buf_len;
}

int mmz_tinfl_decompress_mem_to_callback(const void *pIn_buf, size_t *pIn_buf_size, mmz_tinfl_put_buf_func_ptr pPut_buf_func, void *pPut_buf_user, int flags)
{
  int result = 0;
  mmz_tinfl_decompressor decomp;
  mmz_uint8 *pDict = (mmz_uint8*)MMZ_MALLOC(TINFL_LZ_DICT_SIZE); size_t in_buf_ofs = 0, dict_ofs = 0;
  if (!pDict)
    return MMZ_TINFL_STATUS_FAILED;
  mmz_tinfl_init(&decomp);
  for ( ; ; )
  {
    size_t in_buf_size = *pIn_buf_size - in_buf_ofs, dst_buf_size = MMZ_TINFL_LZ_DICT_SIZE - dict_ofs;
    mmz_tinfl_status status = mmz_tinfl_decompress(&decomp, (const mmz_uint8*)pIn_buf + in_buf_ofs, &in_buf_size, pDict, pDict + dict_ofs, &dst_buf_size,
      (flags & ~(MMZ_TINFL_FLAG_HAS_MORE_INPUT | MMZ_TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF)));
    in_buf_ofs += in_buf_size;
    if ((dst_buf_size) && (!(*pPut_buf_func)(pDict + dict_ofs, (int)dst_buf_size, pPut_buf_user)))
      break;
    if (status != MMZ_TINFL_STATUS_HAS_MORE_OUTPUT)
    {
      result = (status == MMZ_TINFL_STATUS_DONE);
      break;
    }
    dict_ofs = (dict_ofs + dst_buf_size) & (TINFL_LZ_DICT_SIZE - 1);
  }
  MMZ_FREE(pDict);
  *pIn_buf_size = in_buf_ofs;
  return result;
}

// ------------------- Low-level Compression (independent from all decompression API's)

// Purposely making these tables static for faster init and thread safety.
static const mmz_uint16 mmz_s_tdefl_len_sym[256] = {
  257,258,259,260,261,262,263,264,265,265,266,266,267,267,268,268,269,269,269,269,270,270,270,270,271,271,271,271,272,272,272,272,
  273,273,273,273,273,273,273,273,274,274,274,274,274,274,274,274,275,275,275,275,275,275,275,275,276,276,276,276,276,276,276,276,
  277,277,277,277,277,277,277,277,277,277,277,277,277,277,277,277,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,
  279,279,279,279,279,279,279,279,279,279,279,279,279,279,279,279,280,280,280,280,280,280,280,280,280,280,280,280,280,280,280,280,
  281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,
  282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,
  283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,
  284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,285 };

static const mmz_uint8 mmz_s_tdefl_len_extra[256] = {
  0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
  4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
  5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
  5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,0 };

static const mmz_uint8 mmz_s_tdefl_small_dist_sym[512] = {
  0,1,2,3,4,4,5,5,6,6,6,6,7,7,7,7,8,8,8,8,8,8,8,8,9,9,9,9,9,9,9,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,11,11,11,11,11,11,
  11,11,11,11,11,11,11,11,11,11,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,13,
  13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,14,14,14,14,14,14,14,14,14,14,14,14,
  14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,
  14,14,14,14,14,14,14,14,14,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
  15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,16,16,16,16,16,16,16,16,16,16,16,16,16,
  16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
  16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
  16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,17,17,17,17,17,17,17,17,17,17,17,17,17,17,
  17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,
  17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,
  17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17 };

static const mmz_uint8 mmz_s_tdefl_small_dist_extra[512] = {
  0,0,0,0,1,1,1,1,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,
  5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
  6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
  6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
  7,7,7,7,7,7,7,7 };

static const mmz_uint8 mmz_s_tdefl_large_dist_sym[128] = {
  0,0,18,19,20,20,21,21,22,22,22,22,23,23,23,23,24,24,24,24,24,24,24,24,25,25,25,25,25,25,25,25,26,26,26,26,26,26,26,26,26,26,26,26,
  26,26,26,26,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,
  28,28,28,28,28,28,28,28,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29 };

static const mmz_uint8 mmz_s_tdefl_large_dist_extra[128] = {
  0,0,8,8,9,9,9,9,10,10,10,10,10,10,10,10,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,
  12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,
  13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13 };

// Radix sorts mmz_tdefl_sym_freq[] array by 16-bit key m_key. Returns ptr to sorted values.
typedef struct { mmz_uint16 m_key, m_sym_index; } mmz_tdefl_sym_freq;
static mmz_tdefl_sym_freq* mmz_tdefl_radix_sort_syms(mmz_uint num_syms, mmz_tdefl_sym_freq* pSyms0, mmz_tdefl_sym_freq* pSyms1)
{
  mmz_uint32 total_passes = 2, pass_shift, pass, i, hist[256 * 2]; mmz_tdefl_sym_freq* pCur_syms = pSyms0, *pNew_syms = pSyms1; MMZ_CLEAR_OBJ(hist);
  for (i = 0; i < num_syms; i++) { mmz_uint freq = pSyms0[i].m_key; hist[freq & 0xFF]++; hist[256 + ((freq >> 8) & 0xFF)]++; }
  while ((total_passes > 1) && (num_syms == hist[(total_passes - 1) * 256])) total_passes--;
  for (pass_shift = 0, pass = 0; pass < total_passes; pass++, pass_shift += 8)
  {
    const mmz_uint32* pHist = &hist[pass << 8];
    mmz_uint offsets[256], cur_ofs = 0;
    for (i = 0; i < 256; i++) { offsets[i] = cur_ofs; cur_ofs += pHist[i]; }
    for (i = 0; i < num_syms; i++) pNew_syms[offsets[(pCur_syms[i].m_key >> pass_shift) & 0xFF]++] = pCur_syms[i];
    { mmz_tdefl_sym_freq* t = pCur_syms; pCur_syms = pNew_syms; pNew_syms = t; }
  }
  return pCur_syms;
}

// mmz_tdefl_calculate_minimum_redundancy() originally written by: Alistair Moffat, alistair@cs.mu.oz.au, Jyrki Katajainen, jyrki@diku.dk, November 1996.
static void mmz_tdefl_calculate_minimum_redundancy(mmz_tdefl_sym_freq *A, int n)
{
  int root, leaf, next, avbl, used, dpth;
  if (n==0) return; else if (n==1) { A[0].m_key = 1; return; }
  A[0].m_key += A[1].m_key; root = 0; leaf = 2;
  for (next=1; next < n-1; next++)
  {
    if (leaf>=n || A[root].m_key<A[leaf].m_key) { A[next].m_key = A[root].m_key; A[root++].m_key = (mmz_uint16)next; } else A[next].m_key = A[leaf++].m_key;
    if (leaf>=n || (root<next && A[root].m_key<A[leaf].m_key)) { A[next].m_key = (mmz_uint16)(A[next].m_key + A[root].m_key); A[root++].m_key = (mmz_uint16)next; } else A[next].m_key = (mmz_uint16)(A[next].m_key + A[leaf++].m_key);
  }
  A[n-2].m_key = 0; for (next=n-3; next>=0; next--) A[next].m_key = A[A[next].m_key].m_key+1;
  avbl = 1; used = dpth = 0; root = n-2; next = n-1;
  while (avbl>0)
  {
    while (root>=0 && (int)A[root].m_key==dpth) { used++; root--; }
    while (avbl>used) { A[next--].m_key = (mmz_uint16)(dpth); avbl--; }
    avbl = 2*used; dpth++; used = 0;
  }
}

// Limits canonical Huffman code table's max code size.
enum { MMZ_TDEFL_MAX_SUPPORTED_HUFF_CODESIZE = 32 };
static void mmz_tdefl_huffman_enforce_max_code_size(int *pNum_codes, int code_list_len, int max_code_size)
{
  int i; mmz_uint32 total = 0; if (code_list_len <= 1) return;
  for (i = max_code_size + 1; i <= MMZ_TDEFL_MAX_SUPPORTED_HUFF_CODESIZE; i++) pNum_codes[max_code_size] += pNum_codes[i];
  for (i = max_code_size; i > 0; i--) total += (((mmz_uint32)pNum_codes[i]) << (max_code_size - i));
  while (total != (1UL << max_code_size))
  {
    pNum_codes[max_code_size]--;
    for (i = max_code_size - 1; i > 0; i--) if (pNum_codes[i]) { pNum_codes[i]--; pNum_codes[i + 1] += 2; break; }
    total--;
  }
}

static void mmz_tdefl_optimize_huffman_table(mmz_tdefl_compressor *d, int table_num, int table_len, int code_size_limit, int static_table)
{
  int i, j, l, num_codes[1 + MMZ_TDEFL_MAX_SUPPORTED_HUFF_CODESIZE]; mmz_uint next_code[TDEFL_MAX_SUPPORTED_HUFF_CODESIZE + 1]; MMZ_CLEAR_OBJ(num_codes);
  if (static_table)
  {
    for (i = 0; i < table_len; i++) num_codes[d->m_huff_code_sizes[table_num][i]]++;
  }
  else
  {
    mmz_tdefl_sym_freq syms0[TDEFL_MAX_HUFF_SYMBOLS], syms1[TDEFL_MAX_HUFF_SYMBOLS], *pSyms;
    int num_used_syms = 0;
    const mmz_uint16 *pSym_count = &d->m_huff_count[table_num][0];
    for (i = 0; i < table_len; i++) if (pSym_count[i]) { syms0[num_used_syms].m_key = (mmz_uint16)pSym_count[i]; syms0[num_used_syms++].m_sym_index = (mmz_uint16)i; }

    pSyms = mmz_tdefl_radix_sort_syms(num_used_syms, syms0, syms1); mmz_tdefl_calculate_minimum_redundancy(pSyms, num_used_syms);

    for (i = 0; i < num_used_syms; i++) num_codes[pSyms[i].m_key]++;

    mmz_tdefl_huffman_enforce_max_code_size(num_codes, num_used_syms, code_size_limit);

    MMZ_CLEAR_OBJ(d->m_huff_code_sizes[table_num]); MMZ_CLEAR_OBJ(d->m_huff_codes[table_num]);
    for (i = 1, j = num_used_syms; i <= code_size_limit; i++)
      for (l = num_codes[i]; l > 0; l--) d->m_huff_code_sizes[table_num][pSyms[--j].m_sym_index] = (mmz_uint8)(i);
  }

  next_code[1] = 0; for (j = 0, i = 2; i <= code_size_limit; i++) next_code[i] = j = ((j + num_codes[i - 1]) << 1);

  for (i = 0; i < table_len; i++)
  {
    mmz_uint rev_code = 0, code, code_size; if ((code_size = d->m_huff_code_sizes[table_num][i]) == 0) continue;
    code = next_code[code_size]++; for (l = code_size; l > 0; l--, code >>= 1) rev_code = (rev_code << 1) | (code & 1);
    d->m_huff_codes[table_num][i] = (mmz_uint16)rev_code;
  }
}

#define MMZ_TDEFL_PUT_BITS(b, l) do { \
  mmz_uint bits = b; mmz_uint len = l; MMZ_ASSERT(bits <= ((1U << len) - 1U)); \
  d->m_bit_buffer |= (bits << d->m_bits_in); d->m_bits_in += len; \
  while (d->m_bits_in >= 8) { \
    if (d->m_pOutput_buf < d->m_pOutput_buf_end) \
      *d->m_pOutput_buf++ = (mmz_uint8)(d->m_bit_buffer); \
      d->m_bit_buffer >>= 8; \
      d->m_bits_in -= 8; \
  } \
} MMZ_MACRO_END

#define MMZ_TDEFL_RLE_PREV_CODE_SIZE() { if (rle_repeat_count) { \
  if (rle_repeat_count < 3) { \
    d->m_huff_count[2][prev_code_size] = (mmz_uint16)(d->m_huff_count[2][prev_code_size] + rle_repeat_count); \
    while (rle_repeat_count--) packed_code_sizes[num_packed_code_sizes++] = prev_code_size; \
  } else { \
    d->m_huff_count[2][16] = (mmz_uint16)(d->m_huff_count[2][16] + 1); packed_code_sizes[num_packed_code_sizes++] = 16; packed_code_sizes[num_packed_code_sizes++] = (mmz_uint8)(rle_repeat_count - 3); \
} rle_repeat_count = 0; } }

#define MMZ_TDEFL_RLE_ZERO_CODE_SIZE() { if (rle_z_count) { \
  if (rle_z_count < 3) { \
    d->m_huff_count[2][0] = (mmz_uint16)(d->m_huff_count[2][0] + rle_z_count); while (rle_z_count--) packed_code_sizes[num_packed_code_sizes++] = 0; \
  } else if (rle_z_count <= 10) { \
    d->m_huff_count[2][17] = (mmz_uint16)(d->m_huff_count[2][17] + 1); packed_code_sizes[num_packed_code_sizes++] = 17; packed_code_sizes[num_packed_code_sizes++] = (mmz_uint8)(rle_z_count - 3); \
  } else { \
    d->m_huff_count[2][18] = (mmz_uint16)(d->m_huff_count[2][18] + 1); packed_code_sizes[num_packed_code_sizes++] = 18; packed_code_sizes[num_packed_code_sizes++] = (mmz_uint8)(rle_z_count - 11); \
} rle_z_count = 0; } }

static mmz_uint8 mmz_s_tdefl_packed_code_size_syms_swizzle[] = { 16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15 };

static void mmz_tdefl_start_dynamic_block(mmz_tdefl_compressor *d)
{
  int num_lit_codes, num_dist_codes, num_bit_lengths; mmz_uint i, total_code_sizes_to_pack, num_packed_code_sizes, rle_z_count, rle_repeat_count, packed_code_sizes_index;
  mmz_uint8 code_sizes_to_pack[TDEFL_MAX_HUFF_SYMBOLS_0 + MMZ_TDEFL_MAX_HUFF_SYMBOLS_1], packed_code_sizes[TDEFL_MAX_HUFF_SYMBOLS_0 + MMZ_TDEFL_MAX_HUFF_SYMBOLS_1], prev_code_size = 0xFF;

  d->m_huff_count[0][256] = 1;

  mmz_tdefl_optimize_huffman_table(d, 0, MMZ_TDEFL_MAX_HUFF_SYMBOLS_0, 15, MMZ_FALSE);
  mmz_tdefl_optimize_huffman_table(d, 1, MMZ_TDEFL_MAX_HUFF_SYMBOLS_1, 15, MMZ_FALSE);

  for (num_lit_codes = 286; num_lit_codes > 257; num_lit_codes--) if (d->m_huff_code_sizes[0][num_lit_codes - 1]) break;
  for (num_dist_codes = 30; num_dist_codes > 1; num_dist_codes--) if (d->m_huff_code_sizes[1][num_dist_codes - 1]) break;

  memcpy(code_sizes_to_pack, &d->m_huff_code_sizes[0][0], num_lit_codes);
  memcpy(code_sizes_to_pack + num_lit_codes, &d->m_huff_code_sizes[1][0], num_dist_codes);
  total_code_sizes_to_pack = num_lit_codes + num_dist_codes; num_packed_code_sizes = 0; rle_z_count = 0; rle_repeat_count = 0;

  memset(&d->m_huff_count[2][0], 0, sizeof(d->m_huff_count[2][0]) * MMZ_TDEFL_MAX_HUFF_SYMBOLS_2);
  for (i = 0; i < total_code_sizes_to_pack; i++)
  {
    mmz_uint8 code_size = code_sizes_to_pack[i];
    if (!code_size)
    {
      MMZ_TDEFL_RLE_PREV_CODE_SIZE();
      if (++rle_z_count == 138) { MMZ_TDEFL_RLE_ZERO_CODE_SIZE(); }
    }
    else
    {
      MMZ_TDEFL_RLE_ZERO_CODE_SIZE();
      if (code_size != prev_code_size)
      {
        MMZ_TDEFL_RLE_PREV_CODE_SIZE();
        d->m_huff_count[2][code_size] = (mmz_uint16)(d->m_huff_count[2][code_size] + 1); packed_code_sizes[num_packed_code_sizes++] = code_size;
      }
      else if (++rle_repeat_count == 6)
      {
        MMZ_TDEFL_RLE_PREV_CODE_SIZE();
      }
    }
    prev_code_size = code_size;
  }
  if (rle_repeat_count) { MMZ_TDEFL_RLE_PREV_CODE_SIZE(); } else { MMZ_TDEFL_RLE_ZERO_CODE_SIZE(); }

  mmz_tdefl_optimize_huffman_table(d, 2, MMZ_TDEFL_MAX_HUFF_SYMBOLS_2, 7, MMZ_FALSE);

  MMZ_TDEFL_PUT_BITS(2, 2);

  MMZ_TDEFL_PUT_BITS(num_lit_codes - 257, 5);
  MMZ_TDEFL_PUT_BITS(num_dist_codes - 1, 5);

  for (num_bit_lengths = 18; num_bit_lengths >= 0; num_bit_lengths--) if (d->m_huff_code_sizes[2][s_tdefl_packed_code_size_syms_swizzle[num_bit_lengths]]) break;
  num_bit_lengths = MMZ_MAX(4, (num_bit_lengths + 1)); MMZ_TDEFL_PUT_BITS(num_bit_lengths - 4, 4);
  for (i = 0; (int)i < num_bit_lengths; i++) MMZ_TDEFL_PUT_BITS(d->m_huff_code_sizes[2][s_tdefl_packed_code_size_syms_swizzle[i]], 3);

  for (packed_code_sizes_index = 0; packed_code_sizes_index < num_packed_code_sizes; )
  {
    mmz_uint code = packed_code_sizes[packed_code_sizes_index++]; MMZ_ASSERT(code < MMZ_TDEFL_MAX_HUFF_SYMBOLS_2);
    MMZ_TDEFL_PUT_BITS(d->m_huff_codes[2][code], d->m_huff_code_sizes[2][code]);
    if (code >= 16) MMZ_TDEFL_PUT_BITS(packed_code_sizes[packed_code_sizes_index++], "\02\03\07"[code - 16]);
  }
}

static void mmz_tdefl_start_static_block(mmz_tdefl_compressor *d)
{
  mmz_uint i;
  mmz_uint8 *p = &d->m_huff_code_sizes[0][0];

  for (i = 0; i <= 143; ++i) *p++ = 8;
  for ( ; i <= 255; ++i) *p++ = 9;
  for ( ; i <= 279; ++i) *p++ = 7;
  for ( ; i <= 287; ++i) *p++ = 8;

  memset(d->m_huff_code_sizes[1], 5, 32);

  mmz_tdefl_optimize_huffman_table(d, 0, 288, 15, MMZ_TRUE);
  mmz_tdefl_optimize_huffman_table(d, 1, 32, 15, MMZ_TRUE);

  MMZ_TDEFL_PUT_BITS(1, 2);
}

static const mmz_uint mmz_bitmasks[17] = { 0x0000, 0x0001, 0x0003, 0x0007, 0x000F, 0x001F, 0x003F, 0x007F, 0x00FF, 0x01FF, 0x03FF, 0x07FF, 0x0FFF, 0x1FFF, 0x3FFF, 0x7FFF, 0xFFFF };

static mmz_bool mmz_tdefl_compress_lz_codes(mmz_tdefl_compressor *d)
{
  mmz_uint flags;
  mmz_uint8 *pLZ_codes;
  mmz_uint8 *pOutput_buf = d->m_pOutput_buf;
  mmz_uint8 *pLZ_code_buf_end = d->m_pLZ_code_buf;
  mmz_uint64 bit_buffer = d->m_bit_buffer;
  mmz_uint bits_in = d->m_bits_in;

#define MMZ_TDEFL_PUT_BITS_FAST(b, l) { bit_buffer |= (((mmz_uint64)(b)) << bits_in); bits_in += (l); }

  flags = 1;
  for (pLZ_codes = d->m_lz_code_buf; pLZ_codes < pLZ_code_buf_end; flags >>= 1)
  {
    if (flags == 1)
      flags = *pLZ_codes++ | 0x100;

    if (flags & 1)
    {
      mmz_uint s0, s1, n0, n1, sym, num_extra_bits;
      mmz_uint match_len = pLZ_codes[0], match_dist = *(const mmz_uint16 *)(pLZ_codes + 1); pLZ_codes += 3;

      MMZ_ASSERT(d->m_huff_code_sizes[0][s_tdefl_len_sym[match_len]]);
      MMZ_TDEFL_PUT_BITS_FAST(d->m_huff_codes[0][s_tdefl_len_sym[match_len]], d->m_huff_code_sizes[0][s_tdefl_len_sym[match_len]]);
      MMZ_TDEFL_PUT_BITS_FAST(match_len & mmz_bitmasks[s_tdefl_len_extra[match_len]], mmz_s_tdefl_len_extra[match_len]);

      // This sequence coaxes MSVC into using cmov's vs. jmp's.
      s0 = mmz_s_tdefl_small_dist_sym[match_dist & 511];
      n0 = mmz_s_tdefl_small_dist_extra[match_dist & 511];
      s1 = mmz_s_tdefl_large_dist_sym[match_dist >> 8];
      n1 = mmz_s_tdefl_large_dist_extra[match_dist >> 8];
      sym = (match_dist < 512) ? s0 : s1;
      num_extra_bits = (match_dist < 512) ? n0 : n1;

      MMZ_ASSERT(d->m_huff_code_sizes[1][sym]);
      MMZ_TDEFL_PUT_BITS_FAST(d->m_huff_codes[1][sym], d->m_huff_code_sizes[1][sym]);
      MMZ_TDEFL_PUT_BITS_FAST(match_dist & mmz_bitmasks[num_extra_bits], num_extra_bits);
    }
    else
    {
      mmz_uint lit = *pLZ_codes++;
      MMZ_ASSERT(d->m_huff_code_sizes[0][lit]);
      MMZ_TDEFL_PUT_BITS_FAST(d->m_huff_codes[0][lit], d->m_huff_code_sizes[0][lit]);

      if (((flags & 2) == 0) && (pLZ_codes < pLZ_code_buf_end))
      {
        flags >>= 1;
        lit = *pLZ_codes++;
        MMZ_ASSERT(d->m_huff_code_sizes[0][lit]);
        MMZ_TDEFL_PUT_BITS_FAST(d->m_huff_codes[0][lit], d->m_huff_code_sizes[0][lit]);

        if (((flags & 2) == 0) && (pLZ_codes < pLZ_code_buf_end))
        {
          flags >>= 1;
          lit = *pLZ_codes++;
          MMZ_ASSERT(d->m_huff_code_sizes[0][lit]);
          MMZ_TDEFL_PUT_BITS_FAST(d->m_huff_codes[0][lit], d->m_huff_code_sizes[0][lit]);
        }
      }
    }

    if (pOutput_buf >= d->m_pOutput_buf_end)
      return MMZ_FALSE;

    *(mmz_uint64*)pOutput_buf = bit_buffer;
    pOutput_buf += (bits_in >> 3);
    bit_buffer >>= (bits_in & ~7);
    bits_in &= 7;
  }

#undef MMZ_TDEFL_PUT_BITS_FAST

  d->m_pOutput_buf = pOutput_buf;
  d->m_bits_in = 0;
  d->m_bit_buffer = 0;

  while (bits_in)
  {
    mmz_uint32 n = MMZ_MIN(bits_in, 16);
    MMZ_TDEFL_PUT_BITS((mmz_uint)bit_buffer & mmz_bitmasks[n], n);
    bit_buffer >>= n;
    bits_in -= n;
  }

  MMZ_TDEFL_PUT_BITS(d->m_huff_codes[0][256], d->m_huff_code_sizes[0][256]);

  return (d->m_pOutput_buf < d->m_pOutput_buf_end);
}


static mmz_bool mmz_tdefl_compress_block(mmz_tdefl_compressor *d, mmz_bool static_block)
{
  if (static_block)
    mmz_tdefl_start_static_block(d);
  else
    mmz_tdefl_start_dynamic_block(d);
  return mmz_tdefl_compress_lz_codes(d);
}

static int mmz_tdefl_flush_block(mmz_tdefl_compressor *d, int flush)
{
  mmz_uint saved_bit_buf, saved_bits_in;
  mmz_uint8 *pSaved_output_buf;
  mmz_bool comp_block_succeeded = MMZ_FALSE;
  int n, use_raw_block = ((d->m_flags & MMZ_TDEFL_FORCE_ALL_RAW_BLOCKS) != 0) && (d->m_lookahead_pos - d->m_lz_code_buf_dict_pos) <= d->m_dict_size;
  mmz_uint8 *pOutput_buf_start = ((d->m_pPut_buf_func == NULL) && ((*d->m_pOut_buf_size - d->m_out_buf_ofs) >= MMZ_TDEFL_OUT_BUF_SIZE)) ? ((mmz_uint8 *)d->m_pOut_buf + d->m_out_buf_ofs) : d->m_output_buf;

  d->m_pOutput_buf = pOutput_buf_start;
  d->m_pOutput_buf_end = d->m_pOutput_buf + MMZ_TDEFL_OUT_BUF_SIZE - 16;

  MMZ_ASSERT(!d->m_output_flush_remaining);
  d->m_output_flush_ofs = 0;
  d->m_output_flush_remaining = 0;

  *d->m_pLZ_flags = (mmz_uint8)(*d->m_pLZ_flags >> d->m_num_flags_left);
  d->m_pLZ_code_buf -= (d->m_num_flags_left == 8);

  if ((d->m_flags & MMZ_TDEFL_WRITE_ZLIB_HEADER) && (!d->m_block_index))
  {
    MMZ_TDEFL_PUT_BITS(0x78, 8); MMZ_TDEFL_PUT_BITS(0x01, 8);
  }

  MMZ_TDEFL_PUT_BITS(flush == MMZ_TDEFL_FINISH, 1);

  pSaved_output_buf = d->m_pOutput_buf; saved_bit_buf = d->m_bit_buffer; saved_bits_in = d->m_bits_in;

  if (!use_raw_block)
    comp_block_succeeded = mmz_tdefl_compress_block(d, (d->m_flags & MMZ_TDEFL_FORCE_ALL_STATIC_BLOCKS) || (d->m_total_lz_bytes < 48));

  // If the block gets expanded, forget the current contents of the output buffer and send a raw block instead.
  if ( ((use_raw_block) || ((d->m_total_lz_bytes) && ((d->m_pOutput_buf - pSaved_output_buf + 1U) >= d->m_total_lz_bytes))) &&
       ((d->m_lookahead_pos - d->m_lz_code_buf_dict_pos) <= d->m_dict_size) )
  {
    mmz_uint i; d->m_pOutput_buf = pSaved_output_buf; d->m_bit_buffer = saved_bit_buf, d->m_bits_in = saved_bits_in;
    MMZ_TDEFL_PUT_BITS(0, 2);
    if (d->m_bits_in) { MMZ_TDEFL_PUT_BITS(0, 8 - d->m_bits_in); }
    for (i = 2; i; --i, d->m_total_lz_bytes ^= 0xFFFF)
    {
      MMZ_TDEFL_PUT_BITS(d->m_total_lz_bytes & 0xFFFF, 16);
    }
    for (i = 0; i < d->m_total_lz_bytes; ++i)
    {
      MMZ_TDEFL_PUT_BITS(d->m_dict[(d->m_lz_code_buf_dict_pos + i) & MMZ_TDEFL_LZ_DICT_SIZE_MASK], 8);
    }
  }
  // Check for the extremely unlikely (if not impossible) case of the compressed block not fitting into the output buffer when using dynamic codes.
  else if (!comp_block_succeeded)
  {
    d->m_pOutput_buf = pSaved_output_buf; d->m_bit_buffer = saved_bit_buf, d->m_bits_in = saved_bits_in;
    mmz_tdefl_compress_block(d, MMZ_TRUE);
  }

  if (flush)
  {
    if (flush == MMZ_TDEFL_FINISH)
    {
      if (d->m_bits_in) { MMZ_TDEFL_PUT_BITS(0, 8 - d->m_bits_in); }
      if (d->m_flags & MMZ_TDEFL_WRITE_ZLIB_HEADER) { mmz_uint i, a = d->m_adler32; for (i = 0; i < 4; i++) { MMZ_TDEFL_PUT_BITS((a >> 24) & 0xFF, 8); a <<= 8; } }
    }
    else
    {
      mmz_uint i, z = 0; MMZ_TDEFL_PUT_BITS(0, 3); if (d->m_bits_in) { MMZ_TDEFL_PUT_BITS(0, 8 - d->m_bits_in); } for (i = 2; i; --i, z ^= 0xFFFF) { MMZ_TDEFL_PUT_BITS(z & 0xFFFF, 16); }
    }
  }

  MMZ_ASSERT(d->m_pOutput_buf < d->m_pOutput_buf_end);

  memset(&d->m_huff_count[0][0], 0, sizeof(d->m_huff_count[0][0]) * MMZ_TDEFL_MAX_HUFF_SYMBOLS_0);
  memset(&d->m_huff_count[1][0], 0, sizeof(d->m_huff_count[1][0]) * MMZ_TDEFL_MAX_HUFF_SYMBOLS_1);

  d->m_pLZ_code_buf = d->m_lz_code_buf + 1; d->m_pLZ_flags = d->m_lz_code_buf; d->m_num_flags_left = 8; d->m_lz_code_buf_dict_pos += d->m_total_lz_bytes; d->m_total_lz_bytes = 0; d->m_block_index++;

  if ((n = (int)(d->m_pOutput_buf - pOutput_buf_start)) != 0)
  {
    if (d->m_pPut_buf_func)
    {
      *d->m_pIn_buf_size = d->m_pSrc - (const mmz_uint8 *)d->m_pIn_buf;
      if (!(*d->m_pPut_buf_func)(d->m_output_buf, n, d->m_pPut_buf_user))
        return (d->m_prev_return_status = MMZ_TDEFL_STATUS_PUT_BUF_FAILED);
    }
    else if (pOutput_buf_start == d->m_output_buf)
    {
      int bytes_to_copy = (int)MMZ_MIN((size_t)n, (size_t)(*d->m_pOut_buf_size - d->m_out_buf_ofs));
      memcpy((mmz_uint8 *)d->m_pOut_buf + d->m_out_buf_ofs, d->m_output_buf, bytes_to_copy);
      d->m_out_buf_ofs += bytes_to_copy;
      if ((n -= bytes_to_copy) != 0)
      {
        d->m_output_flush_ofs = bytes_to_copy;
        d->m_output_flush_remaining = n;
      }
    }
    else
    {
      d->m_out_buf_ofs += n;
    }
  }

  return d->m_output_flush_remaining;
}


#define MMZ_TDEFL_READ_UNALIGNED_WORD(p) *(const mmz_uint16*)(p)
static MMZ_FORCEINLINE void mmz_tdefl_find_match(mmz_tdefl_compressor *d, mmz_uint lookahead_pos, mmz_uint max_dist, mmz_uint max_match_len, mmz_uint *pMatch_dist, mmz_uint *pMatch_len)
{
  mmz_uint dist, pos = lookahead_pos & MMZ_TDEFL_LZ_DICT_SIZE_MASK, match_len = *pMatch_len, probe_pos = pos, next_probe_pos, probe_len;
  mmz_uint num_probes_left = d->m_max_probes[match_len >= 32];
  const mmz_uint16 *s = (const mmz_uint16*)(d->m_dict + pos), *p, *q;
  mmz_uint16 c01 = MMZ_TDEFL_READ_UNALIGNED_WORD(&d->m_dict[pos + match_len - 1]), s01 = MMZ_TDEFL_READ_UNALIGNED_WORD(s);
  MMZ_ASSERT(max_match_len <= MMZ_TDEFL_MAX_MATCH_LEN); if (max_match_len <= match_len) return;
  for ( ; ; )
  {
    for ( ; ; )
    {
      if (--num_probes_left == 0) return;
      #define MMZ_TDEFL_PROBE \
        next_probe_pos = d->m_next[probe_pos]; \
        if ((!next_probe_pos) || ((dist = (mmz_uint16)(lookahead_pos - next_probe_pos)) > max_dist)) return; \
        probe_pos = next_probe_pos & MMZ_TDEFL_LZ_DICT_SIZE_MASK; \
        if (MMZ_TDEFL_READ_UNALIGNED_WORD(&d->m_dict[probe_pos + match_len - 1]) == c01) break;
      MMZ_TDEFL_PROBE; MMZ_TDEFL_PROBE; MMZ_TDEFL_PROBE;
    }
    if (!dist) break; q = (const mmz_uint16*)(d->m_dict + probe_pos); if (MMZ_TDEFL_READ_UNALIGNED_WORD(q) != s01) continue; p = s; probe_len = 32;
    do { } while ( (MMZ_TDEFL_READ_UNALIGNED_WORD(++p) == MMZ_TDEFL_READ_UNALIGNED_WORD(++q)) && (MMZ_TDEFL_READ_UNALIGNED_WORD(++p) == MMZ_TDEFL_READ_UNALIGNED_WORD(++q)) &&
                   (MMZ_TDEFL_READ_UNALIGNED_WORD(++p) == MMZ_TDEFL_READ_UNALIGNED_WORD(++q)) && (MMZ_TDEFL_READ_UNALIGNED_WORD(++p) == MMZ_TDEFL_READ_UNALIGNED_WORD(++q)) && (--probe_len > 0) );
    if (!probe_len)
    {
      *pMatch_dist = dist; *pMatch_len = MMZ_MIN(max_match_len, MMZ_TDEFL_MAX_MATCH_LEN); break;
    }
    else if ((probe_len = ((mmz_uint)(p - s) * 2) + (mmz_uint)(*(const mmz_uint8*)p == *(const mmz_uint8*)q)) > match_len)
    {
      *pMatch_dist = dist; if ((*pMatch_len = match_len = MMZ_MIN(max_match_len, probe_len)) == max_match_len) break;
      c01 = MMZ_TDEFL_READ_UNALIGNED_WORD(&d->m_dict[pos + match_len - 1]);
    }
  }
}


static mmz_bool mmz_tdefl_compress_fast(mmz_tdefl_compressor *d)
{
  // Faster, minimally featured LZRW1-style match+parse loop with better register utilization. Intended for applications where raw throughput is valued more highly than ratio.
  mmz_uint lookahead_pos = d->m_lookahead_pos, lookahead_size = d->m_lookahead_size, dict_size = d->m_dict_size, total_lz_bytes = d->m_total_lz_bytes, num_flags_left = d->m_num_flags_left;
  mmz_uint8 *pLZ_code_buf = d->m_pLZ_code_buf, *pLZ_flags = d->m_pLZ_flags;
  mmz_uint cur_pos = lookahead_pos & MMZ_TDEFL_LZ_DICT_SIZE_MASK;

  while ((d->m_src_buf_left) || ((d->m_flush) && (lookahead_size)))
  {
    const mmz_uint MMZ_TDEFL_COMP_FAST_LOOKAHEAD_SIZE = 4096;
    mmz_uint dst_pos = (lookahead_pos + lookahead_size) & MMZ_TDEFL_LZ_DICT_SIZE_MASK;
    mmz_uint num_bytes_to_process = (mmz_uint)MMZ_MIN(d->m_src_buf_left, MMZ_TDEFL_COMP_FAST_LOOKAHEAD_SIZE - lookahead_size);
    d->m_src_buf_left -= num_bytes_to_process;
    lookahead_size += num_bytes_to_process;

    while (num_bytes_to_process)
    {
      mmz_uint32 n = MMZ_MIN(MMZ_TDEFL_LZ_DICT_SIZE - dst_pos, num_bytes_to_process);
      memcpy(d->m_dict + dst_pos, d->m_pSrc, n);
      if (dst_pos < (MMZ_TDEFL_MAX_MATCH_LEN - 1))
        memcpy(d->m_dict + MMZ_TDEFL_LZ_DICT_SIZE + dst_pos, d->m_pSrc, MMZ_MIN(n, (MMZ_TDEFL_MAX_MATCH_LEN - 1) - dst_pos));
      d->m_pSrc += n;
      dst_pos = (dst_pos + n) & MMZ_TDEFL_LZ_DICT_SIZE_MASK;
      num_bytes_to_process -= n;
    }

    dict_size = MMZ_MIN(MMZ_TDEFL_LZ_DICT_SIZE - lookahead_size, dict_size);
    if ((!d->m_flush) && (lookahead_size < MMZ_TDEFL_COMP_FAST_LOOKAHEAD_SIZE)) break;

    while (lookahead_size >= 4)
    {
      mmz_uint cur_match_dist, cur_match_len = 1;
      mmz_uint8 *pCur_dict = d->m_dict + cur_pos;
      mmz_uint first_trigram = (*(const mmz_uint32 *)pCur_dict) & 0xFFFFFF;
      mmz_uint hash = (first_trigram ^ (first_trigram >> (24 - (MMZ_TDEFL_LZ_HASH_BITS - 8)))) & MMZ_TDEFL_LEVEL1_HASH_SIZE_MASK;
      mmz_uint probe_pos = d->m_hash[hash];
      d->m_hash[hash] = (mmz_uint16)lookahead_pos;

      if (((cur_match_dist = (mmz_uint16)(lookahead_pos - probe_pos)) <= dict_size) && ((*(const mmz_uint32 *)(d->m_dict + (probe_pos &= MMZ_TDEFL_LZ_DICT_SIZE_MASK)) & 0xFFFFFF) == first_trigram))
      {
        const mmz_uint16 *p = (const mmz_uint16 *)pCur_dict;
        const mmz_uint16 *q = (const mmz_uint16 *)(d->m_dict + probe_pos);
        mmz_uint32 probe_len = 32;
        do { } while ( (MMZ_TDEFL_READ_UNALIGNED_WORD(++p) == MMZ_TDEFL_READ_UNALIGNED_WORD(++q)) && (MMZ_TDEFL_READ_UNALIGNED_WORD(++p) == MMZ_TDEFL_READ_UNALIGNED_WORD(++q)) &&
          (MMZ_TDEFL_READ_UNALIGNED_WORD(++p) == MMZ_TDEFL_READ_UNALIGNED_WORD(++q)) && (MMZ_TDEFL_READ_UNALIGNED_WORD(++p) == MMZ_TDEFL_READ_UNALIGNED_WORD(++q)) && (--probe_len > 0) );
        cur_match_len = ((mmz_uint)(p - (const mmz_uint16 *)pCur_dict) * 2) + (mmz_uint)(*(const mmz_uint8 *)p == *(const mmz_uint8 *)q);
        if (!probe_len)
          cur_match_len = cur_match_dist ? MMZ_TDEFL_MAX_MATCH_LEN : 0;

        if ((cur_match_len < MMZ_TDEFL_MIN_MATCH_LEN) || ((cur_match_len == MMZ_TDEFL_MIN_MATCH_LEN) && (cur_match_dist >= 8U*1024U)))
        {
          cur_match_len = 1;
          *pLZ_code_buf++ = (mmz_uint8)first_trigram;
          *pLZ_flags = (mmz_uint8)(*pLZ_flags >> 1);
          d->m_huff_count[0][(mmz_uint8)first_trigram]++;
        }
        else
        {
          mmz_uint32 s0, s1;
          cur_match_len = MMZ_MIN(cur_match_len, lookahead_size);

          MMZ_ASSERT((cur_match_len >= MMZ_TDEFL_MIN_MATCH_LEN) && (cur_match_dist >= 1) && (cur_match_dist <= MMZ_TDEFL_LZ_DICT_SIZE));

          cur_match_dist--;

          pLZ_code_buf[0] = (mmz_uint8)(cur_match_len - MMZ_TDEFL_MIN_MATCH_LEN);
          *(mmz_uint16 *)(&pLZ_code_buf[1]) = (mmz_uint16)cur_match_dist;
          pLZ_code_buf += 3;
          *pLZ_flags = (mmz_uint8)((*pLZ_flags >> 1) | 0x80);

          s0 = mmz_s_tdefl_small_dist_sym[cur_match_dist & 511];
          s1 = mmz_s_tdefl_large_dist_sym[cur_match_dist >> 8];
          d->m_huff_count[1][(cur_match_dist < 512) ? s0 : s1]++;

          d->m_huff_count[0][s_tdefl_len_sym[cur_match_len - MMZ_TDEFL_MIN_MATCH_LEN]]++;
        }
      }
      else
      {
        *pLZ_code_buf++ = (mmz_uint8)first_trigram;
        *pLZ_flags = (mmz_uint8)(*pLZ_flags >> 1);
        d->m_huff_count[0][(mmz_uint8)first_trigram]++;
      }

      if (--num_flags_left == 0) { num_flags_left = 8; pLZ_flags = pLZ_code_buf++; }

      total_lz_bytes += cur_match_len;
      lookahead_pos += cur_match_len;
      dict_size = MMZ_MIN(dict_size + cur_match_len, MMZ_TDEFL_LZ_DICT_SIZE);
      cur_pos = (cur_pos + cur_match_len) & MMZ_TDEFL_LZ_DICT_SIZE_MASK;
      MMZ_ASSERT(lookahead_size >= cur_match_len);
      lookahead_size -= cur_match_len;

      if (pLZ_code_buf > &d->m_lz_code_buf[TDEFL_LZ_CODE_BUF_SIZE - 8])
      {
        int n;
        d->m_lookahead_pos = lookahead_pos; d->m_lookahead_size = lookahead_size; d->m_dict_size = dict_size;
        d->m_total_lz_bytes = total_lz_bytes; d->m_pLZ_code_buf = pLZ_code_buf; d->m_pLZ_flags = pLZ_flags; d->m_num_flags_left = num_flags_left;
        if ((n = mmz_tdefl_flush_block(d, 0)) != 0)
          return (n < 0) ? MMZ_FALSE : MMZ_TRUE;
        total_lz_bytes = d->m_total_lz_bytes; pLZ_code_buf = d->m_pLZ_code_buf; pLZ_flags = d->m_pLZ_flags; num_flags_left = d->m_num_flags_left;
      }
    }

    while (lookahead_size)
    {
      mmz_uint8 lit = d->m_dict[cur_pos];

      total_lz_bytes++;
      *pLZ_code_buf++ = lit;
      *pLZ_flags = (mmz_uint8)(*pLZ_flags >> 1);
      if (--num_flags_left == 0) { num_flags_left = 8; pLZ_flags = pLZ_code_buf++; }

      d->m_huff_count[0][lit]++;

      lookahead_pos++;
      dict_size = MMZ_MIN(dict_size + 1, MMZ_TDEFL_LZ_DICT_SIZE);
      cur_pos = (cur_pos + 1) & MMZ_TDEFL_LZ_DICT_SIZE_MASK;
      lookahead_size--;

      if (pLZ_code_buf > &d->m_lz_code_buf[TDEFL_LZ_CODE_BUF_SIZE - 8])
      {
        int n;
        d->m_lookahead_pos = lookahead_pos; d->m_lookahead_size = lookahead_size; d->m_dict_size = dict_size;
        d->m_total_lz_bytes = total_lz_bytes; d->m_pLZ_code_buf = pLZ_code_buf; d->m_pLZ_flags = pLZ_flags; d->m_num_flags_left = num_flags_left;
        if ((n = mmz_tdefl_flush_block(d, 0)) != 0)
          return (n < 0) ? MMZ_FALSE : MMZ_TRUE;
        total_lz_bytes = d->m_total_lz_bytes; pLZ_code_buf = d->m_pLZ_code_buf; pLZ_flags = d->m_pLZ_flags; num_flags_left = d->m_num_flags_left;
      }
    }
  }

  d->m_lookahead_pos = lookahead_pos; d->m_lookahead_size = lookahead_size; d->m_dict_size = dict_size;
  d->m_total_lz_bytes = total_lz_bytes; d->m_pLZ_code_buf = pLZ_code_buf; d->m_pLZ_flags = pLZ_flags; d->m_num_flags_left = num_flags_left;
  return MMZ_TRUE;
}

static MMZ_FORCEINLINE void mmz_tdefl_record_literal(mmz_tdefl_compressor *d, mmz_uint8 lit)
{
  d->m_total_lz_bytes++;
  *d->m_pLZ_code_buf++ = lit;
  *d->m_pLZ_flags = (mmz_uint8)(*d->m_pLZ_flags >> 1); if (--d->m_num_flags_left == 0) { d->m_num_flags_left = 8; d->m_pLZ_flags = d->m_pLZ_code_buf++; }
  d->m_huff_count[0][lit]++;
}

static MMZ_FORCEINLINE void mmz_tdefl_record_match(mmz_tdefl_compressor *d, mmz_uint match_len, mmz_uint match_dist)
{
  mmz_uint32 s0, s1;

  MMZ_ASSERT((match_len >= MMZ_TDEFL_MIN_MATCH_LEN) && (match_dist >= 1) && (match_dist <= MMZ_TDEFL_LZ_DICT_SIZE));

  d->m_total_lz_bytes += match_len;

  d->m_pLZ_code_buf[0] = (mmz_uint8)(match_len - MMZ_TDEFL_MIN_MATCH_LEN);

  match_dist -= 1;
  d->m_pLZ_code_buf[1] = (mmz_uint8)(match_dist & 0xFF);
  d->m_pLZ_code_buf[2] = (mmz_uint8)(match_dist >> 8); d->m_pLZ_code_buf += 3;

  *d->m_pLZ_flags = (mmz_uint8)((*d->m_pLZ_flags >> 1) | 0x80); if (--d->m_num_flags_left == 0) { d->m_num_flags_left = 8; d->m_pLZ_flags = d->m_pLZ_code_buf++; }

  s0 = mmz_s_tdefl_small_dist_sym[match_dist & 511]; s1 = mmz_s_tdefl_large_dist_sym[(match_dist >> 8) & 127];
  d->m_huff_count[1][(match_dist < 512) ? s0 : s1]++;

  if (match_len >= MMZ_TDEFL_MIN_MATCH_LEN) d->m_huff_count[0][s_tdefl_len_sym[match_len - MMZ_TDEFL_MIN_MATCH_LEN]]++;
}

static mmz_bool mmz_tdefl_compress_normal(mmz_tdefl_compressor *d)
{
  const mmz_uint8 *pSrc = d->m_pSrc; size_t src_buf_left = d->m_src_buf_left;
  mmz_tdefl_flush flush = d->m_flush;

  while ((src_buf_left) || ((flush) && (d->m_lookahead_size)))
  {
    mmz_uint len_to_move, cur_match_dist, cur_match_len, cur_pos;
    // Update dictionary and hash chains. Keeps the lookahead size equal to MMZ_TDEFL_MAX_MATCH_LEN.
    if ((d->m_lookahead_size + d->m_dict_size) >= (MMZ_TDEFL_MIN_MATCH_LEN - 1))
    {
      mmz_uint dst_pos = (d->m_lookahead_pos + d->m_lookahead_size) & MMZ_TDEFL_LZ_DICT_SIZE_MASK, ins_pos = d->m_lookahead_pos + d->m_lookahead_size - 2;
      mmz_uint hash = (d->m_dict[ins_pos & MMZ_TDEFL_LZ_DICT_SIZE_MASK] << MMZ_TDEFL_LZ_HASH_SHIFT) ^ d->m_dict[(ins_pos + 1) & MMZ_TDEFL_LZ_DICT_SIZE_MASK];
      mmz_uint num_bytes_to_process = (mmz_uint)MMZ_MIN(src_buf_left, MMZ_TDEFL_MAX_MATCH_LEN - d->m_lookahead_size);
      const mmz_uint8 *pSrc_end = pSrc + num_bytes_to_process;
      src_buf_left -= num_bytes_to_process;
      d->m_lookahead_size += num_bytes_to_process;
      while (pSrc != pSrc_end)
      {
        mmz_uint8 c = *pSrc++; d->m_dict[dst_pos] = c; if (dst_pos < (MMZ_TDEFL_MAX_MATCH_LEN - 1)) d->m_dict[TDEFL_LZ_DICT_SIZE + dst_pos] = c;
        hash = ((hash << MMZ_TDEFL_LZ_HASH_SHIFT) ^ c) & (MMZ_TDEFL_LZ_HASH_SIZE - 1);
        d->m_next[ins_pos & MMZ_TDEFL_LZ_DICT_SIZE_MASK] = d->m_hash[hash]; d->m_hash[hash] = (mmz_uint16)(ins_pos);
        dst_pos = (dst_pos + 1) & MMZ_TDEFL_LZ_DICT_SIZE_MASK; ins_pos++;
      }
    }
    else
    {
      while ((src_buf_left) && (d->m_lookahead_size < MMZ_TDEFL_MAX_MATCH_LEN))
      {
        mmz_uint8 c = *pSrc++;
        mmz_uint dst_pos = (d->m_lookahead_pos + d->m_lookahead_size) & MMZ_TDEFL_LZ_DICT_SIZE_MASK;
        src_buf_left--;
        d->m_dict[dst_pos] = c;
        if (dst_pos < (MMZ_TDEFL_MAX_MATCH_LEN - 1))
          d->m_dict[TDEFL_LZ_DICT_SIZE + dst_pos] = c;
        if ((++d->m_lookahead_size + d->m_dict_size) >= MMZ_TDEFL_MIN_MATCH_LEN)
        {
          mmz_uint ins_pos = d->m_lookahead_pos + (d->m_lookahead_size - 1) - 2;
          mmz_uint hash = ((d->m_dict[ins_pos & MMZ_TDEFL_LZ_DICT_SIZE_MASK] << (MMZ_TDEFL_LZ_HASH_SHIFT * 2)) ^ (d->m_dict[(ins_pos + 1) & MMZ_TDEFL_LZ_DICT_SIZE_MASK] << MMZ_TDEFL_LZ_HASH_SHIFT) ^ c) & (MMZ_TDEFL_LZ_HASH_SIZE - 1);
          d->m_next[ins_pos & MMZ_TDEFL_LZ_DICT_SIZE_MASK] = d->m_hash[hash]; d->m_hash[hash] = (mmz_uint16)(ins_pos);
        }
      }
    }
    d->m_dict_size = MMZ_MIN(MMZ_TDEFL_LZ_DICT_SIZE - d->m_lookahead_size, d->m_dict_size);
    if ((!flush) && (d->m_lookahead_size < MMZ_TDEFL_MAX_MATCH_LEN))
      break;

    // Simple lazy/greedy parsing state machine.
    len_to_move = 1; cur_match_dist = 0; cur_match_len = d->m_saved_match_len ? d->m_saved_match_len : (MMZ_TDEFL_MIN_MATCH_LEN - 1); cur_pos = d->m_lookahead_pos & MMZ_TDEFL_LZ_DICT_SIZE_MASK;
    if (d->m_flags & (MMZ_TDEFL_RLE_MATCHES | MMZ_TDEFL_FORCE_ALL_RAW_BLOCKS))
    {
      if ((d->m_dict_size) && (!(d->m_flags & MMZ_TDEFL_FORCE_ALL_RAW_BLOCKS)))
      {
        mmz_uint8 c = d->m_dict[(cur_pos - 1) & MMZ_TDEFL_LZ_DICT_SIZE_MASK];
        cur_match_len = 0; while (cur_match_len < d->m_lookahead_size) { if (d->m_dict[cur_pos + cur_match_len] != c) break; cur_match_len++; }
        if (cur_match_len < MMZ_TDEFL_MIN_MATCH_LEN) cur_match_len = 0; else cur_match_dist = 1;
      }
    }
    else
    {
      mmz_tdefl_find_match(d, d->m_lookahead_pos, d->m_dict_size, d->m_lookahead_size, &cur_match_dist, &cur_match_len);
    }
    if (((cur_match_len == MMZ_TDEFL_MIN_MATCH_LEN) && (cur_match_dist >= 8U*1024U)) || (cur_pos == cur_match_dist) || ((d->m_flags & MMZ_TDEFL_FILTER_MATCHES) && (cur_match_len <= 5)))
    {
      cur_match_dist = cur_match_len = 0;
    }
    if (d->m_saved_match_len)
    {
      if (cur_match_len > d->m_saved_match_len)
      {
        mmz_tdefl_record_literal(d, (mmz_uint8)d->m_saved_lit);
        if (cur_match_len >= 128)
        {
          mmz_tdefl_record_match(d, cur_match_len, cur_match_dist);
          d->m_saved_match_len = 0; len_to_move = cur_match_len;
        }
        else
        {
          d->m_saved_lit = d->m_dict[cur_pos]; d->m_saved_match_dist = cur_match_dist; d->m_saved_match_len = cur_match_len;
        }
      }
      else
      {
        mmz_tdefl_record_match(d, d->m_saved_match_len, d->m_saved_match_dist);
        len_to_move = d->m_saved_match_len - 1; d->m_saved_match_len = 0;
      }
    }
    else if (!cur_match_dist)
      mmz_tdefl_record_literal(d, d->m_dict[MMZ_MIN(cur_pos, sizeof(d->m_dict) - 1)]);
    else if ((d->m_greedy_parsing) || (d->m_flags & MMZ_TDEFL_RLE_MATCHES) || (cur_match_len >= 128))
    {
      mmz_tdefl_record_match(d, cur_match_len, cur_match_dist);
      len_to_move = cur_match_len;
    }
    else
    {
      d->m_saved_lit = d->m_dict[MMZ_MIN(cur_pos, sizeof(d->m_dict) - 1)]; d->m_saved_match_dist = cur_match_dist; d->m_saved_match_len = cur_match_len;
    }
    // Move the lookahead forward by len_to_move bytes.
    d->m_lookahead_pos += len_to_move;
    MMZ_ASSERT(d->m_lookahead_size >= len_to_move);
    d->m_lookahead_size -= len_to_move;
    d->m_dict_size = MMZ_MIN(d->m_dict_size + len_to_move, MMZ_TDEFL_LZ_DICT_SIZE);
    // Check if it's time to flush the current LZ codes to the internal output buffer.
    if ( (d->m_pLZ_code_buf > &d->m_lz_code_buf[TDEFL_LZ_CODE_BUF_SIZE - 8]) ||
         ( (d->m_total_lz_bytes > 31*1024) && (((((mmz_uint)(d->m_pLZ_code_buf - d->m_lz_code_buf) * 115) >> 7) >= d->m_total_lz_bytes) || (d->m_flags & MMZ_TDEFL_FORCE_ALL_RAW_BLOCKS))) )
    {
      int n;
      d->m_pSrc = pSrc; d->m_src_buf_left = src_buf_left;
      if ((n = mmz_tdefl_flush_block(d, 0)) != 0)
        return (n < 0) ? MMZ_FALSE : MMZ_TRUE;
    }
  }

  d->m_pSrc = pSrc; d->m_src_buf_left = src_buf_left;
  return MMZ_TRUE;
}

static mmz_tdefl_status mmz_tdefl_flush_output_buffer(mmz_tdefl_compressor *d)
{
  if (d->m_pIn_buf_size)
  {
    *d->m_pIn_buf_size = d->m_pSrc - (const mmz_uint8 *)d->m_pIn_buf;
  }

  if (d->m_pOut_buf_size)
  {
    size_t n = MMZ_MIN(*d->m_pOut_buf_size - d->m_out_buf_ofs, d->m_output_flush_remaining);
    memcpy((mmz_uint8 *)d->m_pOut_buf + d->m_out_buf_ofs, d->m_output_buf + d->m_output_flush_ofs, n);
    d->m_output_flush_ofs += (mmz_uint)n;
    d->m_output_flush_remaining -= (mmz_uint)n;
    d->m_out_buf_ofs += n;

    *d->m_pOut_buf_size = d->m_out_buf_ofs;
  }

  return (d->m_finished && !d->m_output_flush_remaining) ? MMZ_TDEFL_STATUS_DONE : MMZ_TDEFL_STATUS_OKAY;
}

mmz_tdefl_status mmz_tdefl_compress(mmz_tdefl_compressor *d, const void *pIn_buf, size_t *pIn_buf_size, void *pOut_buf, size_t *pOut_buf_size, mmz_tdefl_flush flush)
{
  if (!d)
  {
    if (pIn_buf_size) *pIn_buf_size = 0;
    if (pOut_buf_size) *pOut_buf_size = 0;
    return MMZ_TDEFL_STATUS_BAD_PARAM;
  }

  d->m_pIn_buf = pIn_buf; d->m_pIn_buf_size = pIn_buf_size;
  d->m_pOut_buf = pOut_buf; d->m_pOut_buf_size = pOut_buf_size;
  d->m_pSrc = (const mmz_uint8 *)(pIn_buf); d->m_src_buf_left = pIn_buf_size ? *pIn_buf_size : 0;
  d->m_out_buf_ofs = 0;
  d->m_flush = flush;

  if ( ((d->m_pPut_buf_func != NULL) == ((pOut_buf != NULL) || (pOut_buf_size != NULL))) || (d->m_prev_return_status != MMZ_TDEFL_STATUS_OKAY) ||
        (d->m_wants_to_finish && (flush != MMZ_TDEFL_FINISH)) || (pIn_buf_size && *pIn_buf_size && !pIn_buf) || (pOut_buf_size && *pOut_buf_size && !pOut_buf) )
  {
    if (pIn_buf_size) *pIn_buf_size = 0;
    if (pOut_buf_size) *pOut_buf_size = 0;
    return (d->m_prev_return_status = MMZ_TDEFL_STATUS_BAD_PARAM);
  }
  d->m_wants_to_finish |= (flush == MMZ_TDEFL_FINISH);

  if ((d->m_output_flush_remaining) || (d->m_finished))
    return (d->m_prev_return_status = mmz_tdefl_flush_output_buffer(d));

  if (((d->m_flags & MMZ_TDEFL_MAX_PROBES_MASK) == 1) &&
      ((d->m_flags & MMZ_TDEFL_GREEDY_PARSING_FLAG) != 0) &&
      ((d->m_flags & (MMZ_TDEFL_FILTER_MATCHES | MMZ_TDEFL_FORCE_ALL_RAW_BLOCKS | MMZ_TDEFL_RLE_MATCHES)) == 0))
  {
    if (!mmz_tdefl_compress_fast(d))
      return d->m_prev_return_status;
  }
  else
  {
    if (!mmz_tdefl_compress_normal(d))
      return d->m_prev_return_status;
  }

  if ((d->m_flags & (MMZ_TDEFL_WRITE_ZLIB_HEADER | MMZ_TDEFL_COMPUTE_ADLER32)) && (pIn_buf))
    d->m_adler32 = (mmz_uint32)mmz_adler32(d->m_adler32, (const mmz_uint8 *)pIn_buf, d->m_pSrc - (const mmz_uint8 *)pIn_buf);

  if ((flush) && (!d->m_lookahead_size) && (!d->m_src_buf_left) && (!d->m_output_flush_remaining))
  {
    if (mmz_tdefl_flush_block(d, flush) < 0)
      return d->m_prev_return_status;
    d->m_finished = (flush == MMZ_TDEFL_FINISH);
    if (flush == MMZ_TDEFL_FULL_FLUSH) { MMZ_CLEAR_OBJ(d->m_hash); MMZ_CLEAR_OBJ(d->m_next); d->m_dict_size = 0; }
  }

  return (d->m_prev_return_status = mmz_tdefl_flush_output_buffer(d));
}

mmz_tdefl_status mmz_tdefl_compress_buffer(mmz_tdefl_compressor *d, const void *pIn_buf, size_t in_buf_size, mmz_tdefl_flush flush)
{
  MMZ_ASSERT(d->m_pPut_buf_func); return mmz_tdefl_compress(d, pIn_buf, &in_buf_size, NULL, NULL, flush);
}

mmz_tdefl_status mmz_tdefl_init(mmz_tdefl_compressor *d, mmz_tdefl_put_buf_func_ptr pPut_buf_func, void *pPut_buf_user, int flags)
{
  d->m_pPut_buf_func = pPut_buf_func; d->m_pPut_buf_user = pPut_buf_user;
  d->m_flags = (mmz_uint)(flags); d->m_max_probes[0] = 1 + ((flags & 0xFFF) + 2) / 3; d->m_greedy_parsing = (flags & MMZ_TDEFL_GREEDY_PARSING_FLAG) != 0;
  d->m_max_probes[1] = 1 + (((flags & 0xFFF) >> 2) + 2) / 3;
  if (!(flags & MMZ_TDEFL_NONDETERMINISTIC_PARSING_FLAG)) MMZ_CLEAR_OBJ(d->m_hash);
  d->m_lookahead_pos = d->m_lookahead_size = d->m_dict_size = d->m_total_lz_bytes = d->m_lz_code_buf_dict_pos = d->m_bits_in = 0;
  d->m_output_flush_ofs = d->m_output_flush_remaining = d->m_finished = d->m_block_index = d->m_bit_buffer = d->m_wants_to_finish = 0;
  d->m_pLZ_code_buf = d->m_lz_code_buf + 1; d->m_pLZ_flags = d->m_lz_code_buf; d->m_num_flags_left = 8;
  d->m_pOutput_buf = d->m_output_buf; d->m_pOutput_buf_end = d->m_output_buf; d->m_prev_return_status = MMZ_TDEFL_STATUS_OKAY;
  d->m_saved_match_dist = d->m_saved_match_len = d->m_saved_lit = 0; d->m_adler32 = 1;
  d->m_pIn_buf = NULL; d->m_pOut_buf = NULL;
  d->m_pIn_buf_size = NULL; d->m_pOut_buf_size = NULL;
  d->m_flush = MMZ_TDEFL_NO_FLUSH; d->m_pSrc = NULL; d->m_src_buf_left = 0; d->m_out_buf_ofs = 0;
  memset(&d->m_huff_count[0][0], 0, sizeof(d->m_huff_count[0][0]) * MMZ_TDEFL_MAX_HUFF_SYMBOLS_0);
  memset(&d->m_huff_count[1][0], 0, sizeof(d->m_huff_count[1][0]) * MMZ_TDEFL_MAX_HUFF_SYMBOLS_1);
  return MMZ_TDEFL_STATUS_OKAY;
}

mmz_tdefl_status mmz_tdefl_get_prev_return_status(mmz_tdefl_compressor *d)
{
  return d->m_prev_return_status;
}

mmz_uint32 mmz_tdefl_get_adler32(mmz_tdefl_compressor *d)
{
  return d->m_adler32;
}

mmz_bool mmz_tdefl_compress_mem_to_output(const void *pBuf, size_t buf_len, mmz_tdefl_put_buf_func_ptr pPut_buf_func, void *pPut_buf_user, int flags)
{
  mmz_tdefl_compressor *pComp; mmz_bool succeeded; if (((buf_len) && (!pBuf)) || (!pPut_buf_func)) return MMZ_FALSE;
  pComp = (mmz_tdefl_compressor*)MMZ_MALLOC(sizeof(mmz_tdefl_compressor)); if (!pComp) return MMZ_FALSE;
  succeeded = (mmz_tdefl_init(pComp, pPut_buf_func, pPut_buf_user, flags) == MMZ_TDEFL_STATUS_OKAY);
  succeeded = succeeded && (mmz_tdefl_compress_buffer(pComp, pBuf, buf_len, MMZ_TDEFL_FINISH) == MMZ_TDEFL_STATUS_DONE);
  MMZ_FREE(pComp); return succeeded;
}

typedef struct
{
  size_t m_size, m_capacity;
  mmz_uint8 *m_pBuf;
  mmz_bool m_expandable;
} mmz_tdefl_output_buffer;

static mmz_bool mmz_tdefl_output_buffer_putter(const void *pBuf, int len, void *pUser)
{
  mmz_tdefl_output_buffer *p = (mmz_tdefl_output_buffer *)pUser;
  size_t new_size = p->m_size + len;
  if (new_size > p->m_capacity)
  {
    size_t new_capacity = p->m_capacity; mmz_uint8 *pNew_buf; if (!p->m_expandable) return MMZ_FALSE;
    do { new_capacity = MMZ_MAX(128U, new_capacity << 1U); } while (new_size > new_capacity);
    pNew_buf = (mmz_uint8*)MMZ_REALLOC(p->m_pBuf, new_capacity); if (!pNew_buf) return MMZ_FALSE;
    p->m_pBuf = pNew_buf; p->m_capacity = new_capacity;
  }
  memcpy((mmz_uint8*)p->m_pBuf + p->m_size, pBuf, len); p->m_size = new_size;
  return MMZ_TRUE;
}

void *mmz_tdefl_compress_mem_to_heap(const void *pSrc_buf, size_t src_buf_len, size_t *pOut_len, int flags)
{
  mmz_tdefl_output_buffer out_buf; MMZ_CLEAR_OBJ(out_buf);
  if (!pOut_len) return MMZ_FALSE; else *pOut_len = 0;
  out_buf.m_expandable = MMZ_TRUE;
  if (!tdefl_compress_mem_to_output(pSrc_buf, src_buf_len, mmz_tdefl_output_buffer_putter, &out_buf, flags)) return NULL;
  *pOut_len = out_buf.m_size; return out_buf.m_pBuf;
}

size_t mmz_tdefl_compress_mem_to_mem(void *pOut_buf, size_t out_buf_len, const void *pSrc_buf, size_t src_buf_len, int flags)
{
  mmz_tdefl_output_buffer out_buf; MMZ_CLEAR_OBJ(out_buf);
  if (!pOut_buf) return 0;
  out_buf.m_pBuf = (mmz_uint8*)pOut_buf; out_buf.m_capacity = out_buf_len;
  if (!tdefl_compress_mem_to_output(pSrc_buf, src_buf_len, mmz_tdefl_output_buffer_putter, &out_buf, flags)) return 0;
  return out_buf.m_size;
}


static const mmz_uint mmz_s_tdefl_num_probes[11] = { 0, 1, 6, 32,  16, 32, 128, 256,  512, 768, 1500 };



#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable:4204) // nonstandard extension used : non-constant aggregate initializer (also supported by GNU C and C99, so no big deal)
#endif

// Simple PNG writer function by Alex Evans, 2011. Released into the public domain: https://gist.github.com/908299, more context at
// http://altdevblogaday.org/2011/04/06/a-smaller-jpg-encoder/.
// This is actually a modification of Alex's original code so PNG files generated by this function pass pngcheck.
void *mmz_tdefl_write_image_to_png_file_in_memory_ex(const void *pImage, int w, int h, int num_chans, size_t *pLen_out, mmz_uint level, mmz_bool flip)
{
  static const mmz_uint mmz_s_tdefl_png_num_probes[11] = { 0, 1, 6, 32,  16, 32, 128, 256,  512, 768, 1500 };
  mmz_tdefl_compressor *pComp = (mmz_tdefl_compressor *)MMZ_MALLOC(sizeof(mmz_tdefl_compressor)); mmz_tdefl_output_buffer out_buf; int i, bpl = w * num_chans, y, z; mmz_uint32 c; *pLen_out = 0;
  if (!pComp) return NULL;
  MMZ_CLEAR_OBJ(out_buf); out_buf.m_expandable = MMZ_TRUE; out_buf.m_capacity = 57+MMZ_MAX(64, (1+bpl)*h); if (NULL == (out_buf.m_pBuf = (mmz_uint8*)MMZ_MALLOC(out_buf.m_capacity))) { MMZ_FREE(pComp); return NULL; }
  // write dummy header
  for (z = 41; z; --z) mmz_tdefl_output_buffer_putter(&z, 1, &out_buf);
  // compress image data
  mmz_tdefl_init(pComp, mmz_tdefl_output_buffer_putter, &out_buf, mmz_s_tdefl_png_num_probes[MMZ_MIN(10, level)] | MMZ_TDEFL_WRITE_ZLIB_HEADER);
  for (y = 0; y < h; ++y) { mmz_tdefl_compress_buffer(pComp, &z, 1, MMZ_TDEFL_NO_FLUSH); mmz_tdefl_compress_buffer(pComp, (mmz_uint8*)pImage + (flip ? (h - 1 - y) : y) * bpl, bpl, MMZ_TDEFL_NO_FLUSH); }
  if (mmz_tdefl_compress_buffer(pComp, NULL, 0, MMZ_TDEFL_FINISH) != MMZ_TDEFL_STATUS_DONE) { MMZ_FREE(pComp); MMZ_FREE(out_buf.m_pBuf); return NULL; }
  // write real header
  *pLen_out = out_buf.m_size-41;
  {
    static const mmz_uint8 chans[] = {0x00, 0x00, 0x04, 0x02, 0x06};
    mmz_uint8 pnghdr[41]={0x89,0x50,0x4e,0x47,0x0d,0x0a,0x1a,0x0a,0x00,0x00,0x00,0x0d,0x49,0x48,0x44,0x52,
      0,0,(mmz_uint8)(w>>8),(mmz_uint8)w,0,0,(mmz_uint8)(h>>8),(mmz_uint8)h,8,chans[num_chans],0,0,0,0,0,0,0,
      (mmz_uint8)(*pLen_out>>24),(mmz_uint8)(*pLen_out>>16),(mmz_uint8)(*pLen_out>>8),(mmz_uint8)*pLen_out,0x49,0x44,0x41,0x54};
    c=(mmz_uint32)mmz_crc32(MMZ_CRC32_INIT,pnghdr+12,17); for (i=0; i<4; ++i, c<<=8) ((mmz_uint8*)(pnghdr+29))[i]=(mmz_uint8)(c>>24);
    memcpy(out_buf.m_pBuf, pnghdr, 41);
  }
  // write footer (IDAT CRC-32, followed by IEND chunk)
  if (!tdefl_output_buffer_putter("\0\0\0\0\0\0\0\0\x49\x45\x4e\x44\xae\x42\x60\x82", 16, &out_buf)) { *pLen_out = 0; MMZ_FREE(pComp); MMZ_FREE(out_buf.m_pBuf); return NULL; }
  c = (mmz_uint32)mmz_crc32(MMZ_CRC32_INIT,out_buf.m_pBuf+41-4, *pLen_out+4); for (i=0; i<4; ++i, c<<=8) (out_buf.m_pBuf+out_buf.m_size-16)[i] = (mmz_uint8)(c >> 24);
  // compute final size of file, grab compressed data buffer and return
  *pLen_out += 57; MMZ_FREE(pComp); return out_buf.m_pBuf;
}
void *mmz_tdefl_write_image_to_png_file_in_memory(const void *pImage, int w, int h, int num_chans, size_t *pLen_out)
{
  // Level 6 corresponds to MMZ_TDEFL_DEFAULT_MAX_PROBES or MMZ_DEFAULT_LEVEL (but we can't depend on MMZ_DEFAULT_LEVEL being available in case the zlib API's where #defined out)
  return mmz_tdefl_write_image_to_png_file_in_memory_ex(pImage, w, h, num_chans, pLen_out, 6, MMZ_FALSE);
}

#ifdef _MSC_VER
#pragma warning (pop)
#endif

#ifdef __cplusplus
}
#endif

#endif // MINIZ_HEADER_FILE_ONLY
