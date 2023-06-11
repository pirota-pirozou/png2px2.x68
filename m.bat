gcc -c -O -DALLMEM -DBIG_ENDIAN -cpp-stack=409600 -IB:\INCLUDE PNG2PX2.c -o PNG2PX2.o
gcc -c -O -DALLMEM -DBIG_ENDIAN -cpp-stack=409600 -IB:\INCLUDE pngctrl.c -o pngctrl.o
gcc -O -cpp-stack=409600 PNG2PX2.o pngctrl.o -o PNG2PX2.x %lib%\libz.a %lib%\libpng.a %lib%\floatfnc.l