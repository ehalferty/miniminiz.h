#include <cstdio>
#include "miniz.h"
#include "miniminiz.h"
typedef unsigned char uint8;
typedef unsigned int uint;
static const char *s_pStr = "Good morning Dr. Chandra. This is Hal. I am ready for my first lesson." \
  "Good morning Dr. Chandra. This is Hal. I am ready for my first lesson." \
  "Good morning Dr. Chandra. This is Hal. I am ready for my first lesson." \
  "Good morning Dr. Chandra. This is Hal. I am ready for my first lesson." \
  "Good morning Dr. Chandra. This is Hal. I am ready for my first lesson." \
  "Good morning Dr. Chandra. This is Hal. I am ready for my first lesson." \
  "Good morning Dr. Chandra. This is Hal. I am ready for my first lesson.";
int main(int argc, char *argv[]) {
    uLong src_len = (uLong)strlen(s_pStr);
    uLong cmp_len = mz_compressBound(src_len);
    uLong uncomp_len = src_len;
    uint8 *pCmp, *pUncomp;
    (void)argc, (void)argv;
    pCmp = (mz_uint8 *)malloc((size_t)cmp_len);
    pUncomp = (mz_uint8 *)malloc((size_t)src_len);
    mz_compress(pCmp, &cmp_len, (const unsigned char *)s_pStr, src_len);
    printf("Compressed from %u to %u bytes\n", (mz_uint32)src_len, (mz_uint32)cmp_len);
    mmz_uncompress(pUncomp, &uncomp_len, pCmp, cmp_len);
    printf("Decompressed from %u to %u bytes\n", (mz_uint32)cmp_len, (mz_uint32)uncomp_len);
    if ((uncomp_len != src_len) || (memcmp(pUncomp, s_pStr, (size_t)src_len)))
    {
        printf("Decompression failed!\n");
        free(pCmp);
        free(pUncomp);
        return EXIT_FAILURE;
    }
    free(pCmp);
    free(pUncomp);
    printf("Success.\n");
    return EXIT_SUCCESS;
}