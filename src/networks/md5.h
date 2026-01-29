#ifndef md5_h 
#define md5_h

typedef void *md5_ctx_t;
void md5_init(md5_ctx_t ctx);
void md5_update(md5_ctx_t ctx, const void* data, unsigned long len);
void md5_final(md5_ctx_t ctx, unsigned char out[16]);
void md5_compute(const void* data, unsigned long len, unsigned char out[16]);
void md5_to_hex(const unsigned char in[16], char out_hex[33]) ;

#endif