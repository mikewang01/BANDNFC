/***************************************************************************/
/**
 * File: font.c
 *
 * Description: Characters display function.
 *
 ******************************************************************************/

#include "main.h"
#include <stdint.h>
#include "font.h"


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
I8U FONT_load_characters(I8U *pin_in, char *string, I8U height, I8U horizontal_len, BOOLEAN b_center)
{	
	I8U  font_data[32];
	I8U  *p0, *p1, *p2, *p3;	
 	I8U  pos=0, offset=0 ,curr_line_pos=0;  	
	I8U  ptr, i; 	
	I8U  line_len = 0;

	p0 = pin_in;	
	
	// When OTA is in progress,do not operate nflash.
  if (OTA_if_enabled())
    return line_len;
	
  // At present,can only display 128 bytes size string.		
	while(pos < 128)
	{
		if ((string[pos] == ' ') && (string[pos+1] == '\0')) {
			// Remove last space (' ') character
			break;
		}
		
		if (string[pos] == '\0') {
		  break;
		}
		
	  if((string[pos] >= 0x20) && (string[pos] <= 0x7e)) {	
      // ASCII code encode format.
			if (height == 8) {
			  _font_read_one_5x7_ascii(string[pos], 6, font_data);	
        memcpy(p0 + offset, font_data, 6);  	
				offset += 6;
				line_len += 6;
		  } else if (height == 16) {	
				if(offset > (horizontal_len - 8)){
				  b_center = FALSE;
					curr_line_pos++;
					if(curr_line_pos >= 2)	
            return line_len;
          p0 += 256;	
          offset = 0;					
		    }					
				_font_read_one_8x16_ascii(string[pos], 16, font_data);
				memcpy(p0 + offset, font_data, 8);
			  memcpy(p0 + offset + 128, &font_data[8], 8);						
				offset += 8;
				line_len += 8;
		  } else 	{
			}
			
		  pos += 1;
	  } else if (((string[pos]&0xF0) == 0xE0) && ((string[pos+1]&0xC0) == 0x80) && ((string[pos+2]&0xC0) == 0x80)) {
			// Chinese characters Utf-8 encode format.	
			if(offset > (horizontal_len - 16)){
				b_center = FALSE;
				curr_line_pos++;
				if(curr_line_pos >= 2)	
          return line_len;
				p0 += 256;	
				offset = 0;					
			}
		  _font_read_one_Chinese_characters((I8U *)(string+pos), 32, font_data);
		  memcpy(p0 + offset, font_data, 16);
      memcpy(p0 + offset + 128, &font_data[16], 16);
			offset += 16;
		  pos += 3;
			line_len += 16;
	  } else {
			// Is not within the scope of the can display,continue to read the next.
			pos++;
		}
	}
	
	// Shift all the display to the middle
	if (b_center) {
		p0 = pin_in;
		p1 = p0+128;
		ptr = (128 - offset)>>1;
		
		if (ptr > 0) {
			if (height == 8) {
				p0 += 127; p2 = p0 - ptr;
			  for (i = 0; i < 128-ptr; i++) {
				  *p0-- = *p2--;
			  }
			  for (; i < 128; i++) {
				  *p0-- = 0;
			  }					
			} else {
		   	p0 += 127; p2 = p0 - ptr;
			  p1 += 127; p3 = p1 - ptr;
			  for (i = 0; i < 128-ptr; i++) {
				  *p0-- = *p2--;
				  *p1-- = *p3--;
			  }
			  for (; i < 128; i++) {
				  *p0-- = 0;
				  *p1-- = 0;
			  }				
			}
		}
	}	
	
	return line_len;
}

I8U FONT_get_string_display_depth(char *string)
{
	I8U  curr_offset=0, curr_line_pos=0;
	I8U  pos=0;
  I8U  sting_index=0;
	
	while(pos < 128)
	{
		if ((string[pos] == ' ') && (string[pos+1] == '\0')) {
			// Remove last space (' ') character
			break;
		}		
		
		if(string[pos] == '\0')
		  break;

		if((string[pos] >= 0x20)&&(string[pos] <= 0x7e)) {
		  // ASCII code encode format.
			if(curr_offset >= 112) {
				curr_offset = 0;
				curr_line_pos++;
				if ((curr_line_pos%2) == 0) {
			    cling.ui.notific.string_pos_buf[sting_index++] = pos;
		    }					
		  }	
      curr_offset += 8;
			pos += 1;
		} else if (((string[pos]&0xF0) == 0xE0)&&((string[pos+1]&0xC0) == 0x80)&&((string[pos+2]&0xC0) == 0x80)) {
		  // Chinese characters Utf-8 encode format.
			if(curr_offset >= 104) {
				curr_offset = 0;
				curr_line_pos++;
				if ((curr_line_pos%2) == 0) {
			    cling.ui.notific.string_pos_buf[sting_index++] = pos;
		    }	
		  }	
			curr_offset += 16;
			pos += 3;
		}
		else {
			// Is not within the scope of the can display,continue to read the next.
			pos++;		
		}		
	}
	
	if (curr_offset > 0)
		curr_line_pos ++;
	
	return ((curr_line_pos+1)>>1);	
}

I16U FONT_get_string_display_len(char *string)
{
	I16U  offset=0;
	I8U   pos=0;
	
	while(pos < 128)
	{
		if ((string[pos] == ' ') && (string[pos+1] == '\0')) {
			// Remove last space (' ') character
			break;
		}
		
		if (string[pos] == '\0')
		  break;

		if ((string[pos] >= 0x20)&&(string[pos] <= 0x7e)) {
		  // ASCII code encode format.
      offset += 8;
			pos += 1;
		} else if (((string[pos]&0xF0) == 0xE0)&&((string[pos+1]&0xC0) == 0x80)&&((string[pos+2]&0xC0) == 0x80)) {
		  // Chinese characters Utf-8 encode format.
			offset += 16;
			pos += 3;
		}
		else {
			// Is not within the scope of the can display,continue to read the next.
			pos++;		
		}
		
		// leave at least one character at the end
		if (offset >= 240)
			break;
	}
	N_SPRINTF("FONT string: %d, %d", pos, offset);
	return offset;
}
