
#include <zephyr.h>
#include <drivers/flash.h>
#include <device.h>
#include <devicetree.h>
#include <stdio.h>
#include <string.h>
#include <logging/log.h>
#include <soc.h>
#include <mbedtls/aes.h>
#include <errno.h>

LOG_MODULE_REGISTER( fw_qspi_flash );

#define FLASH_DEVICE DT_LABEL(DT_INST(0, nordic_qspi_nor))

uint8_t ficr_er_ir[32] ; // ficr encryption root (0x0080) and identity root (0x0090)

static struct device *flash_dev;
bool fw_init_qspi_flash_rom(void)
{
	
	flash_dev = device_get_binding(FLASH_DEVICE);

	if (!flash_dev) {
		return false ;
	}
	memcpy( ficr_er_ir , NRF_FICR_BASE + 0x080 , 32 );
	return true ;
}

/**
*
* System error number The error code returned by the function. Includes the list defined by IEEE Std 1003.1-2017.
* @retval -ENOTSUP eSPI flash logical channel transactions are not supported.
* @retval -EBUSY eSPI flash channel not ready or disabled by host.
* @retval - EIO general input/output error, request to master failed.
* https://docs.zephyrproject.org/apidoc/latest/group__system__errno.html
*/
int fw_flash_erase( long addr, int page )
{
	int rc ;
	long sector_size = page * 4096 ;
	if ((addr + sector_size)>0x800000) { // 4M byte Range
		return ERANGE ; // over range
	}
	rc = flash_erase(flash_dev, addr, sector_size);
	if (rc != 0) {
		LOG_ERR("Flash erase failed! %d\n", rc);
	} else {
		LOG_INF("Flash erase succeeded!\n");
	}
	// need delay for erase ...
	return 0 ;
}

int fw_flash_write(long addr, int size ,char *p_dat)
{
	int rc;

	if ((addr + size)>0x800000) { // 2M byte Range
		return ERANGE ; // over range
	}
	rc = flash_write_protection_set( flash_dev , false );
	if (rc != 0) {
		LOG_ERR("Flash protections set failed! %d\n", rc);
		return rc ;
	}
	rc = flash_write(flash_dev, addr, p_dat, size);
	if (rc != 0) {
		LOG_ERR("Flash write failed! %d\n", rc);
		return rc ;
	}
	LOG_INF("Attempting to write %zu bytes\n", size);
	return 0 ;
}


int fw_flash_read(long addr, int size ,char *p_dat)
{
	int rc;

	if ((addr + size)>0x800000) { // 2M byte Range
		size = (addr + size)-0x800000 ;
	}
	
	memset(p_dat, 0, size);
	rc = flash_read(flash_dev, addr, p_dat, size);
	if (rc != 0) {
		LOG_ERR("Flash read failed! %d\n", rc);
		return rc ;
	}
	LOG_INF("Attempting to read %zu bytes\n", size);
	return 0 ;
}

int fw_flash_write_with_encode(long addr, int size ,char *p_dat)
{
	int rc , i ;
	mbedtls_aes_context n_ctx ;
	unsigned char iv[16] ;
	unsigned char p_out[16] ;
	if ((addr + size)>0x800000) { // 2M byte Range
		return ERANGE ; // over range
	}
	if( ((size % 16) != 0) || ((addr%16) != 0) ){
		return -EINVAL ;
	}
	
	
	mbedtls_aes_init( &n_ctx );
	//ficr_encryption_root
	mbedtls_aes_setkey_enc( &n_ctx , &ficr_er_ir , 128 );
	for( i = 0 ; i < size ; i+=16 ){
		memcpy( iv , &ficr_er_ir[16] , 16 );
		mbedtls_aes_crypt_cbc( &n_ctx , MBEDTLS_AES_ENCRYPT , 16 , iv , &(p_dat[i]) , p_out );
		rc = flash_write(flash_dev, addr + i , p_out, 16);
		if (rc != 0) {
			LOG_ERR("Flash write failed! %d\n", rc);
			return rc ;
		}
	}

	LOG_INF("Attempting to write %zu bytes\n", size);

	mbedtls_aes_free( &n_ctx );

	return 0 ;
}

int fw_flash_read_by_decode(long addr, int size ,char *p_dat)
{
	int rc , i ;
	mbedtls_aes_context n_ctx ;
	unsigned char iv[16] ;
	unsigned char p_in[16] ;

	if ((addr + size)>0x800000) { // 2M byte Range
		size = (addr + size)-0x800000 ;
	}
	
	if( ((size % 16) != 0) || ((addr%16) != 0) ){
		return -EINVAL ;
	}

	memset(p_dat, 0, size);
	mbedtls_aes_init( &n_ctx );
	//ficr_encryption_root
	mbedtls_aes_setkey_dec( &n_ctx , &ficr_er_ir , 128 );
	for( i = 0 ; i < size ; i+=16 ){
		memcpy( iv , &ficr_er_ir[16] , 16 );

		rc = flash_read(flash_dev, addr + i , p_in, 16);
		if (rc != 0) {
			LOG_ERR("Flash read failed! %d\n", rc);
			return rc ;
		}
		mbedtls_aes_crypt_cbc( &n_ctx , MBEDTLS_AES_DECRYPT , 16 , iv ,p_in, &(p_dat[i]) );
	}

	LOG_INF("Attempting to write %zu bytes\n", size);

	mbedtls_aes_free( &n_ctx );


	LOG_INF("Attempting to read %zu bytes\n", size);
	return 0 ;
}
