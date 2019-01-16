/* eagle.flash.bin     @ 0x00000 */
/* eagle.irom0text.bin @ 0x40000 */

/* Flash Map, 4MB SPI Flash */
/* |......|..............................|..........................|.....|....|                       */
/* ^      ^                              ^                          ^     ^                            */
/* |_flash.bin start(0x0000)             |_irom0text.bin start(0x40000)   |                            */
/*        |_flash.bin end                                           |_irom0text.bin end                */
/*                                                                        |_system param area(0x3fb000) */

/* NOTICE: midify by karlno*/ 
/* 1. 你可以修改irom0 org, 但是必须保证irom0text.bin的头部没有覆盖flash.bin的尾部.   */
/* 2. 你可以修改irom0 len, 但是必须保证irom0text.bin的尾部没有覆盖system param区域. */
/* 3. flash.bin尾部到irom0text.bin头部的空间可以存放用户数据.           */
/* 4. irom0text.bin尾部到system param区域的空间可以存放用户数据.         */
/* 5. 必须保证irom0text.bin尾部 < 0x100000                                                        */
/* 6. system param区域的地址:                                                                            */
/*    1>. 512KB--->0x07b000                                                                         */
/*    2>. 1MB----->0x0fb000                                                                         */
/*    3>. 2MB----->0x1fb000                                                                         */
/*    4>. 4MB----->0x3fb000                                                                         */
/* 7. 不要修改其他seg.                                                                   */

MEMORY
{
  dport0_0_seg :                      	org = 0x3FF00000, len = 0x10
  dram0_0_seg :                       	org = 0x3FFE8000, len = 0x18000
  iram1_0_seg :                       	org = 0x40100000, len = 0x8000
  irom0_0_seg :                       	org = 0x40240000, len = 0x80000
}

INCLUDE "../ld/eagle.app.v6.common.ld"