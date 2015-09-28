#include "main.h"
#include <stdint.h>


static I16U _font_utf_to_unicode(I8U *UTF_8_in)
{
	 I16U unicode_16;
	 unicode_16  = (UTF_8_in[0] & 0x1F) << 12;
	 unicode_16 |= (UTF_8_in[1] & 0x3F) << 6;
	 unicode_16 |= (UTF_8_in[2] & 0x3F);
	 return unicode_16;
}



static void _font_read_one_5x7_ascii(I8U ASCIICode,I16U len, I8U *dataBuf)
{
	//start addr:368k 
	I32U addr_in=FONT_ASCII_5X7_SPACE_START;;

  if((ASCIICode >= 0x20)&&(ASCIICode<=0x7e))
  {		
		addr_in += ((ASCIICode-0x20)*8);
 		
	  NOR_readData(addr_in, len, dataBuf);		
  }
	else
	{
   Y_SPRINTF("[FONTS] No search to the 5x7 ASCII ...");		
	}
	
}



static void _font_read_one_8x16_ascii(I8U ASCIICode,I16U len, I8U *dataBuf)
{
	
	//start addr:368k + 768 byte 
	I32U addr_in=FONT_ASCII_8X16_SPACE_START;;

  if((ASCIICode >= 0x20)&&(ASCIICode<=0x7e))
  {		
		addr_in += ((ASCIICode-0x20)*16);
  
	  NOR_readData(addr_in, len, dataBuf);		
  }	
	
  else
	{
   Y_SPRINTF("[FONTS] No search to the 8x16 ASCII ...");		
	}
}


//15x16 unicode Chinese charactersascii size 32 byte
static void _font_read_one_Chinese_characters(I8U *utf_8,I16U len, I8U *dataBuf)
{

	I32U addr_in=FONT_CHINESE_SPACE_START;

  I16U unicode_16;	

	unicode_16=_font_utf_to_unicode(utf_8);
	I8U MSB=(I8U)(unicode_16 >>8);
	I8U LSB=(I8U)unicode_16;

	if(MSB>=0x4E && MSB<=0x9F)
  {		
		addr_in += ((MSB*256+LSB-0x4E00)*32);
   
	  NOR_readData(addr_in, len, dataBuf);		
  }		

  else
	{
   Y_SPRINTF("[FONTS] No search to the Chinese characters ...");		
	}
	
}


#ifdef _ENABLE_FONT_TRANSFER_
void FONT_init(void)
{    

}
#endif


#ifdef _ENABLE_FONT_TRANSFER_
static void _read_block(I32U addr, I16U len, I8U *dataBuf)
{
	I8U  Font_tx_buf[4];
				 
	Font_tx_buf[0]=0x03;
	Font_tx_buf[1]=(I8U)(addr >> 16);	
	Font_tx_buf[2]=(I8U)(addr >> 8);			
	Font_tx_buf[3]=(I8U)(addr);
	N_SPRINTF("[FONT] %x %x %x %x", g_spi_tx_buf[0], g_spi_tx_buf[1], g_spi_tx_buf[2], g_spi_tx_buf[3]);

	spi_master_tx_rx(SPI_MASTER_0, Font_tx_buf, 4, 0, 0, dataBuf, 4, len, GPIO_SPI_0_CS_FONT);
	spi_master_op_wait_done();
}
#endif

#ifdef _ENABLE_FONT_TRANSFER_
static void _block_rw_core(I32U addr_out, I32U addr_in, I32U len)
{
	I32U pos = 0;
	I32U in, out;
	I32U data[32]; // 128 bytes
	in = addr_in;
	out = addr_out;
	
	while (pos < len) {
  Y_SPRINTF("[FONTS] in: %d, out: %d, len: 128", in, out);
	_read_block(out, 128, (I8U *)data);
	NOR_pageProgram(in, 128, (I8U *)data);
	pos += 128;
	out += 128;
	in += 128;
	Watchdog_Feed();
	}
}
#endif

#ifdef _ENABLE_FONT_TRANSFER_
static void _block_verification()
{
	I32U in, i;
	I32U dataO[32]; // 128 bytes
	I8U *p1;

	in = 376832;
	for (i = 0; i < 256; i++) {
	p1 = (I8U *)dataO;
	NOR_readData(in, 128, (I8U *)dataO);
	
	Y_SPRINTF("[FONTS] dataO: %02x %02x %02x %02x %02x %02x %02x %02x", 
	p1[0],p1[1],p1[2],p1[3],p1[4],p1[5],p1[6],p1[7]);
	in += 128;
	Watchdog_Feed();
	}
}
#endif

#ifdef _ENABLE_FONT_TRANSFER_
static void _block_erase_core(I32U addr, I32U pages)
{
	I16U page_to_erase;
	page_to_erase = pages;
	
	Y_SPRINTF("[FONTS] erase all the pages before setup");
	//
	// Note: each page is 4 KB
	while (page_to_erase > 0) {
		if (page_to_erase >= 256) {
			Y_SPRINTF("[FS] erase: %d, Whole chip", addr);
			NFLASH_drv_sector_erasure(addr, ERASE_WHOLE_CHIP);
			addr += 1048576;
			page_to_erase -= 256;
		} else if ((page_to_erase >= 16) && ((addr%65536) == 0)) {
			Y_SPRINTF("[FS] erase: %d, 64K", addr);
			// we should erase 64 KB block
			NFLASH_drv_sector_erasure(addr, ERASE_64K);
			addr += 65536;
			page_to_erase -= 16;
		} else if ((page_to_erase >= 8) && ((addr%32768) == 0)) {
			Y_SPRINTF("[FS] erase: %d, 32K", addr);
			// we should erase 32 KB block
			NFLASH_drv_sector_erasure(addr, ERASE_32K);
			addr += 32768;
			page_to_erase -= 8;
		} else {
			Y_SPRINTF("[FS] erase: %d, 4K", addr);
			// we should erase 4 KB block
			NFLASH_drv_sector_erasure(addr, ERASE_4K);
			addr += 4096;
			page_to_erase -= 1;
		}
	}
}
#endif

