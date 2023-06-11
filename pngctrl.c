#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>

#include	"png.h"
#include	"pngctrl.h"

#define PNG_BYTES_TO_CHECK 4

#ifdef __cplusplus
extern "C" {
#endif

	// png�֘A
	static unsigned char * pngstream_ptr = NULL;

	static int				png_trans_num;				// �����F�̐�
	static png_color_16		png_trans_col;				// �����F
	static png_colorp		palette;                    // �p���b�g

	// png�ǂݍ��݃X�g���[��
	static void libReadStream(png_structp png_ptr, png_bytep data, png_size_t length)
	{
		memcpy(data, pngstream_ptr, length);

		pngstream_ptr += length;
	}

	// png�X�g���[��������
	static void libInitStream(void *pptr)
	{
		pngstream_ptr = (u_char *)pptr;
	}

	// �r�b�g�}�b�v2�����z��m��
	png_bytepp alloc_map(IMAGEDATA* img)
	{
		int width = img->width;
		int height = img->height;
		int i;

		img->map = (png_bytepp) malloc(sizeof(png_bytep) * height);
		if (img->map == NULL)
		{
			return NULL;
		}

		for (i = 0; i < height; i++)
		{
			img->map[i] = (png_bytep) malloc(sizeof(png_byte) * width);
			if (img->map[i] == NULL)
			{
				return NULL;
			}
		}

		return img->map;
	}

	// �r�b�g�}�b�v2�����z����
	void free_map(IMAGEDATA* img)
	{
		int height = img->height;
		int i;
		if (img->map != NULL)
		{
			for (i = 0; i < height; i++)
			{
				free(img->map[i]);
				img->map[i] = NULL;
			}
			free(img->map);
			img->map = NULL;
		}

	}

	// png���������݁i�C���f�b�N�X�J���[��p�j
	int writepng(const char *filename, IMAGEDATA *img)
	{
		int result = -1;
		FILE *fp;

		if (img == NULL) {
			return result;
		}
		fp = fopen(filename, "wb");
		if (fp == NULL) {
			perror(filename);
			return result;
		}
		result = write_png_stream(fp, img);
		fclose(fp);

		return 0;
	}

	// �r�b�g�}�b�v�ƃp���b�g��png�Ƃ��ăZ�[�u
	int write_png_stream(FILE* fp, IMAGEDATA *img)
	{
		int i, x, y;
		int result = -1;
		int row_size;
		int color_type;
		png_structp png = NULL;
		png_infop info = NULL;
		png_bytep row;
		png_bytepp rows = NULL;
		png_colorp palette = NULL;
		if (img == NULL) {
			return result;
		}
		switch (img->color_type) {
		case COLOR_TYPE_INDEX:  // �C���f�b�N�X�J���[
			color_type = PNG_COLOR_TYPE_PALETTE;
			row_size = sizeof(png_byte) * img->width;
			break;
		default:
			return result;
		}
		png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		if (png == NULL) {
			goto error;
		}
		info = png_create_info_struct(png);
		if (info == NULL) {
			goto error;
		}
		if (setjmp(png_jmpbuf(png))) {
			goto error;
		}
		png_init_io(png, fp);
		png_set_IHDR(png, info, img->width, img->height, 8,
			color_type, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
			PNG_FILTER_TYPE_DEFAULT);
		rows = png_malloc(png, sizeof(png_bytep) * img->height);
		if (rows == NULL) {
			goto error;
		}
		png_set_rows(png, info, rows);
		memset(rows, 0, sizeof(png_bytep) * img->height);
		for (y = 0; y < img->height; y++) {
			if ((rows[y] = png_malloc(png, row_size)) == NULL) {
				goto error;
			}
		}
		switch (img->color_type) {
		case COLOR_TYPE_INDEX:  // �C���f�b�N�X�J���[
			palette = png_malloc(png, sizeof(png_color) * img->palette_num);
			for (i = 0; i < img->palette_num; i++) {
				palette[i].red = img->palette[i].r;
				palette[i].green = img->palette[i].g;
				palette[i].blue = img->palette[i].b;
			}
			png_set_PLTE(png, info, palette, img->palette_num);
			for (i = img->palette_num - 1; i >= 0 && img->palette[i].a != 0xFF; i--);
			if (i >= 0) {
				int num_trans = i + 1;
				png_byte trans[256];
				for (i = 0; i < num_trans; i++) {
					trans[i] = img->palette[i].a;
				}
				png_set_tRNS(png, info, trans, num_trans, NULL);
			}
			png_free(png, palette);
			for (y = 0; y < img->height; y++) {
				row = rows[y];
				for (x = 0; x < img->width; x++) {
					*row++ = img->map[y][x];
				}
			}
			break;

		default:;
		}
		png_write_png(png, info, PNG_TRANSFORM_IDENTITY, NULL);
		result = 0;
	error:
		if (rows != NULL) {
			for (y = 0; y < img->height; y++) {
				png_free(png, rows[y]);
			}
			png_free(png, rows);
		}
		png_destroy_write_struct(&png, &info);
		return result;
	}

	// png���J��IMG�ɕϊ�
	pIMGBUF pngptr2img(u_char *pptr)
	{
		png_structp     png_ptr;
		png_infop       info_ptr;
		unsigned long   width, height;
		int             bit_depth, color_type, interlace_type;
		int				i, j;
		png_bytepp		image_row;
		png_bytepp		imagetmp;
		pIMGBUF			img_ptr;
		png_uint_32		row_bytes;
		png_uint_32		ulUnitBytes;
		color_t         *pcolors;
		png_byte 		c;

		int				alc_sz;
		unsigned int	num_pal;

//		png_bytep		pal_trns_p = NULL;
		int				num_trns;

		png_color_16p	trans_values = NULL;

		png_bytep 		src;
		png_bytep		dest;

		// �t�@�C����PNG���H
		if (png_sig_cmp((u_char *)pptr, (png_size_t)0, PNG_BYTES_TO_CHECK))
		{
			printf("Not png file\n");
			return NULL;
		}

		//
		png_ptr = png_create_read_struct(							// png_ptr�\���̂��m�ہE������
			PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		if (png_ptr == NULL)
		{
			printf("png_create_read_struct() error\n");
			return NULL;
		}

		info_ptr = png_create_info_struct(png_ptr);					// info_ptr�\���̂��m�ہE������
		if (info_ptr == NULL)
		{
			printf("png_create_info_struct() error\n");
			png_destroy_read_struct(&png_ptr, NULL, NULL);
			return NULL;
		}

		if (setjmp(png_jmpbuf(png_ptr)))
		{
		    return NULL;
  		}

		libInitStream(pptr);										// �X�g���[����������
		png_set_read_fn(png_ptr, NULL, libReadStream);				// libpng�ɃX�g���[��������ݒ�

		png_read_info(png_ptr, info_ptr);			               // PNG�t�@�C���̃w�b�_��ǂݍ���
		png_get_IHDR(png_ptr, info_ptr, &width, &height,	       // IHDR�`�����N�����擾
			&bit_depth, &color_type, &interlace_type,
			NULL, NULL);

		if (color_type == PNG_COLOR_TYPE_PALETTE)
		{
			png_get_PLTE(png_ptr, info_ptr, &palette, &num_pal);            // �p���b�g�擾
		}
		else
		{
			num_pal = 0;
		}
		/*
			if (color_type == PNG_COLOR_TYPE_PALETTE)
			{
				png_set_palette_to_rgb(png_ptr);	// �����TRNS�`�����N���W�J����A1dot=RGBA�ɓW�J����Ă��܂�
			}
		//    png_set_strip_alpha(png_ptr);			// �����ALPHA��W�J���Ȃ��čς�

			// tell libpng to strip 16 bit/color files down to 8 bits/color
			if (bit_depth == 16)
			   png_set_strip_16(png_ptr);

			if (color_type == PNG_COLOR_TYPE_PALETTE || bit_depth < 8)
				png_set_expand(png_ptr);

			// after the transformations have been registered update info_ptr data
			png_read_update_info(png_ptr, info_ptr);

			// get again width, height and the new bit-depth and color-type
			png_get_IHDR(png_ptr, info_ptr, &width, &height,
							&bit_depth, &color_type, &interlace_type,
							NULL, NULL);

			png_set_bgr(png_ptr);
		  */


		  // �`���̏�����
		  //   �摜�T�C�Y = width, height
		  //   �F�� = bit_depth
		  //   �ق�

		  // ���P���C���̃o�C�g��
		row_bytes = png_get_rowbytes(png_ptr, info_ptr);
		// �P�h�b�g�̃o�C�g��
		ulUnitBytes = row_bytes / width;

		// ����4dot�̔{���ɂȂ�悤��
/*
		if (width & 3)
		{
			int adj = (4 - (width & 3));
			width += adj;
			ulRowBytes += adj * ulUnitBytes;
		}
*/

		// 16�F(Packed)�̏ꍇ�Ƀp�b�`����
		if (ulUnitBytes == 0)
		{
			row_bytes *= 2;
		}

		printf("num_pal=%d, width=%d, height=%d, row_bytes=%d, ulUnitBytes=%d\n", num_pal, width, height, row_bytes, ulUnitBytes);

		// �m�ۃT�C�Y
		alc_sz = (row_bytes * height) + (sizeof(color_t)* num_pal);

		// �r�b�g�}�b�v���m��
		img_ptr = (pIMGBUF)malloc(alc_sz);
		if (img_ptr == NULL)
		{
			printf("pngptr2img(): malloc error\n");
			png_destroy_read_struct(&png_ptr, NULL, NULL);
			return NULL;
		}

		// img_ptr ���N���A
		memset(img_ptr, 0, alc_sz);

		// ���ߐF�`�F�b�N
		png_trans_num = 0;
		if (png_get_tRNS(png_ptr, info_ptr, NULL, &num_trns, &trans_values))
		{
			png_trans_num = num_trns;					// �����F�̐�
			png_trans_col = *trans_values;				// �����F
		}

		pcolors = (color_t *)img_ptr->palette;          // color_t�ւ̃|�C���^

		// �p���b�g�R�s�[
		if (color_type == PNG_COLOR_TYPE_PALETTE)
		{
			for (i = 0; i < num_pal; i++)
			{
				pcolors->r = palette->red;
				pcolors->g = palette->green;
				pcolors->b = palette->blue;
				pcolors->a = 255;
				palette++;
				pcolors++;
			}
		}

		// �W�J�|�C���^�z����m��
		image_row = png_malloc(png_ptr, height * sizeof(png_bytep));

		// 16�F(Packed)�ȊO�̏ꍇ�i�ʏ�j
		if (ulUnitBytes != 0)
		{
			for (i = 0; i < height; i++)
			{
				image_row[i] = png_malloc(png_ptr, width * sizeof(png_byte));
			}
			png_set_rows(png_ptr, info_ptr, image_row);
			png_read_image(png_ptr, image_row);							// �摜�f�[�^�̓W�J
			for (i = 0; i < height; i++)
			{
				memcpy((png_bytep)img_ptr->raw + (i * row_bytes), image_row[i], row_bytes);
			}

		}
		else
		{
			// 16�F(Packed)�̏ꍇ
			imagetmp = (png_bytepp) malloc(height * (width / 2 * sizeof(png_bytep)) );

			for (i = 0; i < height; i++)
			{
				image_row[i] = png_malloc(png_ptr, width * sizeof(png_byte) / 2);
			}
			png_set_rows(png_ptr, info_ptr, image_row);
			png_read_image(png_ptr, image_row);							// �摜�f�[�^�̓W�J

			// 4bit �� 8bit�W�J
			for (j = 0; j < height; j++)
			{
				src = (png_bytep) image_row[j];
				dest = (png_bytep) img_ptr->raw + (j * row_bytes);
				for (i = 0; i < width; i += 2)
				{
					c = *(src++);
					*dest = (c >> 4);
					*(dest+1) = (c & 0x0F);
					dest += 2;
				}
			}
			free(imagetmp);
		}
		for (i = 0; i < height; i++)
			png_free(png_ptr, image_row[i]);

		png_free(png_ptr, image_row);

		png_destroy_read_struct(								// libpng�\���̂̃��������
			&png_ptr, &info_ptr, (png_infopp)NULL);

		return img_ptr;
	}

	// PNGOpenFile
	pIMGBUF PngOpenFile(const char* szFile)
	{
		pIMGBUF pimg = NULL;
		u_char* png = NULL;

		size_t fsize = 0;
		size_t rbytes;

		FILE *fp;

		// �t�@�C������PNG�f�[�^���w���|�C���^���H
		if (!png_sig_cmp((const u_char *)szFile, (png_size_t)0, PNG_BYTES_TO_CHECK))
		{
			printf("PngOpenFile: png_sig_cmp(is Pointer)\n");
			// ���������������璼�ړW�J
			pimg = pngptr2img((u_char *)szFile);

			return pimg;
		}

		printf("PngOpenFile(%s)\n", szFile);
		// PNG�t�@�C�����ۂ��ƊJ���ă������m��
		fp = fopen(szFile, "rb");
		if (fp == NULL)
		{
			printf("PngOpenFile: fopen(%s) failed\n", szFile);
			// �J���Ȃ�
			return NULL;
		}
		fseek(fp, 0L, SEEK_END);							// �t�@�C���T�C�Y�̎擾
		fsize = ftell(fp); 	        						// get the current file pointer
		fseek(fp, 0L, SEEK_SET);							// �t�@�C���̐擪��

		// �������m��
		png = (u_char *)malloc(fsize);
		if (png == NULL)
		{
			printf("PngOpenFile: malloc(%d) failed\n", (int)fsize);
			fclose(fp);
			return NULL;
		}

		// �ǂݍ���
		rbytes = fread(png, 1, fsize, fp);
		fclose(fp);

		// �S���ǂ߂Ȃ������H
		if (fsize != rbytes)
		{
			printf("PngOpenFile: fread(%d) failed\n", (int)fsize);
			return NULL;
		}

		// PNG����img�ɓW�J
		pimg = pngptr2img(png);

		// PNG�C���[�W�J��
		free(png);

		return pimg;
	}

#ifdef __cplusplus
}	// extern "C"
#endif
