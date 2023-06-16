/////////////////////////////////////////////////////////////////////////////
// PNG2PX2.c
//
// PNG to X68 Sprite Pattern Converter
// programmed by pirota
/////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __NEWLIB__
#include <fcntl.h>
#include <unistd.h>
#endif

#include "png.h"
#include "pngctrl.h"

#define INFILE_EXT  ".png"
#define OUTFILE_EXT ".PX2"
#define OUTFILEPAT_EXT ".PAT"
#define OUTFILEPAL_EXT ".PAL"

#define _MAX_DRIVE	3
#define _MAX_DIR	256
#define _MAX_FNAME	256
#define _MAX_EXT	256

// PX2�t�@�C���\����
typedef struct
{
	unsigned short atr[256];										// �c�[���p�A�g���r���[�g
	unsigned short pal[256];										// X68K�p���b�g
	unsigned char  sprpat[0x8000];									// X68K�X�v���C�g�p�^�[��

} PX2FILE, * pPX2FILE;

static pPX2FILE px2buf = NULL;									// PX2�o�b�t�@
PDIB dibbuf = NULL;

static char infilename[256];
static char outfilename[256];
static char patfilename[256];
static char palfilename[256];

static int main_result = 0;

static int opt_p = 0;												// �ׂ������I�v�V����

static int readjob(void);
static int cvjob(void);

// �g�p�@�̕\��
static void usage(void)
{
    printf("usage: PNG2PX2 infile[" INFILE_EXT "] OutFile\n"\
		   "\t-p\t�X�v���C�g�p�^�[��(PAT)/�p���b�g(PAL)�x�^�o��\n"
		   );

    exit(0);
}


/// @brief basename
/// @param path
/// @return
char* basename(char *path)
{
	char *p;
	if( path == NULL || *path == '\0' )
		return ".";
	p = path + strlen(path) - 1;
	while( *p == '/' ) {
		if( p == path )
			return path;
		*p-- = '\0';
	}
	while( p >= path && *p != '/' )
		p--;
	return p + 1;
}

/// @brief dirname
/// @param path
/// @return
char* dirname(char *path)
{
	char *p;
	if( path == NULL || *path == '\0' )
		return ".";
	p = path + strlen(path) - 1;
	while( *p == '/' ) {
		if( p == path )
			return path;
		*p-- = '\0';
	}
	while( p >= path && *p != '/' )
		p--;
	if( p < path )
		return ".";
	*p = '\0';
	return path;
}

/// @brief �p�X���𕪉�����
/// @param path
/// @param drive
/// @param dir
/// @param name
/// @param ext
void splitpath(const char *path, char *drive, char *dir, char *name, char *ext)
{
    char *basec, *bname, *dirc, *dname;
    char *lastdot;
    char *extc;

    /* Copy path */
#ifdef _MSC_VER
    basec = _strdup(path);
    dirc = _strdup(path);
    extc = _strdup(path);
#else
	basec = strdup(path);
	dirc = strdup(path);
	extc = strdup(path);
#endif

    /* Get file name and directory */
    bname = basename(basec);
    dname = dirname(dirc);

    /* Get the last dot position */
    lastdot = strrchr(bname, '.');

    /* Split the file name and extension */
    if (lastdot != NULL) {
        *lastdot = '\0';
        strcpy(ext, lastdot + 1);
    } else {
        ext[0] = '\0';
    }

    strcpy(name, bname);
    strcpy(dir, dname);
    drive[0] = '\0';

    free(basec);
    free(dirc);
    free(extc);
}

