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

#ifndef u_char
#define u_char unsigned char
#endif

typedef struct {
    unsigned char r;        // Red
    unsigned char g;        // Green
    unsigned char b;        // Blue
    unsigned char a;        // Alpha
} color_t;

typedef struct
{
    int width;         // 幅
    int height;        // 高さ
    unsigned short color_type;  // 色表現の種別
    unsigned short palette_num; // カラーパレットの数
    color_t *palette;           // カラーパレットへのポインタ
    unsigned char ** map;       // 画像データ
} IMAGEDATA;

typedef struct
{
    color_t palette[256];       // カラーパレットへのポインタ
    u_char raw[4];              // 画像データ
} IMGBUF, *pIMGBUF;


unsigned char** alloc_map(IMAGEDATA* img);
void free_map(IMAGEDATA* img);


int writepng(const char* filename, IMAGEDATA* img);
int write_png_stream(FILE* fp, IMAGEDATA* img);


pIMGBUF pngptr2img(u_char *pptr);
pIMGBUF PngOpenFile(const char* szFile);

//////////////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _PNGCTRL_H_
