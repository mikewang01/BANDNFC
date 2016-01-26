/***************************************************************************/
/**
 * File: font.c
 *
 * Description: Characters display function.
 *
 ******************************************************************************/

#include "main.h"
#include <stdint.h>


const I8U font_content[] = {
	
0x00,0xf0,0x08,0x04,0x04,0x08,0xf0,0x00,0x00,0x0f,0x10,0x20,0x20,0x10,0x0f,0x00, /*0*/
0x00,0x00,0x10,0x08,0xfc,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3f,0x00,0x00,0x00, /*1*/
0x10,0x08,0x04,0x04,0x04,0x88,0x70,0x00,0x30,0x28,0x24,0x22,0x21,0x20,0x20,0x00, /*2*/
0x08,0x04,0x04,0x84,0x84,0x48,0x30,0x00,0x10,0x20,0x20,0x20,0x20,0x11,0x0e,0x00, /*3*/
0x00,0x80,0x60,0x18,0xc4,0x00,0x00,0x00,0x06,0x05,0x04,0x04,0x3f,0x04,0x04,0x00, /*4*/
0x00,0xfc,0x44,0x44,0x44,0x84,0x04,0x00,0x00,0x10,0x20,0x20,0x20,0x10,0x0f,0x00, /*5*/
0x80,0x60,0x90,0x88,0x84,0x00,0x00,0x00,0x0f,0x11,0x20,0x20,0x20,0x11,0x0e,0x00, /*6*/
0x04,0x04,0x04,0x04,0xc4,0x34,0x0c,0x00,0x00,0x00,0x00,0x3e,0x01,0x00,0x00,0x00, /*7*/
0x30,0x48,0x84,0x84,0x84,0x48,0x30,0x00,0x0e,0x11,0x20,0x20,0x20,0x11,0x0e,0x00, /*8*/
0x70,0x88,0x04,0x04,0x04,0x88,0xf0,0x00,0x00,0x00,0x21,0x11,0x09,0x06,0x01,0x00, /*9*/	
0x3c,0x42,0x42,0xbc,0x60,0x18,0x06,0x00,0x60,0x18,0x06,0x3d,0x42,0x42,0x3c,0x00, /*%*/	
};

/**@brief Function for string encoding conversion.
 * 
 * Description: UTF-8 code(3 B) converted to unicode code (2 B).
 */
static I16U _encode_conversion_pro(I8U *utf_8)
{
	I16U unicode;
	
	unicode  = (utf_8[0] & 0x0F) << 12;
	unicode |= (utf_8[1] & 0x3F) << 6;
	unicode |= (utf_8[2] & 0x3F);
	
	return unicode;
}

/**@brief Function for reading font.
 * 
 * Description: Get one 5x7 size ascii string OLED dot matrix data(8 B).
 */
static void _font_read_one_5x7_ascii(I8U ASCII_code, I16U len, I8U *dataBuf)
{
	I32U addr_in = FONT_ASCII_5X7_SPACE_START;;

	addr_in += ((ASCII_code - 0x20) << 3);
	
	NOR_readData(addr_in, len, dataBuf);	
}

/**@brief Function for reading font.
 * 
 * Description: Get one 8x16 size ascii string OLED dot matrix data(16 B).
 */
static void _font_read_one_8x16_ascii(I8U ASCII_code, I16U len, I8U *dataBuf)
{
	I32U addr_in = FONT_ASCII_8X16_SPACE_START;;

	addr_in += ((ASCII_code - 0x20) << 4);

	NOR_readData(addr_in, len, dataBuf);	
}

/**@brief Function for reading font.
 * 
 * Description: Get one 15x16 size Chinese characters OLED dot matrix data(32 B).
 */
static void _font_read_one_Chinese_characters(I8U *utf_8, I16U len, I8U *dataBuf)
{
	I32U addr_in = FONT_CHINESE_15X16_SPACE_START;
  I16U unicode;	
	I8U  data[16];

	unicode = _encode_conversion_pro(utf_8);

	if(unicode < 0x4E00 || unicode > 0x9FA5)  {
		
    N_SPRINTF("[FONTS] No search to the simple Chinese characters.");	
		
		memset(dataBuf, 0, 32);
		
		// Use succedaneous ascii characters (' " ')in here.
		_font_read_one_8x16_ascii('"',16,data);
		
		memcpy(dataBuf+4,data,8);
		memcpy(dataBuf+20,&data[8],8);	
		
    return;		
	}
	
	addr_in += ((unicode - 0x4E00) << 5);
	NOR_readData(addr_in, len, dataBuf);	
}	

/**@brief Function for get entire display length.
 * 
 * Description: display length, must <= 128.
 *
 * height is 8: only need display 5x7 ascii sting.
 *
 * height is 16: display 8x16 ascii sting and 15x16 chinese characters.
 */
