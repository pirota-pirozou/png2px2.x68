// pngctrl.h
//

#pragma once

#ifndef _PNGCTRL_H_
#define _PNGCTRL_H_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

//////////////////////////////////////////////////////////////////////////////////////////

#define COLOR_TYPE_INDEX 0   // インデックスカラー

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
    int width;                  // 幅
    int height;                 // 高さ
    unsigned short color_type;  // 色表現の種別
    unsigned short palette_num; // カラーパレットの数
    color_t *palette;           // カラーパレットへのポインタ
    png_bytepp map;             // 画像データ
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
