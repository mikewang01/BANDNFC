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


static I16U _font_get_gb12345_code(I16U	Unicode)
{
	I32U BaseAddr ,Address;
	I16U GB12345_code;
	I8U  data[2];	
	
	if(Unicode <= 0x0451 && Unicode >= 0x00a0)
  {
    BaseAddr=0;
    Address=BaseAddr+((Unicode-0x00a0)<<1);
  }
  else if(Unicode <= 0x2642 && Unicode>= 0x2010)
  {
    BaseAddr=1892;
    Address=BaseAddr+((Unicode-0x2010)<<1);
  }
  else if(Unicode <= 0x33d5 && Unicode >= 0x3000)
  {
    BaseAddr=5066;
    Address=BaseAddr+((Unicode-0x3000)<<1);
  }
  else if(Unicode <= 0x9fa5 && Unicode >= 0x4e00)
  {
    BaseAddr=7030;
    Address=BaseAddr+((Unicode-0x4e00)<<1);
  }
  else if(Unicode <= 0xfe6b && Unicode >= 0xfe30)
  {
    BaseAddr=48834;
    Address=BaseAddr+((Unicode-0xfe30)<<1);
  }
  else if(Unicode <= 0xff5e && Unicode >= 0xff01)
  {
    BaseAddr=48954;
    Address=BaseAddr+((Unicode-0xff01)<<1);
  }
  else if(Unicode <= 0xffe5 && Unicode >= 0xffe0)
  {
    BaseAddr=49142;
    Address=BaseAddr+((Unicode-0xffe0)<<1);
  }
  else if(Unicode <= 0xFA29 && Unicode >= 0xF92C)
  {
    BaseAddr=49312;
    Address=BaseAddr+((Unicode-0XF92C)<<1);
  }
  else if(Unicode <= 0xE864 && Unicode >= 0xE816)
  {
    BaseAddr=49820;
    Address=BaseAddr+((Unicode-0XE816)<<1);
  }
  else if(Unicode <= 0x2ECA && Unicode >= 0x2E81)
  {
    BaseAddr=49978;
    Address=BaseAddr+((Unicode-0X2E81)<<1);
  }
  else if(Unicode <= 0x49B7 && Unicode >= 0x4947)
  {
    BaseAddr=50126;
    Address=BaseAddr+((Unicode-0X4947)<<1);
  }
  else if(Unicode <= 0x4DAE && Unicode >= 0x4C77)
  {
    BaseAddr=50352;
    Address=BaseAddr+((Unicode-0X4C77)<<1);
  }
  else if(Unicode <= 0x3CE0 && Unicode >= 0x3447)
  {
    BaseAddr=50976;
    Address=BaseAddr+((Unicode-0X3447)<<1);
  }
  else if(Unicode <= 0X478D && Unicode >= 0x4056)
  {
    BaseAddr=55380;
    Address=BaseAddr+((Unicode-0X4056)<<1);
  }
  else
    return 0;

  Address += FONT_DIGIT_TRANSLATION_SPACE_START;

  NOR_readData(Address, 2, data);

  GB12345_code =  ((data[0]<<8) + data[1]);

  return GB12345_code;
}


//chinese traditional characters need change display mode
static void _font_display_conversion(I8U *dataBuf)
{
	I8U data_replace[32];
  I8U	pos_1 = 0;
  I8U	pos_2 = 0;	

	for(I8U index=0;index<32;index++)
	{
		if(index <= 7)
		pos_2	=14;
		else if(index <= 15 && index >= 8)
		pos_2	=15;
		else if(index <= 23 && index >= 16)			
		pos_2	=30;
		else if(index <= 31 && index >= 24)		
		pos_2	=31;		
		
		data_replace[index]=0;
			
	  data_replace[index] |= (dataBuf[pos_2]&(0x80>>pos_1))<<pos_1;
		
		if(pos_1 <= 1) 
	  data_replace[index] |= (dataBuf[pos_2-2]&(0x80>>pos_1))>>(1-pos_1);
		else
		data_replace[index] |= (dataBuf[pos_2-2]&(0x80>>pos_1))<<(pos_1 - 1);
		
		if(pos_1 <= 2)
	  data_replace[index] |= (dataBuf[pos_2-4]&(0x80>>pos_1))>>(2-pos_1);
		else
		data_replace[index] |= (dataBuf[pos_2-4]&(0x80>>pos_1))<<(pos_1-2);	
		
		if(pos_1 <= 3)
	  data_replace[index] |= (dataBuf[pos_2-6]&(0x80>>pos_1))>>(3-pos_1);
		else
		data_replace[index] |= (dataBuf[pos_2-6]&(0x80>>pos_1))<<(pos_1-3);	
		
		if(pos_1 <= 4)
	  data_replace[index] |= (dataBuf[pos_2-8]&(0x80>>pos_1))>>(4-pos_1);
		else
    data_replace[index] |= (dataBuf[pos_2-8]&(0x80>>pos_1))<<(pos_1-4);			
		
		if(pos_1 <= 5)
	  data_replace[index] |= (dataBuf[pos_2-10]&(0x80>>pos_1))>>(5-pos_1);
		else
		data_replace[index] |= (dataBuf[pos_2-10]&(0x80>>pos_1))<<(pos_1-5);	
		
		if(pos_1 <= 6)
	  data_replace[index] |= (dataBuf[pos_2-12]&(0x80>>pos_1))>>(6-pos_1);
		else
		data_replace[index] |= (dataBuf[pos_2-12]&(0x80>>pos_1))<<(pos_1-6);	
		
	  data_replace[index] |= (dataBuf[pos_2-14]&(0x80>>pos_1))>>(7-pos_1);
		
		pos_1++;
		if(pos_1 >= 8)
		  pos_1 = 0;	
	}

	memcpy(dataBuf,data_replace,32);
}