static I8U _font_get_display_len(I8U *data, I8U height)
{
	I8U  s_len,d_len;  
	I8U  a_num=0;
	I8U  c_num=0;
	I8U  ch_to_process;
	I16U s_pos = 0;

	s_len = strlen((char*)data);
	
	while(s_pos < s_len)
	{
		ch_to_process = (I8U)data[s_pos];
		
	  if((ch_to_process >= 0x20)&&(ch_to_process <= 0x7e)){
	    // ASCII code encode format.
		  a_num += 1;
	    s_pos += 1;
	  }
    else if(((data[s_pos]&0xF0) == 0xE0)&&((data[s_pos+1]&0xC0) == 0x80)&&((data[s_pos+2]&0xC0) == 0x80)){
	    // Chinese characters Utf-8 encode format.	
		  c_num += 1;
	    s_pos += 3;
	  }
		else{
			// Is not within the scope of the can display,continue to read the next.
			s_pos++;
		}
	}
	
	if(height == 16)	
		d_len = (a_num << 3) + (c_num << 4);	
	else if(height == 8)
		d_len = (a_num << 3);
	else
		return 0;

	// At present,can only display 128 bytes.	
	if(d_len >= 128)
		d_len = 128;
	
	return d_len;	
}

/**@brief Function for loading characters.
 * 
 * Description: get OLED dot matrix data of a continuous string.
 *
 * height is 8: only need display 5x7 ascii sting.
 *
 * height is 16: display 8x16 ascii sting and 15x16 chinese characters.
 *
 * b_center : display in the middle(TRUE),display from the top(FALSE). 
 */
I8U FONT_load_characters(I8U *ptr, char *data, I8U height, BOOLEAN b_center)
{	
	I8U  font_data[32];
  I8U  s_len,d_len;  
	I8U  a_num = 0;
	I8U  c_num = 0;	
	I8U  ch_to_process;
	I16U s_pos = 0;
  I16U p_data_offset = 0;

	// When OTA is in progress,do not operate nflash.
  if (OTA_if_enabled())
    return (0xff);
	
	s_len  = strlen(data);
	
	// The length of the effective display.
  d_len = _font_get_display_len((I8U*)data,height);
	
	while(s_pos < s_len)
	{
		ch_to_process = (I8U)data[s_pos];
		
		p_data_offset = (a_num << 3) + (c_num << 4);
		
		if(p_data_offset > 128)
			break;
		
	  if((ch_to_process >= 0x20)&&(ch_to_process<=0x7e)){
	  	// ASCII code encode format.
		  if(height==16){
				
				_font_read_one_8x16_ascii(ch_to_process,16,font_data);
				
				if(b_center){
					memcpy(&ptr[((128 - d_len) >> 1) + p_data_offset],font_data,8);
					memcpy(&ptr[128 + ((128 - d_len) >> 1) + p_data_offset],&font_data[8],8);	
				}else{
					memcpy(&ptr[p_data_offset],font_data,8);
					memcpy(&ptr[128 + p_data_offset],&font_data[8],8);						
				}
		  } else {
		 
			  _font_read_one_5x7_ascii(ch_to_process,8,font_data);
				
			  if(b_center)
          memcpy(&ptr[((128 - d_len) >> 1) + 6*a_num],font_data,6);	 
			  else
          memcpy(&ptr[6*a_num],font_data,6);  
		  }
			
			a_num += 1;
		  s_pos += 1;
	  }	
    else if(((data[s_pos]&0xF0) == 0xE0)&&((data[s_pos+1]&0xC0) == 0x80)&&((data[s_pos+2]&0xC0) == 0x80)){
			// Chinese characters Utf-8 encode format.	 
		  _font_read_one_Chinese_characters((I8U*)(data+s_pos),32,font_data);

		  if(b_center){
		    memcpy(&ptr[((128 - d_len) >> 1) + p_data_offset],font_data,16);
        memcpy(&ptr[128+((128 - d_len) >> 1) + p_data_offset],&font_data[16],16);
		  }else{
		    memcpy(&ptr[p_data_offset],font_data,16);
        memcpy(&ptr[128 + p_data_offset],&font_data[16],16);
		  }
		   
		   c_num += 1;
		   s_pos += 3;
	  } else{
			// Is not within the scope of the can display,continue to read the next.
			s_pos++;
		}
	}
	
	N_SPRINTF("[FONTS] display chinese number: %d,ascii number: %d ",ch_pos,as_pos);

	if(d_len <= 128)
		return d_len;
	
	return (0xff);
}

/**@brief Function for loading ota percent characters.
 * 
 * Description: In order to solve the problem of resource conflicts.
 *              When OTA is in progress,do not operate nflash.
 * 1: Height is 16: only need display 8x16 ascii sting.
 * 2: Display in the middle.
 */
void FONT_load_ota_percent(I8U *ptr, I8U percent)
{
  I8U	ten,single;
	I8U pos_1,pos_2;
	
	if(percent >= 99)
		percent = 99;
	
	if(percent <= 9){
		pos_1 = percent*16;
		memcpy(&ptr[52],&font_content[pos_1],8);
		memcpy(&ptr[180],&font_content[pos_1+8],8);
		memset(&ptr[60],0,8);
		memset(&ptr[188],0,8);
		memcpy(&ptr[68],&font_content[160],8);
		memcpy(&ptr[196],&font_content[168],8);	
	}else{
		ten = percent/10;
		single = percent%10;
		pos_1 = ten*16; 
		pos_2 = single*16; 
		memcpy(&ptr[48],&font_content[pos_1],8);
		memcpy(&ptr[176],&font_content[pos_1+8],8);	
		memcpy(&ptr[56],&font_content[pos_2],8);
		memcpy(&ptr[184],&font_content[pos_2+8],8);		
		memset(&ptr[64],0,8);
		memset(&ptr[192],0,8);	
		memcpy(&ptr[72],&font_content[160],8);
		memcpy(&ptr[200],&font_content[168],8);				
	}	
}
