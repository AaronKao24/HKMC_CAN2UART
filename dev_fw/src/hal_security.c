

#include <mbedtls/sha256.h>
#include <mbedtls/aes.h>



uint8_t sha256_in[32] = {0};
uint8_t sha256_out[32] = {0};
mbedtls_sha256_context sha_ctx;
void encode_sha256(uint8_t *dst_buf_32_bytes, const uint8_t *src_buf, uint32_t src_buf_size) {


  mbedtls_sha256_init(&sha_ctx);

  mbedtls_sha256_starts(&sha_ctx, 0);
  mbedtls_sha256_update(&sha_ctx, src_buf, src_buf_size);
  
  mbedtls_sha256_finish(&sha_ctx, dst_buf_32_bytes);

  mbedtls_sha256_free(&sha_ctx);
}


//	encode_sha256( sha256_out , sha256_in , 16 );
