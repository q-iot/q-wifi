#ifndef SPI_FLASH_API_H
#define SPI_FLASH_API_H

void SpiFlsEraseSector(u16 sector,u16 num);
#define SpiFlsReadData(Addr,Len,pBuf) spi_flash_read(Addr,pBuf,Len)
#define SpiFlsWriteData(Addr,Len,pBuf) spi_flash_write(Addr,pBuf,Len) //addr必须4字节对齐，否则可能出现问题，esp sdk bug
//#define SpiFlsInit() 

#define FLASH_SECTOR_BYTES 4096
#define FLASH_PAGE_SIZE 256

// 4m bytes flash布局
// 4k一个扇区
// start	size		sec number		name
// 0			64k:		sec 0 - 15 		user_flash.bin
// 64k		192k:	sec 16 - 63 		user data/user param
// 256k		768k:	sec 64 - 255 	irom0text.bin
// 1024k	1024k:	sec 256 - 511 	user data
// 2048k	1024k:	sec 512 - 767 	user data
// 3072k	1004k:	sec 768 - 1018 user data
// 4076k	20k:		sec 1019 -1023 system param

//基地址
#define CHIP_ROM_BASE 0x40240000 //cpu中的寻址地址，映射到spi flash的irom0
#define SPI_FLASH_IROM0_START_ADDR 0x40000//irom0存放在spi flash中的地址
#define IROM0_ADDR_OFFSET (CHIP_ROM_BASE-SPI_FLASH_IROM0_START_ADDR)

//flash code后面的剩余
#define FM_CODE_AFTER_BASE_SECTOR 16
#define FM_CODE_AFTER_SEC_NUM 48

//irom后面的剩余
#define FM_IROM_AFTER_BASE_SECTOR 192
#define FM_IROM_AFTER_SEC_NUM 64

//-------------------------------- 1M以后的布局 --------------------------------
//基础数据库
#define FM_DATABASE_BASE_SECTOR 256 //0x10 0000 :1M
#define FM_DATABASE_SEC_NUM 16

//快速存储数据库
#define FM_QUICKSAVE_BASE_SECTOR 300
#define FM_QUICKSAVE_SEC_NUM 16

//硬件配置数据库
#define FM_HWCONF_BASE_SECTOR 320
#define FM_HWCONF_SEC_NUM 16

//info数据库
#define FM_INFOSAVE_BASE_SECTOR 350
#define FM_INFOSAVE_SEC_NUM 96

//字库
#define FM_FONT_A14B_BASE_SECTOR 452 //0x1C 4000
#define FM_FONT_A14B_SEC_NUM 1

#define FM_FONT_G16_BASE_SECTOR 464
#define FM_FONT_G16_SEC_NUM 200

#define FM_FONT_G12_BASE_SECTOR 464
#define FM_FONT_G12_SEC_NUM 0

#define FM_FONT_G21_BASE_SECTOR 464
#define FM_FONT_G21_SEC_NUM 0

//web文件
#define FM_WEB_FILES_BASE_SECTOR 768 //0x30 0000
#define FM_WEB_FILES_SEC_NUM 200

//system param，程序不可用
#define FM_SYSTEM_PARAM_BASE_SECTOR 1019 //0x3f b000
#define FM_SYSTME_PARAM_SEC_NUM 5
//-------------------------------- 4M 布局结束 --------------------------------







#endif

