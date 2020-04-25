#ifndef MINIMINIZ_HEADER_INCLUDED
#define MINIMINIZ_HEADER_INCLUDED
#include <stdlib.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef unsigned long mmz_ulong;
typedef void *(*mmz_alloc_func)(void *opaque, size_t items, size_t size);
typedef void (*mmz_free_func)(void *opaque, void *address);
enum { MMZ_NO_FLUSH = 0, MMZ_PARTIAL_FLUSH = 1, MMZ_SYNC_FLUSH = 2, MMZ_FULL_FLUSH = 3, MMZ_FINISH = 4, MMZ_BLOCK = 5 };
enum { MMZ_OK = 0, MMZ_STREAM_END = 1, MMZ_NEED_DICT = 2, MMZ_ERRNO = -1, MMZ_STREAM_ERROR = -2, MMZ_DATA_ERROR = -3, MMZ_MEM_ERROR = -4, MMZ_BUF_ERROR = -5, MMZ_VERSION_ERROR = -6, MMZ_PARAM_ERROR = -10000 };
#define MMZ_DEFAULT_WINDOW_BITS 15
struct mmz_internal_state;
typedef struct mmz_stream_s {
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
int mmz_inflateInit(mmz_streamp pStream);
int mmz_inflateInit2(mmz_streamp pStream, int window_bits);
int mmz_inflate(mmz_streamp pStream, int flush);
int mmz_inflateEnd(mmz_streamp pStream);
int mmz_uncompress(unsigned char *pDest, mmz_ulong *pDest_len, const unsigned char *pSource, mmz_ulong source_len);
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
#ifdef _MSC_VER
#define MMZ_MACRO_END while (0, 0)
#else
#define MMZ_MACRO_END while (0)
#endif
enum {
    MMZ_TINFL_FLAG_PARSE_ZLIB_HEADER = 1,
    MMZ_TINFL_FLAG_HAS_MORE_INPUT = 2,
    MMZ_TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF = 4,
    MMZ_TINFL_FLAG_COMPUTE_ADLER32 = 8
};
struct mmz_tinfl_decompressor_tag; typedef struct mmz_tinfl_decompressor_tag mmz_tinfl_decompressor;
#define MMZ_TINFL_LZ_DICT_SIZE 32768
typedef enum {
    MMZ_TINFL_STATUS_BAD_PARAM = -3,
    MMZ_TINFL_STATUS_ADLER32_MISMATCH = -2,
    MMZ_TINFL_STATUS_FAILED = -1,
    MMZ_TINFL_STATUS_DONE = 0,
    MMZ_TINFL_STATUS_NEEDS_MORE_INPUT = 1,
    MMZ_TINFL_STATUS_HAS_MORE_OUTPUT = 2
} mmz_tinfl_status;
#define mmz_tinfl_init(r) do { (r)->m_state = 0; } MMZ_MACRO_END
#define mmz_tinfl_get_adler32(r) (r)->m_check_adler32
mmz_tinfl_status mmz_tinfl_decompress(mmz_tinfl_decompressor *r, const mmz_uint8 *pIn_buf_next, size_t *pIn_buf_size, mmz_uint8 *pOut_buf_start, mmz_uint8 *pOut_buf_next, size_t *pOut_buf_size, const mmz_uint32 decomp_flags);
enum {
    MMZ_TINFL_MAX_HUFF_TABLES = 3, MMZ_TINFL_MAX_HUFF_SYMBOLS_0 = 288, MMZ_TINFL_MAX_HUFF_SYMBOLS_1 = 32, MMZ_TINFL_MAX_HUFF_SYMBOLS_2 = 19,
    MMZ_TINFL_FAST_LOOKUP_BITS = 10, MMZ_TINFL_FAST_LOOKUP_SIZE = 1 << MMZ_TINFL_FAST_LOOKUP_BITS
};
typedef struct {
    mmz_uint8 m_code_size[TINFL_MAX_HUFF_SYMBOLS_0];
    mmz_int16 m_look_up[TINFL_FAST_LOOKUP_SIZE], m_tree[TINFL_MAX_HUFF_SYMBOLS_0 * 2];
} mmz_tinfl_huff_table;
typedef mmz_uint64 mmz_tinfl_bit_buf_t;
struct mmz_tinfl_decompressor_tag {
    mmz_uint32 m_state, m_num_bits, m_zhdr0, m_zhdr1, m_z_adler32, m_final, m_type, m_check_adler32, m_dist, m_counter, m_num_extra, m_table_sizes[TINFL_MAX_HUFF_TABLES];
    mmz_tinfl_bit_buf_t m_bit_buf;
    size_t m_dist_from_out_buf_start;
    mmz_tinfl_huff_table m_tables[TINFL_MAX_HUFF_TABLES];
    mmz_uint8 m_raw_header[4], m_len_codes[TINFL_MAX_HUFF_SYMBOLS_0 + MMZ_TINFL_MAX_HUFF_SYMBOLS_1 + 137];
};
enum {
    MMZ_TDEFL_WRITE_ZLIB_HEADER             = 0x01000,
    MMZ_TDEFL_COMPUTE_ADLER32               = 0x02000,
    MMZ_TDEFL_GREEDY_PARSING_FLAG           = 0x04000,
    MMZ_TDEFL_NONDETERMINISTIC_PARSING_FLAG = 0x08000,
    MMZ_TDEFL_RLE_MATCHES                   = 0x10000,
    MMZ_TDEFL_FILTER_MATCHES                = 0x20000,
    MMZ_TDEFL_FORCE_ALL_STATIC_BLOCKS       = 0x40000,
    MMZ_TDEFL_FORCE_ALL_RAW_BLOCKS          = 0x80000
};
typedef mmz_bool (*mmz_tdefl_put_buf_func_ptr)(const void* pBuf, int len, void *pUser);
enum { MMZ_TDEFL_MAX_HUFF_TABLES = 3, MMZ_TDEFL_MAX_HUFF_SYMBOLS_0 = 288, MMZ_TDEFL_MAX_HUFF_SYMBOLS_1 = 32, MMZ_TDEFL_MAX_HUFF_SYMBOLS_2 = 19, MMZ_TDEFL_LZ_DICT_SIZE = 32768, MMZ_TDEFL_LZ_DICT_SIZE_MASK = MMZ_TDEFL_LZ_DICT_SIZE - 1, MMZ_TDEFL_MIN_MATCH_LEN = 3, MMZ_TDEFL_MAX_MATCH_LEN = 258 };
enum { MMZ_TDEFL_LZ_CODE_BUF_SIZE = 64 * 1024, MMZ_TDEFL_OUT_BUF_SIZE = (MMZ_TDEFL_LZ_CODE_BUF_SIZE * 13 ) / 10, MMZ_TDEFL_MAX_HUFF_SYMBOLS = 288, MMZ_TDEFL_LZ_HASH_BITS = 15, MMZ_TDEFL_LEVEL1_HASH_SIZE_MASK = 4095, MMZ_TDEFL_LZ_HASH_SHIFT = (MMZ_TDEFL_LZ_HASH_BITS + 2) / 3, MMZ_TDEFL_LZ_HASH_SIZE = 1 << MMZ_TDEFL_LZ_HASH_BITS };
typedef enum {
    MMZ_TDEFL_STATUS_BAD_PARAM = -2,
    MMZ_TDEFL_STATUS_PUT_BUF_FAILED = -1,
    MMZ_TDEFL_STATUS_OKAY = 0,
    MMZ_TDEFL_STATUS_DONE = 1,
} mmz_tdefl_status;
typedef enum {
    MMZ_TDEFL_NO_FLUSH = 0,
    MMZ_TDEFL_SYNC_FLUSH = 2,
    MMZ_TDEFL_FULL_FLUSH = 3,
    MMZ_TDEFL_FINISH = 4
} mmz_tdefl_flush;
typedef struct {
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
#ifdef __cplusplus
}
#endif
#endif
#ifndef MINIZ_HEADER_FILE_ONLY
typedef unsigned char mmz_validate_uint16[sizeof(mmz_uint16)==2 ? 1 : -1];
typedef unsigned char mmz_validate_uint32[sizeof(mmz_uint32)==4 ? 1 : -1];
typedef unsigned char mmz_validate_uint64[sizeof(mmz_uint64)==8 ? 1 : -1];
#include <string.h>
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
typedef struct {
    mmz_tinfl_decompressor m_decomp;
    mmz_uint m_dict_ofs, m_dict_avail, m_first_call, m_has_flushed; int m_window_bits;
    mmz_uint8 m_dict[TINFL_LZ_DICT_SIZE];
    mmz_tinfl_status m_last_status;
} mmz_inflate_state;
int mmz_inflateInit2(mmz_streamp pStream, int window_bits) {
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
int mmz_inflateInit(mmz_streamp pStream) {
    return mmz_inflateInit2(pStream, MMZ_DEFAULT_WINDOW_BITS);
}
int mmz_inflate(mmz_streamp pStream, int flush) {
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
    if ((flush == MMZ_FINISH) && (first_call)) {
        decomp_flags |= MMZ_TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF;
        in_bytes = pStream->avail_in; out_bytes = pStream->avail_out;
        status = mmz_tinfl_decompress(&pState->m_decomp, pStream->next_in, &in_bytes, pStream->next_out, pStream->next_out, &out_bytes, decomp_flags);
        pState->m_last_status = status;
        pStream->next_in += (mmz_uint)in_bytes; pStream->avail_in -= (mmz_uint)in_bytes; pStream->total_in += (mmz_uint)in_bytes;
        pStream->adler = mmz_tinfl_get_adler32(&pState->m_decomp);
        pStream->next_out += (mmz_uint)out_bytes; pStream->avail_out -= (mmz_uint)out_bytes; pStream->total_out += (mmz_uint)out_bytes;
        if (status < 0)
            return MMZ_DATA_ERROR;
        else if (status != MMZ_TINFL_STATUS_DONE) {
            pState->m_last_status = MMZ_TINFL_STATUS_FAILED;
            return MMZ_BUF_ERROR;
        }
        return MMZ_STREAM_END;
    }
    // flush != MMZ_FINISH then we must assume there's more input.
    if (flush != MMZ_FINISH) decomp_flags |= MMZ_TINFL_FLAG_HAS_MORE_INPUT;
    if (pState->m_dict_avail) {
        n = MMZ_MIN(pState->m_dict_avail, pStream->avail_out);
        memcpy(pStream->next_out, pState->m_dict + pState->m_dict_ofs, n);
        pStream->next_out += n; pStream->avail_out -= n; pStream->total_out += n;
        pState->m_dict_avail -= n; pState->m_dict_ofs = (pState->m_dict_ofs + n) & (TINFL_LZ_DICT_SIZE - 1);
        return ((pState->m_last_status == MMZ_TINFL_STATUS_DONE) && (!pState->m_dict_avail)) ? MMZ_STREAM_END : MMZ_OK;
    }
    for ( ; ; ) {
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
        else if (flush == MMZ_FINISH) {
            if (status == MMZ_TINFL_STATUS_DONE)
                return pState->m_dict_avail ? MMZ_BUF_ERROR : MMZ_STREAM_END;
            else if (!pStream->avail_out)
                return MMZ_BUF_ERROR;
        }
        else if ((status == MMZ_TINFL_STATUS_DONE) || (!pStream->avail_in) || (!pStream->avail_out) || (pState->m_dict_avail))
            break;
    }
    return ((status == MMZ_TINFL_STATUS_DONE) && (!pState->m_dict_avail)) ? MMZ_STREAM_END : MMZ_OK;
}
int mmz_inflateEnd(mmz_streamp pStream) {
    if (!pStream)
        return MMZ_STREAM_ERROR;
    if (pStream->state) {
        pStream->zfree(pStream->opaque, pStream->state);
        pStream->state = NULL;
    }
    return MMZ_OK;
}
int mmz_uncompress(unsigned char *pDest, mmz_ulong *pDest_len, const unsigned char *pSource, mmz_ulong source_len) {
    mmz_stream stream;
    int status;
    memset(&stream, 0, sizeof(stream));
    if ((source_len | *pDest_len) > 0xFFFFFFFFU) return MMZ_PARAM_ERROR;
    stream.next_in = pSource;
    stream.avail_in = (mmz_uint32)source_len;
    stream.next_out = pDest;
    stream.avail_out = (mmz_uint32)*pDest_len;
    status = mmz_inflateInit(&stream);
    if (status != MMZ_OK)
        return status;
    status = mmz_inflate(&stream, MMZ_FINISH);
    if (status != MMZ_STREAM_END) {
        mmz_inflateEnd(&stream);
        return ((status == MMZ_BUF_ERROR) && (!stream.avail_in)) ? MMZ_DATA_ERROR : status;
    }
    *pDest_len = stream.total_out;
    return mmz_inflateEnd(&stream);
}
#define MMZ_TINFL_MEMCPY(d, s, l) memcpy(d, s, l)
#define MMZ_TINFL_MEMSET(p, c, l) memset(p, c, l)
#define MMZ_TINFL_CR_BEGIN switch(r->m_state) { case 0:
#define MMZ_TINFL_CR_RETURN(state_index, result) do { status = result; r->m_state = state_index; goto common_exit; case state_index:; } MMZ_MACRO_END
#define MMZ_TINFL_CR_RETURN_FOREVER(state_index, result) do { for ( ; ; ) { MMZ_TINFL_CR_RETURN(state_index, result); } } MMZ_MACRO_END
#define MMZ_TINFL_CR_FINISH }
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
mmz_tinfl_status mmz_tinfl_decompress(mmz_tinfl_decompressor *r, const mmz_uint8 *pIn_buf_next, size_t *pIn_buf_size, mmz_uint8 *pOut_buf_start, mmz_uint8 *pOut_buf_next, size_t *pOut_buf_size, const mmz_uint32 decomp_flags) {
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
    if (((out_buf_size_mask + 1) & out_buf_size_mask) || (pOut_buf_next < pOut_buf_start)) { *pIn_buf_size = *pOut_buf_size = 0; return MMZ_TINFL_STATUS_BAD_PARAM; }
    num_bits = r->m_num_bits; bit_buf = r->m_bit_buf; dist = r->m_dist; counter = r->m_counter; num_extra = r->m_num_extra; dist_from_out_buf_start = r->m_dist_from_out_buf_start;
    MMZ_TINFL_CR_BEGIN
            bit_buf = num_bits = dist = counter = num_extra = r->m_zhdr0 = r->m_zhdr1 = 0; r->m_z_adler32 = r->m_check_adler32 = 1;
            if (decomp_flags & MMZ_TINFL_FLAG_PARSE_ZLIB_HEADER) {
                MMZ_TINFL_GET_BYTE(1, r->m_zhdr0); MMZ_TINFL_GET_BYTE(2, r->m_zhdr1);
                counter = (((r->m_zhdr0 * 256 + r->m_zhdr1) % 31 != 0) || (r->m_zhdr1 & 32) || ((r->m_zhdr0 & 15) != 8));
                if (!(decomp_flags & MMZ_TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF)) counter |= (((1U << (8U + (r->m_zhdr0 >> 4))) > 32768U) || ((out_buf_size_mask + 1) < (size_t)(1U << (8U + (r->m_zhdr0 >> 4)))));
                if (counter) { MMZ_TINFL_CR_RETURN_FOREVER(36, MMZ_TINFL_STATUS_FAILED); }
            }
            do {
                MMZ_TINFL_GET_BITS(3, r->m_final, 3); r->m_type = r->m_final >> 1;
                if (r->m_type == 0) {
                    MMZ_TINFL_SKIP_BITS(5, num_bits & 7);
                    for (counter = 0; counter < 4; ++counter) { if (num_bits) MMZ_TINFL_GET_BITS(6, r->m_raw_header[counter], 8); else MMZ_TINFL_GET_BYTE(7, r->m_raw_header[counter]); }
                    if ((counter = (r->m_raw_header[0] | (r->m_raw_header[1] << 8))) != (mmz_uint)(0xFFFF ^ (r->m_raw_header[2] | (r->m_raw_header[3] << 8)))) { MMZ_TINFL_CR_RETURN_FOREVER(39, MMZ_TINFL_STATUS_FAILED); }
                    while ((counter) && (num_bits)) {
                        MMZ_TINFL_GET_BITS(51, dist, 8);
                        while (pOut_buf_cur >= pOut_buf_end) { MMZ_TINFL_CR_RETURN(52, MMZ_TINFL_STATUS_HAS_MORE_OUTPUT); }
                        *pOut_buf_cur++ = (mmz_uint8)dist;
                        counter--;
                    }
                    while (counter) {
                        size_t n; while (pOut_buf_cur >= pOut_buf_end) { MMZ_TINFL_CR_RETURN(9, MMZ_TINFL_STATUS_HAS_MORE_OUTPUT); }
                        while (pIn_buf_cur >= pIn_buf_end) {
                            if (decomp_flags & MMZ_TINFL_FLAG_HAS_MORE_INPUT) {
                                MMZ_TINFL_CR_RETURN(38, MMZ_TINFL_STATUS_NEEDS_MORE_INPUT);
                            } else {
                                MMZ_TINFL_CR_RETURN_FOREVER(40, MMZ_TINFL_STATUS_FAILED);
                            }
                        }
                        n = MMZ_MIN(MMZ_MIN((size_t)(pOut_buf_end - pOut_buf_cur), (size_t)(pIn_buf_end - pIn_buf_cur)), counter);
                        MMZ_TINFL_MEMCPY(pOut_buf_cur, pIn_buf_cur, n); pIn_buf_cur += n; pOut_buf_cur += n; counter -= (mmz_uint)n;
                    }
                } else if (r->m_type == 3) {
                    MMZ_TINFL_CR_RETURN_FOREVER(10, MMZ_TINFL_STATUS_FAILED);
                } else {
                    if (r->m_type == 1) {
                        mmz_uint8 *p = r->m_tables[0].m_code_size; mmz_uint i;
                        r->m_table_sizes[0] = 288; r->m_table_sizes[1] = 32; MMZ_TINFL_MEMSET(r->m_tables[1].m_code_size, 5, 32);
                        for ( i = 0; i <= 143; ++i) *p++ = 8; for ( ; i <= 255; ++i) *p++ = 9; for ( ; i <= 279; ++i) *p++ = 7; for ( ; i <= 287; ++i) *p++ = 8;
                    } else {
                        for (counter = 0; counter < 3; counter++) { MMZ_TINFL_GET_BITS(11, r->m_table_sizes[counter], "\05\05\04"[counter]); r->m_table_sizes[counter] += s_min_table_sizes[counter]; }
                        MMZ_CLEAR_OBJ(r->m_tables[2].m_code_size); for (counter = 0; counter < r->m_table_sizes[2]; counter++) { mmz_uint s; MMZ_TINFL_GET_BITS(14, s, 3); r->m_tables[2].m_code_size[s_length_dezigzag[counter]] = (mmz_uint8)s; }
                        r->m_table_sizes[2] = 19;
                    }
                    for ( ; (int)r->m_type >= 0; r->m_type--) {
                        int tree_next, tree_cur; mmz_tinfl_huff_table *pTable;
                        mmz_uint i, j, used_syms, total, sym_index, next_code[17], total_syms[16]; pTable = &r->m_tables[r->m_type]; MMZ_CLEAR_OBJ(total_syms); MMZ_CLEAR_OBJ(pTable->m_look_up); MMZ_CLEAR_OBJ(pTable->m_tree);
                        for (i = 0; i < r->m_table_sizes[r->m_type]; ++i) total_syms[pTable->m_code_size[i]]++;
                        used_syms = 0, total = 0; next_code[0] = next_code[1] = 0;
                        for (i = 1; i <= 15; ++i) { used_syms += total_syms[i]; next_code[i + 1] = (total = ((total + total_syms[i]) << 1)); }
                        if ((65536 != total) && (used_syms > 1)) {
                            MMZ_TINFL_CR_RETURN_FOREVER(35, MMZ_TINFL_STATUS_FAILED);
                        }
                        for (tree_next = -1, sym_index = 0; sym_index < r->m_table_sizes[r->m_type]; ++sym_index) {
                            mmz_uint rev_code = 0, l, cur_code, code_size = pTable->m_code_size[sym_index]; if (!code_size) continue;
                            cur_code = next_code[code_size]++; for (l = code_size; l > 0; l--, cur_code >>= 1) rev_code = (rev_code << 1) | (cur_code & 1);
                            if (code_size <= MMZ_TINFL_FAST_LOOKUP_BITS) { mmz_int16 k = (mmz_int16)((code_size << 9) | sym_index); while (rev_code < MMZ_TINFL_FAST_LOOKUP_SIZE) { pTable->m_look_up[rev_code] = k; rev_code += (1 << code_size); } continue; }
                            if (0 == (tree_cur = pTable->m_look_up[rev_code & (TINFL_FAST_LOOKUP_SIZE - 1)])) { pTable->m_look_up[rev_code & (TINFL_FAST_LOOKUP_SIZE - 1)] = (mmz_int16)tree_next; tree_cur = tree_next; tree_next -= 2; }
                            rev_code >>= (TINFL_FAST_LOOKUP_BITS - 1);
                            for (j = code_size; j > (TINFL_FAST_LOOKUP_BITS + 1); j--) {
                                tree_cur -= ((rev_code >>= 1) & 1);
                                if (!pTable->m_tree[-tree_cur - 1]) { pTable->m_tree[-tree_cur - 1] = (mmz_int16)tree_next; tree_cur = tree_next; tree_next -= 2; } else tree_cur = pTable->m_tree[-tree_cur - 1];
                            }
                            tree_cur -= ((rev_code >>= 1) & 1); pTable->m_tree[-tree_cur - 1] = (mmz_int16)sym_index;
                        }
                        if (r->m_type == 2) {
                            for (counter = 0; counter < (r->m_table_sizes[0] + r->m_table_sizes[1]); ) {
                                mmz_uint s; MMZ_TINFL_HUFF_DECODE(16, dist, &r->m_tables[2]); if (dist < 16) { r->m_len_codes[counter++] = (mmz_uint8)dist; continue; }
                                if ((dist == 16) && (!counter)) {
                                    MMZ_TINFL_CR_RETURN_FOREVER(17, MMZ_TINFL_STATUS_FAILED);
                                }
                                num_extra = "\02\03\07"[dist - 16]; MMZ_TINFL_GET_BITS(18, s, num_extra); s += "\03\03\013"[dist - 16];
                                MMZ_TINFL_MEMSET(r->m_len_codes + counter, (dist == 16) ? r->m_len_codes[counter - 1] : 0, s); counter += s;
                            }
                            if ((r->m_table_sizes[0] + r->m_table_sizes[1]) != counter) {
                                MMZ_TINFL_CR_RETURN_FOREVER(21, MMZ_TINFL_STATUS_FAILED);
                            }
                            MMZ_TINFL_MEMCPY(r->m_tables[0].m_code_size, r->m_len_codes, r->m_table_sizes[0]); MMZ_TINFL_MEMCPY(r->m_tables[1].m_code_size, r->m_len_codes + r->m_table_sizes[0], r->m_table_sizes[1]);
                        }
                    }
                    for ( ; ; ) {
                        mmz_uint8 *pSrc;
                        for ( ; ; ) {
                            if (((pIn_buf_end - pIn_buf_cur) < 4) || ((pOut_buf_end - pOut_buf_cur) < 2)) {
                                MMZ_TINFL_HUFF_DECODE(23, counter, &r->m_tables[0]);
                                if (counter >= 256)
                                    break;
                                while (pOut_buf_cur >= pOut_buf_end) { MMZ_TINFL_CR_RETURN(24, MMZ_TINFL_STATUS_HAS_MORE_OUTPUT); }
                                *pOut_buf_cur++ = (mmz_uint8)counter;
                            } else {
                                int sym2; mmz_uint code_len;
                                if (num_bits < 30) { bit_buf |= (((tinfl_bit_buf_t)MMZ_READ_LE32(pIn_buf_cur)) << num_bits); pIn_buf_cur += 4; num_bits += 32; }
                                if ((sym2 = r->m_tables[0].m_look_up[bit_buf & (TINFL_FAST_LOOKUP_SIZE - 1)]) >= 0)
                                    code_len = sym2 >> 9;
                                else {
                                    code_len = MMZ_TINFL_FAST_LOOKUP_BITS; do { sym2 = r->m_tables[0].m_tree[~sym2 + ((bit_buf >> code_len++) & 1)]; } while (sym2 < 0);
                                }
                                counter = sym2; bit_buf >>= code_len; num_bits -= code_len;
                                if (counter & 256)
                                    break;
                                if ((sym2 = r->m_tables[0].m_look_up[bit_buf & (TINFL_FAST_LOOKUP_SIZE - 1)]) >= 0)
                                    code_len = sym2 >> 9;
                                else {
                                    code_len = MMZ_TINFL_FAST_LOOKUP_BITS; do { sym2 = r->m_tables[0].m_tree[~sym2 + ((bit_buf >> code_len++) & 1)]; } while (sym2 < 0);
                                }
                                bit_buf >>= code_len; num_bits -= code_len;
                                pOut_buf_cur[0] = (mmz_uint8)counter;
                                if (sym2 & 256) {
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
                        if ((dist > dist_from_out_buf_start) && (decomp_flags & MMZ_TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF)) {
                            MMZ_TINFL_CR_RETURN_FOREVER(37, MMZ_TINFL_STATUS_FAILED);
                        }
                        pSrc = pOut_buf_start + ((dist_from_out_buf_start - dist) & out_buf_size_mask);
                        if ((MMZ_MAX(pOut_buf_cur, pSrc) + counter) > pOut_buf_end) {
                            while (counter--) {
                                while (pOut_buf_cur >= pOut_buf_end) { MMZ_TINFL_CR_RETURN(53, MMZ_TINFL_STATUS_HAS_MORE_OUTPUT); }
                                *pOut_buf_cur++ = pOut_buf_start[(dist_from_out_buf_start++ - dist) & out_buf_size_mask];
                            }
                            continue;
                        }
                        else if ((counter >= 9) && (counter <= dist)) {
                            const mmz_uint8 *pSrc_end = pSrc + (counter & ~7);
                            do {
                                ((mmz_uint32 *)pOut_buf_cur)[0] = ((const mmz_uint32 *)pSrc)[0];
                                ((mmz_uint32 *)pOut_buf_cur)[1] = ((const mmz_uint32 *)pSrc)[1];
                                pOut_buf_cur += 8;
                            } while ((pSrc += 8) < pSrc_end);
                            if ((counter &= 7) < 3) {
                                if (counter) {
                                    pOut_buf_cur[0] = pSrc[0];
                                    if (counter > 1)
                                        pOut_buf_cur[1] = pSrc[1];
                                    pOut_buf_cur += counter;
                                }
                                continue;
                            }
                        }
                        do {
                            pOut_buf_cur[0] = pSrc[0];
                            pOut_buf_cur[1] = pSrc[1];
                            pOut_buf_cur[2] = pSrc[2];
                            pOut_buf_cur += 3; pSrc += 3;
                        } while ((int)(counter -= 3) > 2);
                        if ((int)counter > 0) {
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

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable:4204) // nonstandard extension used : non-constant aggregate initializer (also supported by GNU C and C99, so no big deal)
#endif
#ifdef _MSC_VER
#pragma warning (pop)
#endif
#ifdef __cplusplus
}
#endif
#endif
