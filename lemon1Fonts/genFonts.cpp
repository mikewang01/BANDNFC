// genFonts.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "stdio.h"
#include "stdlib.h"

// Cling Device Assets
//#define CLING_DISPLAY_SEC_IMAGE "d:\\my_cplusplus\\fonts\\test_images\\bmp_2015_04_20\\"

// Lemon Device Assets
#define LEMON_DISPLAY_SEC_IMAGE "BMP\\"

#define DEVICE_DISPLAY_SEC_IMAGE LEMON_DISPLAY_SEC_IMAGE

#define ICON_NUM 23
#define ASCII_NUM 128
#define CLING_ICON_NUM 256

int getWH(int *w, int *h, const char *fileName)
{

	char bmpName[128];                                  // bmp file name  
	FILE *fp = NULL;                                          // FILE pointer  
	unsigned char header[54];                       // for bmp header look ahead  
	int temp = 0, width = 0, height = 0;     // local variable for calculation  
	int i = 0;                                                          // loop variable  
	sprintf(bmpName, "%s.bmp", fileName);// 24bit bit map 未使用調色盤  

	if (!(fp = fopen(bmpName, "rb")))
		return -1;

	fread(header, sizeof(unsigned char), 54, fp);  // read bmp header into header[54]  

	for (i = 0; i != 4; ++i)                                         // get width from byte 18 ~ byte 22 in header[54]   
	{
		temp = (long)header[18 + i];
		temp = temp << (8 * i);
		width += temp;
	}
	for (i = 0; i != 4; ++i)                                         // get width from byte 22 ~ byte 26 in header[54]   
	{
		temp = (long)header[22 + i];
		temp = temp << (8 * i);
		height += temp;
	}
	*w = width;
	*h = height;
	fclose(fp);

	return 0;
}//end if getWT  

int bmp_read(unsigned char *image, int xSize, int ySize, const char *fileName)
{
	char fnameBmp[128] = { 0 };
	FILE *fp = NULL;
	unsigned char header[54] = { 0 };   // for bmp header  
	unsigned char unknown[8] = { 0 };

	sprintf(fnameBmp, "%s.bmp", fileName);

	if (!(fp = fopen(fnameBmp, "rb")))
		return -1;

	fread(header, sizeof(unsigned char), 54, fp);  // read bmp header into header[54], based on byte  

	fread(unknown, sizeof(unsigned char), 8, fp);  // read bmp header into header[54], based on byte  

	fread(image, sizeof(unsigned char), (size_t)(long)xSize * ySize * 3, fp);  // read image array, based on byte  

	fclose(fp);
	return 0;
}//end of function bmp read  


int bmp_write(unsigned char* image, int xSize, int ySize, char *fileName)
{
	unsigned char header[54] = {
		0x42, 0x4d, 0, 0, 0, 0, 0, 0, 0, 0,        // 0~1 st byte : 'BM'  
		54, 0, 0, 0, 40, 0, 0, 0, 0, 0,       // 10th byte : offset = 54 = header size  
		0, 0, 0, 0, 0, 0, 1, 0, 24, 0,     // 26th byte : 圖層數，28th byte : 24 bit 點陣圖  
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0
	};

	// sizeof(long) = 4  
	long fileSize = (long)xSize * (long)ySize * 3 + 54;     // image array size + 54  
	long temp = 0;                          // temp for byte convertion  
	long width = xSize;                 // image width  
	long height = ySize;                // image height  
	char bmpName[128] = { 0 };    // filename   
	int i = 0;                                      // loop variable  

	FILE *fp;

	for (i = 0; i != 4; ++i)                // write fileSize from byte2 ~ byte 6  
	{
		header[2 + i] = (unsigned char)(fileSize >> (8 * i)) & 0x000000ff;
	}
	for (i = 0; i != 4; ++i)                // write width from byte18 ~ byte 22  
	{
		header[18 + i] = (unsigned char)(width >> (8 * i)) & 0x000000ff;
	}
	for (i = 0; i != 4; ++i)                // write height from byte22 ~ byte 26  
	{
		header[22 + i] = (unsigned char)(height >> (8 * i)) & 0x000000ff;
	}

	sprintf(bmpName, "%s.bmp", fileName);

	if (!(fp = fopen(bmpName, "wb")))                                   // 檔案名稱  
		return -1;

	fwrite(header, sizeof(unsigned char), 54, fp);                                              // write header  
	fwrite(image, sizeof(unsigned char), (size_t)(long)xSize*ySize * 3, fp);       // write image array  

	fclose(fp);
	return 0;
}//end of function bmp_write  

