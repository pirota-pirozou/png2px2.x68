#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>

#include	"png.h"
#include	"pngctrl.h"

#define PNG_BYTES_TO_CHECK 4

#ifdef __cplusplus
extern "C" {
#endif

	// png関連
	static unsigned char * pngstream_ptr = NULL;

	static int				png_trans_num;				// 透明色の数
	static png_color_16		png_trans_col;				// 透明色
	static png_colorp		palette;                    // パレット

	// png読み込みストリーム
	static void libReadStream(png_structp png_ptr, png_bytep data, png_size_t length)
	{
		memcpy(data, pngstream_ptr, length);

		pngstream_ptr += length;
	}

	// pngストリーム初期化
	static void libInitStream(void *pptr)
	{
		pngstream_ptr = (unsigned char *)pptr;
	}

	// ビットマップ2次元配列確保
	unsigned char ** alloc_map(IMAGEDATA* img)
	{
		int width = img->width;
		int height = img->height;
		int i;

		img->map = malloc(sizeof(unsigned char*) * height);
		if (img->map == NULL)
		{
			return NULL;
		}

		for (i = 0; i < height; i++)
		{
			img->map[i] = malloc(sizeof(unsigned char) * width);
			if (img->map[i] == NULL)
			{
				return NULL;
			}
		}

		return img->map;
	}

	// ビットマップ2次元配列解放
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

	// pngを書き込み（インデックスカラー専用）
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

	// ビットマップとパレットをpngとしてセーブ
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
		case COLOR_TYPE_INDEX:  // インデックスカラー
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
		case COLOR_TYPE_INDEX:  // インデックスカラー
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

	// pngを開きIMGに変換
	pIMGBUF pngptr2img(u_char *pptr)
	{
		png_structp     png_ptr;
		png_infop       info_ptr;
		unsigned long   width, height;
		int             bit_depth, color_type, interlace_type;
		u_char			**image;
		unsigned int	i;
		pIMGBUF			img_ptr = NULL;
		png_uint_32		ulRowBytes;
		png_uint_32		ulUnitBytes;
		png_bytep		imagetmp;
		color_t         *pcolors;
		png_bytep 		ptr;

		u_char*			imgp;
		int				alc_sz;
		unsigned int num_pal;

//		png_bytep		pal_trns_p = NULL;
		int				num_trns;

		png_color_16p	trans_values = NULL;

		u_char			*dest;
		int				x, y;
		png_byte 		c;

		// ファイルはPNGか？
		if (png_sig_cmp((unsigned char *)pptr, (png_size_t)0, PNG_BYTES_TO_CHECK))
			return NULL;

		//
		png_ptr = png_create_read_struct(							// png_ptr構造体を確保・初期化
			PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		if (png_ptr == NULL)
		{
			return NULL;
		}

		info_ptr = png_create_info_struct(png_ptr);					// info_ptr構造体を確保・初期化
		if (info_ptr == NULL)
		{
			png_destroy_read_struct(&png_ptr, NULL, NULL);
			return NULL;
		}

		libInitStream(pptr);										// ストリームを初期化
		png_set_read_fn(png_ptr, NULL, libReadStream);				// libpngにストリーム処理を設定

		png_read_info(png_ptr, info_ptr);			               // PNGファイルのヘッダを読み込み
		png_get_IHDR(png_ptr, info_ptr, &width, &height,	       // IHDRチャンク情報を取得
			&bit_depth, &color_type, &interlace_type,
			NULL, NULL);

		if (color_type == PNG_COLOR_TYPE_PALETTE)
		{
			png_get_PLTE(png_ptr, info_ptr, &palette, &num_pal);            // パレット取得
		}
		else
		{
			num_pal = 0;
		}
		/*
			if (color_type == PNG_COLOR_TYPE_PALETTE)
			{
				png_set_palette_to_rgb(png_ptr);	// これでTRNSチャンクも展開され、1dot=RGBAに展開されてしまう
			}
		//    png_set_strip_alpha(png_ptr);			// これでALPHAを展開しなくて済む

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


		  // 描画先の初期化
		  //   画像サイズ = width, height
		  //   色数 = bit_depth
		  //   ほか

		  // 横１ラインのバイト数
		ulRowBytes = png_get_rowbytes(png_ptr, info_ptr);
		// １ドットのバイト数
		ulUnitBytes = ulRowBytes / width;

		// 横は4dotの倍数になるように
		if (width & 3)
		{
			int adj = (4 - (width & 3));
			width += adj;
			ulRowBytes += adj * ulUnitBytes;
		}

		// 16色(Packed)の場合にパッチする
		if (ulUnitBytes == 0)
		{
			ulRowBytes *= 2;
		}

		printf("num_pal=%d, width=%d, height=%d, ulRowBytes=%d, ulUnitBytes=%d\n", num_pal, width, height, ulRowBytes, ulUnitBytes);

		// 確保サイズ
		alc_sz = (sizeof(png_byte)* ulRowBytes * height) + sizeof(color_t)* num_pal;

		// ビットマップを確保
		img_ptr = (pIMGBUF)malloc(alc_sz);
		if (img_ptr == NULL)
		{
			printf("pngptr2img(): malloc error\n");
			png_destroy_read_struct(&png_ptr, NULL, NULL);
			return NULL;
		}

		// img_ptr をクリア
		memset(img_ptr, 0, alc_sz);

		// 透過色チェック
		png_trans_num = 0;
		if (png_get_tRNS(png_ptr, info_ptr, NULL, &num_trns, &trans_values))
		{
			png_trans_num = num_trns;					// 透明色の数
			png_trans_col = *trans_values;				// 透明色
		}

		// ヘッダに書き込み
		//
		imgp = (u_char *)img_ptr->raw;					// ビットマップへのポインタ
		pcolors = (color_t *)img_ptr->palette;          // color_tへのポインタ

		// パレットコピー
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

		// 展開ポインタ配列を確保
		image = (png_bytepp)malloc(height * sizeof(png_bytep));

		// 16色(Packed)以外の場合（通常）
		if (ulUnitBytes != 0)
		{
			for (i = 0; i < height; i++) {
				image[i] = (png_bytep)(imgp + (i * ulRowBytes));
			}
			png_read_image(png_ptr, image);							// 画像データの展開
		}
		else
		{
			// 16色(Packed)の場合
			imagetmp = (png_bytepp)malloc(height * width / 2);

			for (i = 0; i < height; i++) {
				image[i] = (png_bytep)imagetmp + (i * (width / 2));
			}
			png_read_image(png_ptr, image);							// 画像データの展開

			// 4bit → 8bit展開
			for (y = 0; y < height; y++)
			{
				ptr = image[y];
				dest = (png_bytep)imgp + (y * ulRowBytes);
				for (x = 0; x < width; x += 2)
				{
					c = *ptr;
					*dest = (c >> 4);
					*(dest+1) = (c & 0x0F);
					ptr++;
					dest += 2;
				}
			}
			free(imagetmp);
		}
		free(image);

		png_destroy_read_struct(								// libpng構造体のメモリ解放
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

		// ファイル名はPNGデータを指すポインタか？
		if (!png_sig_cmp((const u_char *)szFile, (png_size_t)0, PNG_BYTES_TO_CHECK))
		{
			printf("PngOpenFile: png_sig_cmp(is Pointer)\n");
			// もしそうだったら直接展開
			pimg = pngptr2img((const u_char *)szFile);

			return pimg;
		}

		printf("PngOpenFile(%s)\n", szFile);
		// PNGファイルを丸ごと開いてメモリ確保
		fp = fopen(szFile, "rb");
		if (fp == NULL)
		{
			printf("PngOpenFile: fopen(%s) failed\n", szFile);
			// 開けない
			return NULL;
		}
		fseek(fp, 0L, SEEK_END);							// ファイルサイズの取得
		fsize = ftell(fp); 	        						// get the current file pointer
		fseek(fp, 0L, SEEK_SET);							// ファイルの先頭に

		// メモリ確保
		png = (u_char *)malloc(fsize);
		if (png == NULL)
		{
			printf("PngOpenFile: malloc(%d) failed\n", fsize);
			fclose(fp);
			return NULL;
		}

		// 読み込み
		rbytes = fread(png, 1, fsize, fp);
		fclose(fp);

		// 全部読めなかった？
		if (fsize != rbytes)
		{
			printf("PngOpenFile: fread(%d) failed\n", fsize);
			return NULL;
		}

		// PNGからimgに展開
		pimg = pngptr2img(png);

		// PNGイメージ開放
		free(png);

		return pimg;
	}

#ifdef __cplusplus
}	// extern "C"
#endif