#ifdef _ENABLE_FONT_TRANSFER_
void FONT_flash_setup()
{
	I32U source_addr, dest_addr, len, pages;
	#if 1
	// Erase whole Nor Flash before setup fonts library
	dest_addr = 376832;
	len = 671744;
	pages = len >> 12; // overall pages in 4 kBytes
	_block_erase_core(dest_addr, pages);

	// ASCII 5x7
	source_addr = 0;
	 len = 768;
	_block_rw_core(source_addr, dest_addr, len);

	// ASCII 8x16
	source_addr = 2688;
	dest_addr += len;
	len = 1536;
	_block_rw_core(source_addr, dest_addr, len);
	
	// UNICODE 15x16
	source_addr = 663840;
	dest_addr += len;
 	len = 668864+576;
	_block_rw_core(source_addr, dest_addr, len);
#endif
	// Just for verificagtion
	//_block_verification();
}
#endif
	


static I8U _get_display_len(I8U *data,I8U height)
{
	I8U  len;  //The length of the string
	I8U ascii_number=0;
	I8U chinese_number=0;
	I8U pos = 0;

	len = strlen((char*)data);
	while(pos < len)
	{
	 if((data[pos] >= 0x20)&&(data[pos]<=0x7e))//ascii
	 {
		 ascii_number+=1;
	   pos+=1;//one  ascii characters need 1 bytes
	 }
  if((data[pos]&0x80)==0x80)//chinese characters  Byte highest=1;	 
	 {
		chinese_number+=1;
	  pos+=3;//one  utf-8  chinese characters need 1 bytes
	 }
	}
	if(height==16)
		return (ascii_number*8+chinese_number*16);
	if(height==8)
		return	ascii_number*8;	
	
	return 0;
}


/*height size 16 or 8.
one  ascii characters need 1 bytes
one  utf-8 chinese characters need 3 bytes*/
//b_center ture : flash
I8U FONT_load_characters(I8U *ptr,char *data,I8U height, BOOLEAN b_center)
{
	//font data
	FONT_CTX font;
  I8U  string_len;  //The length of the string
  I8U  display_len; //The length of the display	
	I8U  as_pos = 0;
	I8U  ch_pos = 0;	

	string_len = strlen(data);
  display_len=_get_display_len((I8U*)data,height);
	N_SPRINTF("[FONTS] display len :%d ",display_len);	
	while((as_pos+ch_pos*3) < string_len)
	{
	  if(((I8U)data[as_pos+ch_pos*3] >= 0x20)&&((I8U)data[as_pos+ch_pos*3]<=0x7e))
	  {
		  if(height==16)
		  {
			  _font_read_one_8x16_ascii((I8U)data[as_pos+ch_pos*3],16,font.data);
				if(b_center==TRUE)
				 {
					 memcpy(&ptr[((128-display_len)>>1)+8*as_pos+16*ch_pos],font.data,8);
	         memcpy(&ptr[128+((128-display_len)>>1)+8*as_pos+16*ch_pos],&font.data[8],8);	
				 }
				else
				 {
					 memcpy(&ptr[8*as_pos+16*ch_pos],font.data,8);
	         memcpy(&ptr[128+8*as_pos+16*ch_pos],&font.data[8],8);						
				 }
		  }
		 if(height==8)
		 {
			 //only display 5x7 ascii
			 _font_read_one_5x7_ascii((I8U)data[as_pos+ch_pos*3],8,font.data);
			 if(b_center==TRUE)
			 {
         memcpy(&ptr[((128-display_len)>>1)+6*as_pos],font.data,6); //In the middle			 
			 }
			 else
			 {
         memcpy(&ptr[6*as_pos],font.data,6); //The starting address began to show		 
			 }

		 }
	  as_pos+=1;
		 continue;
	 }		 
   if(((I8U)data[as_pos+ch_pos*3]&0x80)==0x80)//chinese characters  Byte highest=1;
	 {
	   _font_read_one_Chinese_characters((I8U*)&data[(as_pos+ch_pos*3)],32,font.data);
		 if(b_center==TRUE)
		 {
		   memcpy(&ptr[((128-display_len)>>1)+8*as_pos+16*ch_pos],font.data,16);
       memcpy(&ptr[128+((128-display_len)>>1)+8*as_pos+16*ch_pos],&font.data[16],16);
		 }
		 else
		 {
		   memcpy(&ptr[8*as_pos+16*ch_pos],font.data,16);
       memcpy(&ptr[128+8*as_pos+16*ch_pos],&font.data[16],16);
		 }
		 ch_pos+=1;
		 continue;
	 }	
	 break;
	}
	N_SPRINTF("[FONTS] display chinese number: %d,ascii number: %d ",ch_pos,as_pos);
	if(display_len <=128)
	{
	return display_len;
	}
	else return (0xff);
}