static void _gen_icon_array(unsigned char * imgin, unsigned char *out, int x, int y, int icon_idx, char *filename, const char *array_name, int row_num)
{
	int i, j, k, k_pos;
	int dw_width = (x + 31) >> 5;
	int width = dw_width << 2;
	int height = y;
	unsigned char *pout;
	char fout_name[1024];
	unsigned char imgout[5000];
	FILE *fp = NULL;
	int round = row_num;

	for (i = 0; i < x*y; i++)
		out[i] = 0;

	for (i = 0; i < y; i++) {
		for (j = 0; j < width; j++) {
			imgout[(y - 1 - i)*width + j] = imgin[i*width + j] ^ 0xff;
		}
	}

	k_pos = 0;

	for (round = 0; round < row_num; round++) {

		// First row
		for (j = 0; j < 8; j++) {
			pout = out + round;
			for (k = 0; k < width; k++) {
				for (i = 0; i < 8; i++) {
					*pout |= imgout[k_pos] & 0x80;
					if (j != 7)
						*pout >>= 1;
					imgout[k_pos] <<= 1;
					pout += row_num;
				}
				k_pos++;
			}
		}
	}
	/*
	// First row
	for (j = 0; j < 8; j++) {
		pout = out;
		for (k = 0; k < width; k++) {
			for (i = 0; i < 8; i++) {
				*pout |= imgout[k_pos] & 0x80;
				if (j != 7)
					*pout >>= 1;
				imgout[k_pos] <<= 1;
				pout += 4;
			}
			k_pos++;
		}
	}
	// Second row
	for (j = 0; j < 8; j++) {
		pout = out + 1;
		for (k = 0; k < width; k++) {
			for (i = 0; i < 8; i++) {
				*pout |= imgout[k_pos] & 0x80;
				if (j != 7)
					*pout >>= 1;
				imgout[k_pos] <<= 1;
				pout += 4;
			}
			k_pos++;
		}
	}

	// third row
	for (j = 0; j < 8; j++) {
		pout = out + 2;
		for (k = 0; k < width; k++) {
			for (i = 0; i < 8; i++) {
				*pout |= imgout[k_pos] & 0x80;
				if (j != 7)
					*pout >>= 1;
				imgout[k_pos] <<= 1;
				pout += 4;
			}
			k_pos++;
		}
	}

	// 4th row
	for (j = 0; j < 8; j++) {
		pout = out + 3;
		for (k = 0; k < width; k++) {
			for (i = 0; i < 8; i++) {
				*pout |= imgout[k_pos] & 0x80;
				if (j != 7)
					*pout >>= 1;
				imgout[k_pos] <<= 1;
				pout += 4;
			}
			k_pos++;
		}
	}
	*/
	sprintf(fout_name, "%s.h", filename);

	if ((fp = fopen(fout_name, "wb")) == NULL)
		return;
	fprintf(fp, "#define %s_%02d_LEN %d\n\n", array_name, icon_idx, x);
	fprintf(fp, "const I8U %s_%02d[%d] = {\n", array_name, icon_idx, x * row_num);

	for (i = 0; i < x * row_num; i++) {
		fprintf(fp, "%d,", out[i]);

		if (i && i % 8 == 0) {
			fprintf(fp, "\n");
		}
	}
	fprintf(fp, "\n};\n");

	fclose(fp);



}


void _gen_sec_0_icon(int &acc_len, int &lenLen, int &lenPos, int &lenTab, char *pLen, char *pPos, char *pTab)
{
	char filename[1024];
	char image[10000];
	unsigned char out[10000];
	int x, y, i, j;
	FILE *fp = NULL;                                          // FILE pointer  

	for (i = 0; i < CLING_ICON_NUM; i++) {
		sprintf(filename, "%ssec_0_%02d", DEVICE_DISPLAY_SEC_IMAGE, i);

		if (getWH(&x, &y, filename) != -1) {

			bmp_read((unsigned char *)image, x, y, filename);

			_gen_icon_array((unsigned char *)image, out, x, y, i, filename, "SEC_0", 2);

			lenLen += sprintf(pLen + lenLen, "%d,", x);
			lenPos += sprintf(pPos + lenPos, "%d,", acc_len);
			acc_len += x * 2;

			for (j = 0; j < x * 2; j++) {
				lenTab += sprintf(pTab + lenTab, "%d,", out[j]);

				if (j && j % 8 == 0) {
					lenTab += sprintf(pTab + lenTab, "\n");
				}
			}
		}
		else {
			lenLen += sprintf(pLen + lenLen, "0,");
			lenPos += sprintf(pPos + lenPos, "0,");
		}
		if (i && i % 8 == 0) {
			lenLen += sprintf(pLen + lenLen, "\n");
			lenPos += sprintf(pPos + lenPos, "\n");
		}
	}
}