/////////////////////////////////////
// main
/////////////////////////////////////
int main(int argc, char *argv[])
{
	int i;
	int ch;

	char drive[_MAX_DRIVE] = { 0 };
	char dir[_MAX_DIR] = { 0 };
	char fname[_MAX_FNAME] = { 0 };
	char ext[_MAX_EXT] = { 0 };

#ifdef ALLMEM
	allmem();
#endif

	// �R�}���h���C�����
	memset(fname, 0, sizeof(fname));
	memset(infilename, 0, sizeof(infilename));
	memset(outfilename, 0, sizeof(outfilename) );
	memset(patfilename, 0, sizeof(patfilename) );
	memset(palfilename, 0, sizeof(palfilename) );

    printf("PNG to PX2 Converter Ver0.12a " __DATE__ "," __TIME__ " Programmed by Pirota\n");

    if (argc <= 1)
	{
		usage();
	}


	for (i=1; i<argc; i++)
	{
		ch = argv[i][0];
		if (ch == '-' || ch == '/')
		{
			// �X�C�b�`
			switch (argv[i][1])
			{
			case 'p':
				opt_p = 1;
				break;
			default:
				printf("-%c �I�v�V�������Ⴂ�܂��B\n",argv[i][1]);
				break;
			}

			continue;
		}
		// �t�@�C��������
		if (!infilename[0])
		{
			strcpy(infilename, argv[i]);
			splitpath(infilename , drive, dir, fname, ext );
			if (ext[0]==0)
				strcat(infilename, INFILE_EXT);							// �g���q�⊮

			continue;
		}

		// �o�̓t�@�C�����̍쐬
		if (!outfilename[0])
		{
			// �o�̓t�@�C���l�[��
			strcpy(outfilename, argv[i]);
		}

	}
	// �o�̓t�@�C�������ȗ�����Ă���
	if (!outfilename[0])
	{
		// �o�̓t�@�C���l�[��
		strcpy(outfilename, fname);
	}

	splitpath(outfilename, drive, dir, fname, ext);
	if (ext[0] == 0)
		strcat(outfilename, OUTFILE_EXT);							// �g���q�⊮

	// �ׂ������t�@�C���l�[��
	splitpath(outfilename , drive, dir, fname, ext );
	strcpy(patfilename, fname);
	strcat(patfilename, OUTFILEPAT_EXT);							// �g���q�⊮

	strcpy(palfilename, fname);
	strcat(palfilename, OUTFILEPAL_EXT);							// �g���q�⊮

    // �t�@�C���ǂݍ��ݏ���
    if (readjob()<0)
		goto cvEnd;


	// �o�̓o�b�t�@�̊m��
	px2buf = (pPX2FILE) malloc(sizeof(PX2FILE));
	if (px2buf == NULL)
	{
		printf("�o�̓o�b�t�@�͊m�ۂł��܂���\n");
		goto cvEnd;
	}
	memset(px2buf, 0, sizeof(PX2FILE));

    // �ϊ�����
	if (cvjob() < 0)
	{
		goto cvEnd;
	}

cvEnd:
	// ��n��
	// PX2�o�̓o�b�t�@�J��
	if (px2buf != NULL)
	{
		free(px2buf);
	}

	// �I�t�X�N���[���J��
	if (dibbuf != NULL)
	{
		free(dibbuf);
	}

	return main_result;
}

//----------------------------
// �o�m�f�ǂݍ��݂c�h�a�ɕϊ�
//----------------------------
// Out: 0=�n�j
//      -1 = �G���[
static int readjob(void)
{
	dibbuf = PngOpenFile((const char*)infilename);
	if (dibbuf == NULL)
	{
		printf("Can't open '%s'.\n", infilename);
		return -1;
	}

	// �e���|�����ɏo��
	// * debug *
/*
	FILE* fp = fopen("img_ptr.bin", "wb");
	fwrite(dibbuf, 1, sizeof(IMGBUF)+32768-4, fp);
	fclose(fp);
 */

	return 0;
}


