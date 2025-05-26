:zcc +cpm -compiler=ez80clang  -DNODELAY -lm -create-app -subtype=z80pack -Os %1.c -o %1.asm -a %2
zcc +MSX -compiler=ez80clang  -DNODELAY -lm -create-app -subtype=disk -Os %1.c -o %1.msx %2