void _gen_sec_2_icon(int &acc_len, int &lenLen, int &lenPos, int &lenTab, char *pLen, char *pPos, char *pTab)
{
	char filename[1024];
	char image[10000];
	unsigned char out[10000];
	int x, y, i, j;
	FILE *fp = NULL;                                          // FILE pointer  

	for (i = 0; i < CLING_ICON_NUM; i++) {
		sprintf(filename, "%ssec_2_%02d", DEVICE_DISPLAY_SEC_IMAGE, i);

		if (getWH(&x, &y, filename) != -1) {

			bmp_read((unsigned char *)image, x, y, filename);

			_gen_icon_array((unsigned char *)image, out, x, y, i, filename, "SEC_2", 2);

			lenLen += sprintf(pLen + lenLen, "%d,", x);
			lenPos += sprintf(pPos + lenPos, "%d,", acc_len);
			acc_len += x * 2;

			for (j = 0; j < x * 2; j++) {
				lenTab += sprintf(pTab + lenTab, "%d,", out[j]);

				if (j && j % 8 == 0) {
					lenTab += sprintf(pTab + lenTab, "\n");
				}
			}
		}
		else {
			lenLen += sprintf(pLen + lenLen, "0,");
			lenPos += sprintf(pPos + lenPos, "0,");
		}
		if (i && i % 8 == 0) {
			lenLen += sprintf(pLen + lenLen, "\n");
			lenPos += sprintf(pPos + lenPos, "\n");
		}
	}
}

void _gen_sec_3_icon(int &acc_len, int &lenLen, int &lenPos, int &lenTab, char *pLen, char *pPos, char *pTab)
{
	char filename[1024];
	char image[10000];
	unsigned char out[10000];
	int x, y, i, j;
	FILE *fp = NULL;                                          // FILE pointer  

	for (i = 0; i < CLING_ICON_NUM; i++) {
		sprintf(filename, "%ssec_3_%02d", DEVICE_DISPLAY_SEC_IMAGE, i);

		if (getWH(&x, &y, filename) != -1) {

			bmp_read((unsigned char *)image, x, y, filename);

			_gen_icon_array((unsigned char *)image, out, x, y, i, filename, "SEC_3", 3);

			lenLen += sprintf(pLen + lenLen, "%d,", x);
			lenPos += sprintf(pPos + lenPos, "%d,", acc_len);
			acc_len += x * 3;

			for (j = 0; j < x * 3; j++) {
				lenTab += sprintf(pTab+lenTab, "%d,", out[j]);

				if (j && j % 8 == 0) {
					lenTab += sprintf(pTab+lenTab, "\n");
				}
			}
		}
		else {
			lenLen += sprintf(pLen+lenLen, "0,");
			lenPos += sprintf(pPos+lenPos, "0,");
		}
		if (i && i % 8 == 0) {
			lenLen += sprintf(pLen+lenLen, "\n");
			lenPos += sprintf(pPos+lenPos, "\n");
		}
	}
}

void _gen_sec_1_icon(int &acc_len, int &lenLen, int &lenPos, int &lenTab, char *pLen, char *pPos, char *pTab)
{
	char filename[1024];
	char image[10000];
	unsigned char out[10000];
	int x, y, i, j;
	FILE *fp = NULL;                                          // FILE pointer  

	for (i = 0; i < CLING_ICON_NUM; i++) {
		sprintf(filename, "%ssec_1_%02d", DEVICE_DISPLAY_SEC_IMAGE, i);

		if (getWH(&x, &y, filename) != -1) {

			bmp_read((unsigned char *)image, x, y, filename);

			_gen_icon_array((unsigned char *)image, out, x, y, i, filename, "SEC_1", 1);

			lenLen += sprintf(pLen + lenLen, "%d,", x);
			lenPos += sprintf(pPos + lenPos, "%d,", acc_len);
			acc_len += x;

			for (j = 0; j < x * 1; j++) {
				lenTab += sprintf(pTab + lenTab, "%d,", out[j]);

				if (j && j % 8 == 0) {
					lenTab += sprintf(pTab + lenTab, "\n");
				}
			}
		}
		else {
			lenLen += sprintf(pLen + lenLen, "0,");
			lenPos += sprintf(pPos + lenPos, "0,");
		}
		if (i && i % 8 == 0) {
			lenLen += sprintf(pLen + lenLen, "\n");
			lenPos += sprintf(pPos + lenPos, "\n");
		}
	}
}