///////////////////////////////
// �p�^�[���f�[�^�ɕϊ�����
///////////////////////////////
static int cvjob(void)
{
	BITMAPINFOHEADER* bi;
	size_t a;
	u_char* pimg;
	u_char* outptr;
	u_char* atrptr;
	FILE* fp;
	int x, y;
	int xl, yl;
	RGBQUAD* paltmp;
	WORD x68pal;
	u_char dot2;
	int err = 0;
	RGBQUAD* dibpal;

	bi = (BITMAPINFOHEADER*)dibbuf;

	// �p�^�[���ϊ�����
	outptr = px2buf->sprpat;										// �X�v���C�g�p�^�[���o�̓o�b�t�@
	atrptr = (u_char*)px2buf + 1;									// �A�g���r���[�g�o�̓o�b�t�@

	pimg = NULL;
	for (yl = 0; yl < bi->biHeight; yl += 16)
	{
		for (xl = 0; xl < bi->biWidth; xl += 8)
		{
			if ((xl & 15) == 0)
			{
				// �A�g���r���[�g��������
				pimg = (u_char*)dibbuf + (sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * bi->biClrUsed) + (yl * bi->biWidth) + xl;
				*atrptr = (((*pimg) & 0xF0) >> 4); // �G���f�B�A���l��
				atrptr += 2;
			}
			// �s�N�Z���ϊ�
			for (y = 0; y < 16; y++)
			{
				pimg = (u_char*)dibbuf + (sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * bi->biClrUsed) + ((yl + y) * bi->biWidth) + xl;
				for (x = 0; x < 8; x += 2)
				{
					dot2 = (((*pimg) & 0x0F) << 4) | (*(pimg + 1) & 0x0F);
					pimg += 2;
					*(outptr++) = dot2;
				} // x
			} // y
		} // xl

	} // yl

	// �p���b�g�ϊ�
	dibpal = (RGBQUAD*)((u_char*)dibbuf + sizeof(BITMAPINFOHEADER));
	// GGGGG_RRRRR_BBBBB_I
	for (x=0; x<256; x++)
	{
		paltmp = &dibpal[x];
		x68pal = ((paltmp->rgbGreen >> 3) << 11) | ((paltmp->rgbRed >> 3) << 6) | ((paltmp->rgbBlue >> 3) << 1);
#ifdef BIG_ENDIAN
		px2buf->pal[x] = x68pal;
#else
		px2buf->pal[x] = (x68pal>>8)|((x68pal & 0xFF) << 8);			// �G���f�B�A���ϊ�
#endif
	}

	if (!opt_p)
	{
		// PX2�t�@�C���o��
		fp = fopen(outfilename,"wb");
		if (fp == NULL)
		{
			printf("Can't write '%s'.\n", outfilename);
			return -1;
		}

		// PX2�p�^�[���o��
		a = fwrite(px2buf, 1, sizeof(PX2FILE), fp);
		if (a != (sizeof(PX2FILE)))
		{
			printf("'%s' �t�@�C�����������������߂܂���ł����I\n", outfilename);
			err++;
		}

		fclose(fp);

		// ���ʏo��
		if (err == 0)
		{
			printf("PX2�f�[�^ '%s'���쐬���܂����B\n", outfilename);
		}
		else
		{
			return -1;
		}
	}
	else
	{
		// �x�^�t�@�C���o��
		fp = fopen(patfilename,"wb");
		if (fp == NULL)
		{
			printf("Can't write '%s'.\n", patfilename);
			return -1;
		}

		// PAT�p�^�[���o��
		err = 0;
		a = fwrite(px2buf->sprpat, 1, sizeof(px2buf->sprpat), fp);
		if (a != (sizeof(px2buf->sprpat)))
		{
			printf("'%s' �t�@�C�����������������߂܂���ł����I\n", patfilename);
			err++;
		}

		fclose(fp);

		// ���ʏo��
		if (err == 0)
		{
			printf("�p�^�[���f�[�^ '%s'���쐬���܂����B\n", patfilename);
		}
		else
		{
			return -1;
		}

		fp = fopen(palfilename, "wb");
		if (fp == NULL)
		{
			printf("Can't write '%s'.\n", palfilename);
			return -1;
		}

		// PAL�o��
		err = 0;
		a = fwrite(px2buf->pal, 1, sizeof(px2buf->pal), fp);
		if (a != (sizeof(px2buf->pal)))
		{
			printf("'%s' �t�@�C�����������������߂܂���ł����I\n", palfilename);
			err++;
		}

		fclose(fp);

		// ���ʏo��
		if (err == 0)
		{
			printf("�p���b�g�f�[�^ '%s'���쐬���܂����B\n", palfilename);
		}
		else
		{
			return -1;
		}

	}

	return 0;
}