static void _font_read_one_5x7_ascii(I8U ASCIICode,I16U len, I8U *dataBuf)
{
	//start addr:368k 
	I32U addr_in=FONT_ASCII_5X7_SPACE_START;;

	addr_in += ((ASCIICode-0x20)<<3);
	
	NOR_readData(addr_in, len, dataBuf);	
}



static void _font_read_one_8x16_ascii(I8U ASCIICode,I16U len, I8U *dataBuf)
{
	//start addr:368k + 768 byte 
	I32U addr_in=FONT_ASCII_8X16_SPACE_START;;

	addr_in += ((ASCIICode-0x20)<<4);

	NOR_readData(addr_in, len, dataBuf);	
}

//simple chinese characters
//15x16 unicode simple Chinese characters size 32 byte
static void _font_read_one_simple_Chinese_characters(I8U *utf_8,I16U len, I8U *dataBuf)
{
	I32U addr_in=FONT_SIMPLE_CHINESE_SPACE_START;
  I16U unicode_16;	
	I8U replace_string[16];

	unicode_16=_font_utf_to_unicode(utf_8);
	I8U MSB=(I8U)(unicode_16 >>8);
	I8U LSB=(I8U)unicode_16;

	if((MSB<0x4E) || (MSB>0x9F)) {
    N_SPRINTF("[FONTS] No search to the Chinese characters ...");	
		memset(dataBuf, 0, 32);
		
		// use some predefined characters ("")in here
		_font_read_one_8x16_ascii('"',16,replace_string);
		memcpy(dataBuf+4,replace_string,8);
		memcpy(dataBuf+16,&replace_string[8],8);		
    return;		
	}
	
	addr_in += ((MSB*256+LSB-0x4E00)<<5);
	NOR_readData(addr_in, len, dataBuf);	

}


//15x16 unicode traditional Chinese  charactersa size 32 byte
static void _font_read_one_traditional_Chinese_characters(I8U *utf_8,I16U len, I8U *dataBuf)
{
	I32U BaseAdd=FONT_TRADITIONAL_CHINESE_SPACE_START;
	I32U addr;
  I16U unicode_16;	
  I16U GB_12345;		

	unicode_16=_font_utf_to_unicode(utf_8);

	GB_12345 = _font_get_gb12345_code(unicode_16);

	if(GB_12345 == 0)
	{
		memset(dataBuf,0,32);	
    return;		
	}
	I8U MSB=(I8U)(GB_12345 >>8);
	I8U LSB=(I8U)GB_12345;

	if(MSB >=0xA1 && MSB <= 0Xa9 && LSB >=0xA1)
	{
	  addr =((MSB - 0xA1)*94 + (LSB - 0xA1))*32+ BaseAdd;
	}

  else if(MSB >=0xB0 && MSB <= 0Xf9 && LSB >=0xA1)
	{
  	addr = ((MSB - 0xB0)*94 + (LSB - 0xA1)+ 1038)*32+ BaseAdd;
	}
  else
	{
	 	memset(dataBuf,0,32);	
    return;		
	}

	NOR_readData(addr, 32, dataBuf);	
	
  _font_display_conversion(dataBuf);
}


