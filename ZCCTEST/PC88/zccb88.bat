zcc +pc88 -DNODELAY -lm %1.c -o %1.bin %2
appmake +msx -b %1.bin --org 0xA700 -o %1.tmp
conv %1.tmp %1.b88
