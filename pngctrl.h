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
    int width;         // ��
    int height;        // ����
    unsigned short color_type;  // �F�\���̎��
    unsigned short palette_num; // �J���[�p���b�g�̐�
    color_t *palette;           // �J���[�p���b�g�ւ̃|�C���^
    unsigned char ** map;       // �摜�f�[�^
} IMAGEDATA;

typedef struct
{
    color_t palette[256];       // �J���[�p���b�g�ւ̃|�C���^
    u_char raw[4];              // �摜�f�[�^
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
