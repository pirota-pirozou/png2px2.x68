// pngctrl.h
//

#pragma once

#ifndef _PNGCTRL_H_
#define _PNGCTRL_H_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

//////////////////////////////////////////////////////////////////////////////////////////

#define COLOR_TYPE_INDEX 0   // �C���f�b�N�X�J���[

typedef unsigned char u_char;

typedef unsigned int   LONG;
typedef unsigned int   DWORD;
typedef unsigned short WORD;
typedef unsigned char  BYTE;

typedef struct _tRGBQUAD {
    BYTE rgbBlue;
    BYTE rgbGreen;
    BYTE rgbRed;
    BYTE rgbReserved;
} RGBQUAD;

#define BI_RGB        0
#define BI_BITFIELDS  3

typedef struct _tBITMAPINFOHEADER {
    DWORD biSize;
    LONG  biWidth;
    LONG  biHeight;
    WORD  biPlanes;
    WORD  biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    LONG  biXPelsPerMeter;
    LONG  biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
} BITMAPINFOHEADER, * LPBITMAPINFOHEADER, * PBITMAPINFOHEADER;

typedef LPBITMAPINFOHEADER		PDIB;

typedef struct _tBITMAPINFO {
    BITMAPINFOHEADER bmiHeader;
    RGBQUAD          bmiColors[1];
} BITMAPINFO, * LPBITMAPINFO, * PBITMAPINFO;

typedef struct {
    unsigned char r;        // Red
    unsigned char g;        // Green
    unsigned char b;        // Blue
    unsigned char a;        // Alpha
} color_t;

typedef struct
{
    int width;                  // ��
    int height;                 // ����
    unsigned short color_type;  // �F�\���̎��
    unsigned short palette_num; // �J���[�p���b�g�̐�
    color_t *palette;           // �J���[�p���b�g�ւ̃|�C���^
    png_bytepp map;             // �摜�f�[�^
} IMAGEDATA;

u_char** alloc_map(IMAGEDATA* img);
void free_map(IMAGEDATA* img);

int writepng(const char* filename, IMAGEDATA* img);
int write_png_stream(FILE* fp, IMAGEDATA* img);

PDIB pngptr2dib(u_char* pptr);

PDIB PngOpenFile(const char* szFile);

//////////////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _PNGCTRL_H_