//00 :40 :20 :40 :10 :a0
//get chinese font type
void FONT_init(void)
{    
  I32U addr = FONT_TRADITIONAL_CHINESE_SPACE_START+40960;
	I8U  data[6];
	NOR_readData(addr, 6, data);	
	N_SPRINTF("[FONTS] font init read data :%02x :%02x :%02x :%02x :%02x :%02x",data[0],data[1],data[2],data[3],data[4],data[5]);
	if((data[0]==0x00)&&(data[1]==0x40)&&(data[2]==0x20)&&(data[3]==0x40)&&(data[4]==0x10)&&(data[5]==0xa0))
	cling.font.font_type = FONT_TYPE_TRADITIONAL_CHINESE;
	else
	cling.font.font_type = FONT_TYPE_SIMPLE_CHINESE;	
}


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
//	spi_master_op_wait_done();
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
  N_SPRINTF("[FONTS] in: %d, out: %d, len: 128", in, out);
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
	
	N_SPRINTF("[FONTS] dataO: %02x %02x %02x %02x %02x %02x %02x %02x", 
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
	
	N_SPRINTF("[FONTS] erase all the pages before setup");
	//
	// Note: each page is 4 KB
	while (page_to_erase > 0) {
		if (page_to_erase >= 256) {
			N_SPRINTF("[FS] erase: %d, Whole chip", addr);
			NFLASH_drv_sector_erasure(addr, ERASE_WHOLE_CHIP);
			addr += 1048576;
			page_to_erase -= 256;
		} else if ((page_to_erase >= 16) && ((addr%65536) == 0)) {
			N_SPRINTF("[FS] erase: %d, 64K", addr);
			// we should erase 64 KB block
			NFLASH_drv_sector_erasure(addr, ERASE_64K);
			addr += 65536;
			page_to_erase -= 16;
		} else if ((page_to_erase >= 8) && ((addr%32768) == 0)) {
			N_SPRINTF("[FS] erase: %d, 32K", addr);
			// we should erase 32 KB block
			NFLASH_drv_sector_erasure(addr, ERASE_32K);
			addr += 32768;
			page_to_erase -= 8;
		} else {
			N_SPRINTF("[FS] erase: %d, 4K", addr);
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
	I8U font_data[32];
  I8U string_len;  //The length of the string
  I8U display_len; //The length of the display	
	I8U as_pos = 0;
	I8U ch_pos = 0;	
	I8U ch_to_process;
	I16U p_data_offset;

	string_len = strlen(data);
  display_len=_get_display_len((I8U*)data,height);
	N_SPRINTF("[FONTS] display len :%d ",display_len);	
	p_data_offset = 0;
	while(p_data_offset < string_len)
	{
		ch_to_process = (I8U) data[p_data_offset];
		
	  if((ch_to_process >= 0x20)&&(ch_to_process<=0x7e))
	  {
		  if(height==16)
		  {
					_font_read_one_8x16_ascii(ch_to_process,16,font_data);
					if(b_center==TRUE)
					{
						 memcpy(&ptr[((128-display_len)>>1)+8*as_pos+16*ch_pos],font_data,8);
						 memcpy(&ptr[128+((128-display_len)>>1)+8*as_pos+16*ch_pos],&font_data[8],8);	
					}
					else
					{
						 memcpy(&ptr[8*as_pos+16*ch_pos],font_data,8);
						 memcpy(&ptr[128+8*as_pos+16*ch_pos],&font_data[8],8);						
					}
		  }
		 if(height==8)
		 {
			 //only display 5x7 ascii
			 _font_read_one_5x7_ascii(ch_to_process,8,font_data);
			 if(b_center==TRUE)
			 {
         memcpy(&ptr[((128-display_len)>>1)+6*as_pos],font_data,6); //In the middle			 
			 }
			 else
			 {
         memcpy(&ptr[6*as_pos],font_data,6); //The starting address began to show		 
			 }

		 }
			as_pos+=1;
		 p_data_offset = as_pos+ch_pos*3;
		 continue;
	 }		 
   if((ch_to_process&0x80)==0x80)//chinese characters  Byte highest=1;
	 {
     if(cling.font.font_type == FONT_TYPE_TRADITIONAL_CHINESE)
			 	_font_read_one_traditional_Chinese_characters((I8U*)(data+p_data_offset),32,font_data);
		 else 
			  _font_read_one_simple_Chinese_characters((I8U*)(data+p_data_offset),32,font_data);

		 if(b_center==TRUE)
		 {
		   memcpy(&ptr[((128-display_len)>>1)+8*as_pos+16*ch_pos],font_data,16);
       memcpy(&ptr[128+((128-display_len)>>1)+8*as_pos+16*ch_pos],&font_data[16],16);
		 }
		 else
		 {
		   memcpy(&ptr[8*as_pos+16*ch_pos],font_data,16);
       memcpy(&ptr[128+8*as_pos+16*ch_pos],&font_data[16],16);
		 }
		 ch_pos+=1;
		 p_data_offset = as_pos+ch_pos*3;
		 continue;
	 }
	 
	 break;
	}
	N_SPRINTF("[FONTS] display chinese number: %d,ascii number: %d ",ch_pos,as_pos);
	if(display_len <=128)
	{
		return display_len;
	}
	else 
		return (0xff);
}


