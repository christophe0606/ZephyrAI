/* md5.c - public domain style minimal MD5 for bare metal (no external libs)
 * Generate by GPT 5.2
 *
 * API:
 *   - md5_init(MD5_CTX*)
 *   - md5_update(MD5_CTX*, const void* data, unsigned long len)
 *   - md5_final(MD5_CTX*, unsigned char out[16])
 *   - md5_compute(const void* data, unsigned long len, unsigned char out[16])
 *   - md5_to_hex(const unsigned char in[16], char out_hex[33])  (optional)
 */

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "md5.h"


/* -------- MD5 core -------- */
typedef struct {
    uint32_t a, b, c, d;      /* state */
    uint64_t bits;            /* total message length in bits */
    uint8_t  buf[64];         /* partial block buffer */
    uint32_t buflen;          /* bytes in buf */
} MD5_CTX;

static uint32_t md5_rotl(uint32_t x, uint32_t n) {
    return (x << n) | (x >> (32u - n));
}

static uint32_t md5_load_le32(const uint8_t *p) {
    return ((uint32_t)p[0])       |
           ((uint32_t)p[1] << 8)  |
           ((uint32_t)p[2] << 16) |
           ((uint32_t)p[3] << 24);
}

static void md5_store_le32(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)(v);
    p[1] = (uint8_t)(v >> 8);
    p[2] = (uint8_t)(v >> 16);
    p[3] = (uint8_t)(v >> 24);
}

#define F(x,y,z) (((x) & (y)) | (~(x) & (z)))
#define G(x,y,z) (((x) & (z)) | ((y) & ~(z)))
#define H(x,y,z) ((x) ^ (y) ^ (z))
#define I(x,y,z) ((y) ^ ((x) | ~(z)))

#define STEP(f, a, b, c, d, x, t, s) \
    do { \
        (a) += f((b),(c),(d)) + (x) + (t); \
        (a) = md5_rotl((a), (s)); \
        (a) += (b); \
    } while (0)

static void md5_transform(md5_ctx_t ctx_, const uint8_t block[64]) {
    MD5_CTX *ctx = ctx_;
    uint32_t a = ctx->a, b = ctx->b, c = ctx->c, d = ctx->d;
    uint32_t x[16];

    for (int i = 0; i < 16; i++) {
        x[i] = md5_load_le32(block + (size_t)i * 4u);
    }

    /* Round 1 */
    STEP(F, a,b,c,d, x[ 0], 0xd76aa478,  7);
    STEP(F, d,a,b,c, x[ 1], 0xe8c7b756, 12);
    STEP(F, c,d,a,b, x[ 2], 0x242070db, 17);
    STEP(F, b,c,d,a, x[ 3], 0xc1bdceee, 22);
    STEP(F, a,b,c,d, x[ 4], 0xf57c0faf,  7);
    STEP(F, d,a,b,c, x[ 5], 0x4787c62a, 12);
    STEP(F, c,d,a,b, x[ 6], 0xa8304613, 17);
    STEP(F, b,c,d,a, x[ 7], 0xfd469501, 22);
    STEP(F, a,b,c,d, x[ 8], 0x698098d8,  7);
    STEP(F, d,a,b,c, x[ 9], 0x8b44f7af, 12);
    STEP(F, c,d,a,b, x[10], 0xffff5bb1, 17);
    STEP(F, b,c,d,a, x[11], 0x895cd7be, 22);
    STEP(F, a,b,c,d, x[12], 0x6b901122,  7);
    STEP(F, d,a,b,c, x[13], 0xfd987193, 12);
    STEP(F, c,d,a,b, x[14], 0xa679438e, 17);
    STEP(F, b,c,d,a, x[15], 0x49b40821, 22);

    /* Round 2 */
    STEP(G, a,b,c,d, x[ 1], 0xf61e2562,  5);
    STEP(G, d,a,b,c, x[ 6], 0xc040b340,  9);
    STEP(G, c,d,a,b, x[11], 0x265e5a51, 14);
    STEP(G, b,c,d,a, x[ 0], 0xe9b6c7aa, 20);
    STEP(G, a,b,c,d, x[ 5], 0xd62f105d,  5);
    STEP(G, d,a,b,c, x[10], 0x02441453,  9);
    STEP(G, c,d,a,b, x[15], 0xd8a1e681, 14);
    STEP(G, b,c,d,a, x[ 4], 0xe7d3fbc8, 20);
    STEP(G, a,b,c,d, x[ 9], 0x21e1cde6,  5);
    STEP(G, d,a,b,c, x[14], 0xc33707d6,  9);
    STEP(G, c,d,a,b, x[ 3], 0xf4d50d87, 14);
    STEP(G, b,c,d,a, x[ 8], 0x455a14ed, 20);
    STEP(G, a,b,c,d, x[13], 0xa9e3e905,  5);
    STEP(G, d,a,b,c, x[ 2], 0xfcefa3f8,  9);
    STEP(G, c,d,a,b, x[ 7], 0x676f02d9, 14);
    STEP(G, b,c,d,a, x[12], 0x8d2a4c8a, 20);

    /* Round 3 */
    STEP(H, a,b,c,d, x[ 5], 0xfffa3942,  4);
    STEP(H, d,a,b,c, x[ 8], 0x8771f681, 11);
    STEP(H, c,d,a,b, x[11], 0x6d9d6122, 16);
    STEP(H, b,c,d,a, x[14], 0xfde5380c, 23);
    STEP(H, a,b,c,d, x[ 1], 0xa4beea44,  4);
    STEP(H, d,a,b,c, x[ 4], 0x4bdecfa9, 11);
    STEP(H, c,d,a,b, x[ 7], 0xf6bb4b60, 16);
    STEP(H, b,c,d,a, x[10], 0xbebfbc70, 23);
    STEP(H, a,b,c,d, x[13], 0x289b7ec6,  4);
    STEP(H, d,a,b,c, x[ 0], 0xeaa127fa, 11);
    STEP(H, c,d,a,b, x[ 3], 0xd4ef3085, 16);
    STEP(H, b,c,d,a, x[ 6], 0x04881d05, 23);
    STEP(H, a,b,c,d, x[ 9], 0xd9d4d039,  4);
    STEP(H, d,a,b,c, x[12], 0xe6db99e5, 11);
    STEP(H, c,d,a,b, x[15], 0x1fa27cf8, 16);
    STEP(H, b,c,d,a, x[ 2], 0xc4ac5665, 23);

    /* Round 4 */
    STEP(I, a,b,c,d, x[ 0], 0xf4292244,  6);
    STEP(I, d,a,b,c, x[ 7], 0x432aff97, 10);
    STEP(I, c,d,a,b, x[14], 0xab9423a7, 15);
    STEP(I, b,c,d,a, x[ 5], 0xfc93a039, 21);
    STEP(I, a,b,c,d, x[12], 0x655b59c3,  6);
    STEP(I, d,a,b,c, x[ 3], 0x8f0ccc92, 10);
    STEP(I, c,d,a,b, x[10], 0xffeff47d, 15);
    STEP(I, b,c,d,a, x[ 1], 0x85845dd1, 21);
    STEP(I, a,b,c,d, x[ 8], 0x6fa87e4f,  6);
    STEP(I, d,a,b,c, x[15], 0xfe2ce6e0, 10);
    STEP(I, c,d,a,b, x[ 6], 0xa3014314, 15);
    STEP(I, b,c,d,a, x[13], 0x4e0811a1, 21);
    STEP(I, a,b,c,d, x[ 4], 0xf7537e82,  6);
    STEP(I, d,a,b,c, x[11], 0xbd3af235, 10);
    STEP(I, c,d,a,b, x[ 2], 0x2ad7d2bb, 15);
    STEP(I, b,c,d,a, x[ 9], 0xeb86d391, 21);

    ctx->a += a;
    ctx->b += b;
    ctx->c += c;
    ctx->d += d;
}

