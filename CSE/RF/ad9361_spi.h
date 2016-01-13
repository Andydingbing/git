#ifndef __AD9361_SPI__
#define __AD9361_SPI__

#include "visa.h"
#include "vpptype.h"
#include "visatype.h"
#include "visa_driver.h"
#include "stdint.h"

#define E_K7REG_W						0x00000000 << 31
#define E_K7REG_R						0x00000001 << 31
//------SPI-----------------------------------------------------------------------------------
#define E_AD9361_SPI_W					0x00000001 << 27
#define E_AD9361_SPI_W_Enable			0x00000002 << 27
#define E_AD9361_SPI_W_OverTime			0x00000003 << 27

#define E_AD9361_SPI_R					0x00000004 << 27
#define E_AD9361_SPI_R_Enable			0x00000005 << 27
#define E_AD9361_SPI_R_OverTime			0x00000006 << 27 

#define E_IOEXT_SPI_W					0x00000007 << 27 
#define E_IOEXT_SPI_W_Enable			0x00000008 << 27
#define E_IOEXT_SPI_W_OverTime			0x00000009 << 27 

#define E_IOEXT_SPI_R					0x0000000A << 27 	//Reserved
#define E_IOEXT_SPI_D					0x0000000B << 27 

int32_t ad9361_spi_read(uint32_t Addr, uint32_t *ReadData);
int32_t ad9361_spi_write(uint32_t Addr,uint32_t WriteData);

int32_t ad9361_spi_writef(uint32_t Addr,uint32_t mask, uint32_t val);
int32_t ad9361_spi_readf(uint32_t reg,uint32_t mask, uint32_t *buf);

int32_t ad9361_spi_readm(uint32_t reg, uint32_t *rbuf, uint32_t num);
int32_t ad9361_spi_writem(uint32_t reg, uint32_t *tbuf, uint32_t num);

int32_t ioexte_spi_read(uint32_t Addr, uint32_t *ReadData);
int32_t ioexte_spi_write(uint32_t Addr,uint32_t WriteData);

int32_t wait_spi_busy();
#endif