# この makefile は、msys や cygwin などの Unix 互換環境上で利用することを想定している。
# ビルドには xdev68k が必要。
# https://github.com/yosshin4004/xdev68k

# 必要な環境変数が定義されていることを確認する。
ifndef ELFX68K_DIR
	$(error ERROR : ELFX68K_DIR is not defined.)
endif

# デフォルトサフィックスを削除
.SUFFIXES:

# ビルド対象の CPU
CPU = 68000

# 各種コマンド短縮名
CXX = ${ELFX68K_DIR}/bin/m68k-xelf-c++.exe
CC = ${ELFX68K_DIR}/bin/m68k-xelf-gcc.exe
HAS = ${ELFX68K_DIR}/bin/m68k-xelf-gcc.exe
HLK = ${ELFX68K_DIR}/bin/m68k-xelf-gcc.exe

LDFLAGS = -lc -lm

# 最終生成物
EXECUTABLE = PNG2PX2.X
TARGET_FILES = $(EXECUTABLE)

# ヘッダ検索パス
INCLUDE_FLAGS = -I./


# コンパイルフラグ
COMMON_FLAGS = -m$(CPU) -Os $(INCLUDE_FLAGS)
CFLAGS = $(COMMON_FLAGS) -finput-charset=cp932 -fexec-charset=cp932 -fverbose-asm -DBIG_ENDIAN
CXXFLAGS = $(CFLAGS) -std=gnu++17

# *.c ソースファイル
C_SRCS = PNG2PX2.c pngctrl.c

# *.cpp ソースファイル
CPP_SRCS =

# *.s ソースファイル
ASM_SRCS =

# リンク対象のライブラリファイル
LIBS =\
	${ELFX68K_DIR}/m68k-elf/lib/libpng.a \
	${ELFX68K_DIR}/m68k-elf/lib/libz.a \
	${ELFX68K_DIR}/m68k-elf/lib/libc.a

# 中間ファイル生成用ディレクトリ
INTERMEDIATE_DIR = _build/m$(CPU)

# オブジェクトファイル
OBJS =	$(addprefix $(INTERMEDIATE_DIR)/,$(patsubst %.c,%.o,$(C_SRCS))) \
	$(addprefix $(INTERMEDIATE_DIR)/,$(patsubst %.cpp,%.o,$(CPP_SRCS))) \
	$(addprefix $(INTERMEDIATE_DIR)/,$(patsubst %.s,%.o,$(ASM_SRCS)))

# 依存関係ファイル
DEPS =	$(addprefix $(INTERMEDIATE_DIR)/,$(patsubst %.c,%.d,$(C_SRCS))) \
	$(addprefix $(INTERMEDIATE_DIR)/,$(patsubst %.cpp,%.d,$(CPP_SRCS)))

# HLK に入力するリンクリスト
HLK_LINK_LIST = $(INTERMEDIATE_DIR)/_lk_list.tmp

# デフォルトのターゲット
all : $(TARGET_FILES)

# 依存関係ファイルの include
-include $(DEPS)

# 中間生成物の削除
clean :
	rm -f $(TARGET_FILES)
	rm -rf $(INTERMEDIATE_DIR)

# 実行ファイルの生成
#	HLK に長いパス文字を与えることは難しい。
#	回避策としてリンク対象ファイルを $(INTERMEDIATE_DIR) 以下にコピーし、
#	短い相対パスを用いてリンクを実行させる。
$(EXECUTABLE) : $(OBJS) $(LIBS)
	mkdir -p $(INTERMEDIATE_DIR)
	rm -f $(HLK_LINK_LIST)
	echo $(LDFLAGS) >> $(HLK_LINK_LIST)
	@for FILENAME in $(OBJS); do\
		echo $$FILENAME >> $(HLK_LINK_LIST); \
	done
	@for FILENAME in $(LIBS); do\
		cp $$FILENAME $(INTERMEDIATE_DIR)/`basename $$FILENAME`; \
		echo $(INTERMEDIATE_DIR)/`basename $$FILENAME` >> $(HLK_LINK_LIST); \
	done
	$(HLK) @$(HLK_LINK_LIST) -o $(EXECUTABLE)

# *.c ソースのコンパイル
$(INTERMEDIATE_DIR)/%.o : %.c makefile
	mkdir -p $(dir $(INTERMEDIATE_DIR)/$*.o)
	$(CC) -c $(CFLAGS) -o $(INTERMEDIATE_DIR)/$*.o -MT $(INTERMEDIATE_DIR)/$*.o -MD -MP -MF $(INTERMEDIATE_DIR)/$*.d $<

# *.cpp ソースのコンパイル
$(INTERMEDIATE_DIR)/%.o : %.cpp makefile
	mkdir -p $(dir $(INTERMEDIATE_DIR)/$*.o)
	$(CXX) -c $(CXXFLAGS) -o $(INTERMEDIATE_DIR)/$*.o -MT $(INTERMEDIATE_DIR)/$*.o -MD -MP -MF $(INTERMEDIATE_DIR)/$*.d $<

# *.s ソースのアセンブル
$(INTERMEDIATE_DIR)/%.o : %.s makefile
	mkdir -p $(dir $(INTERMEDIATE_DIR)/$*.o)
	$(HAS) $(INCLUDE_FLAGS) $*.s -o $(INTERMEDIATE_DIR)/$*.o