void md5_init(md5_ctx_t ctx_) {
    MD5_CTX *ctx = ctx_;
    ctx->a = 0x67452301u;
    ctx->b = 0xefcdab89u;
    ctx->c = 0x98badcfeu;
    ctx->d = 0x10325476u;
    ctx->bits = 0;
    ctx->buflen = 0;
}

void md5_update(md5_ctx_t ctx_, const void *data, unsigned long len) {
    MD5_CTX *ctx = ctx_;
    const uint8_t *p = (const uint8_t*)data;

    ctx->bits += (uint64_t)len * 8u;

    while (len > 0) {
        uint32_t space = 64u - ctx->buflen;
        uint32_t take = (len < space) ? (uint32_t)len : space;

        memcpy(ctx->buf + ctx->buflen, p, take);
        ctx->buflen += take;
        p += take;
        len -= take;

        if (ctx->buflen == 64u) {
            md5_transform(ctx, ctx->buf);
            ctx->buflen = 0;
        }
    }
}

void md5_final(md5_ctx_t ctx_, unsigned char out[16]) {
    MD5_CTX *ctx = ctx_;
    /* Padding: 0x80 then zeros, then 64-bit length (little endian) */
    uint8_t pad[64];
    memset(pad, 0, sizeof(pad));
    pad[0] = 0x80;

    /* how many padding bytes needed to reach 56 mod 64 */
    uint32_t mod = ctx->buflen;
    uint32_t padlen = (mod < 56u) ? (56u - mod) : (56u + 64u - mod);

    /* Save original bit count before adding padding (padding must not
       change the appended length field). */
    uint64_t orig_bits = ctx->bits;

    md5_update(ctx, pad, padlen);

    /* append original length in bits as little-endian 64-bit */
    uint8_t lenle[8];
    uint64_t bits = orig_bits;
    for (int i = 0; i < 8; i++) {
        lenle[i] = (uint8_t)(bits & 0xffu);
        bits >>= 8;
    }
    md5_update(ctx, lenle, 8);

    /* output digest as little-endian A,B,C,D */
    md5_store_le32(out + 0, ctx->a);
    md5_store_le32(out + 4, ctx->b);
    md5_store_le32(out + 8, ctx->c);
    md5_store_le32(out +12, ctx->d);

    /* wipe state (optional) */
    memset(ctx, 0, sizeof(*ctx));
}

void md5_compute(const void *data, unsigned long len, unsigned char out[16]) {
    MD5_CTX ctx;
    md5_init(&ctx);
    md5_update(&ctx, data, len);
    md5_final(&ctx, out);
}

/* Optional: convert digest to hex string (33 bytes incl. NUL) */
void md5_to_hex(const unsigned char in[16], char out_hex[33]) {
    static const char hexd[16] = "0123456789abcdef";
    for (int i = 0; i < 16; i++) {
        out_hex[i*2 + 0] = hexd[(in[i] >> 4) & 0xF];
        out_hex[i*2 + 1] = hexd[(in[i] >> 0) & 0xF];
    }
    out_hex[32] = '\0';
}