void _gen_sec_4_icon(int &acc_len, int &lenLen, int &lenPos, int &lenTab, char *pLen, char *pPos, char *pTab)
{
	char filename[1024];
	char image[10000];
	unsigned char out[10000];
	int x, y, i, j;
	FILE *fp = NULL;                                          // FILE pointer  

	for (i = 0; i < CLING_ICON_NUM; i++) {
		sprintf(filename, "%ssec_4_%02d", DEVICE_DISPLAY_SEC_IMAGE, i);

		if (getWH(&x, &y, filename) != -1) {

			bmp_read((unsigned char *)image, x, y, filename);

			_gen_icon_array((unsigned char *)image, out, x, y, i, filename, "SEC_4", 4);

			lenLen += sprintf(pLen + lenLen, "%d,", x);
			lenPos += sprintf(pPos + lenPos, "%d,", acc_len);
			acc_len += x * 4;

			for (j = 0; j < x * 4; j++) {
				lenTab += sprintf(pTab + lenTab, "%d,", out[j]);

				if (j && j % 8 == 0) {
					lenTab += sprintf(pTab + lenTab, "\n");
				}
			}
		}
		else {
			lenLen += sprintf(pLen + lenLen, "0,");
			lenPos += sprintf(pPos + lenPos, "0,");
		}
		if (i && i % 8 == 0) {
			lenLen += sprintf(pLen + lenLen, "\n");
			lenPos += sprintf(pPos + lenPos, "\n");
		}
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	char *tab_len = (char *)calloc(10240, sizeof(char));
	char *tab_ptr = (char *)calloc(10240, sizeof(char));
	char *tab_content = (char *)calloc(100000, sizeof(char));
	int lenTab = 0;
	int lenLen = 0;
	int lenPos = 0;
	int acc_len = 0;
	char fout_name[128];
	FILE *fp;

#if 0
	_gen_icons();

	_gen_small_ascii();

	_gen_large_ascii();
#endif
//	_gen_sec_0_icon(acc_len, lenLen, lenPos, lenTab, tab_len, tab_ptr, tab_content);
	_gen_sec_1_icon(acc_len, lenLen, lenPos, lenTab, tab_len, tab_ptr, tab_content);
	_gen_sec_2_icon(acc_len, lenLen, lenPos, lenTab, tab_len, tab_ptr, tab_content);
	_gen_sec_3_icon(acc_len, lenLen, lenPos, lenTab, tab_len, tab_ptr, tab_content);
	_gen_sec_4_icon(acc_len, lenLen, lenPos, lenTab, tab_len, tab_ptr, tab_content);
	tab_len[lenLen] = 0;
	tab_ptr[lenPos] = 0;
	tab_content[lenTab] = 0;

	sprintf(fout_name, "%sasset_len.h", DEVICE_DISPLAY_SEC_IMAGE);
	if ((fp = fopen(fout_name, "wb")) == NULL)
		return 0;
	fprintf(fp, "const I8U asset_len[] = {\n");
	fprintf(fp, "%s0};\n", tab_len);
	fclose(fp);

	sprintf(fout_name, "%sasset_pos.h", DEVICE_DISPLAY_SEC_IMAGE);
	if ((fp = fopen(fout_name, "wb")) == NULL)
		return 0;
	fprintf(fp, "const I16U asset_pos[] = {\n");
	fprintf(fp, "%s0};\n", tab_ptr);
	fclose(fp);

	sprintf(fout_name, "%sasset_content.h", DEVICE_DISPLAY_SEC_IMAGE);
	if ((fp = fopen(fout_name, "wb")) == NULL)
		return 0;
	fprintf(fp, "const I8U asset_content[] = {\n");
	fprintf(fp, "%s0};\n", tab_content);
	fclose(fp);

	free(tab_len);
	free(tab_ptr);

	return 0;
}


