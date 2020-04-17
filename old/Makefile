PROGS= scntrn

CC= gcc
CFLAGS= -g -Wall -O2 -I/usr/pkg/include -I/usr/pkg/include/freetype2 \
	-I/usr/pkg/include/ImageMagick-6

LIBPATH= -L/usr/pkg/lib -Wl,-R/usr/pkg/lib \
         -L/usr/X11R7/lib -Wl,-R/usr/X11R7/lib -R/usr/X11R7/lib \
         -Wl,--rpath -Wl,/usr/pkg/lib
LIBS= -lz -lMagickCore-6.Q16 -llcms -ltiff -ljasper -ljpeg \
 -lpng16 -lXext -lXt -lSM -lICE -lX11 -lbz2 -lxml2 -lz -lm -lMagickWand-6.Q16 -lMagickCore-6.Q16
#-lfreetype -Xext
all: $(PROGS)
scntrn: scntrn.o
	$(CC) -g -o scntrn scntrn.o $(LIBPATH) $(LIBS)

clean:
	rm -f *~ *.o *core *.BAK $(PROGS)
