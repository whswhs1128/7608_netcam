include Makefile.params
 
.PHONY:all clean distclean

SRC=$(wildcard *.c)
OBJS=$(SRC:.c=.o)
PROGRAM=plist

all:$(OBJS)
	$(GCC) $(CFLAGS) $(EXTRA_CFLAGS) $^ -o $(PROGRAM)
	$(STRIP) $(PROGRAM)
#ifeq ($(LIBRARYS),static)
#	$(LD) $(LIB_CFLAGS) $(LIB) $(EXTRA_CFLAGS) $^ 
#	$(RANLIB) $(LIB)  
#else
#	$(LD) $(LIB_CFLAGS) $(LIB) $(EXTRA_CFLAGS) $^ 
#endif

%o:%c
	$(GCC) $(CFLAGS) $(EXTRA_CFLAGS) $^ -c

clean:
	@rm $(wildcard *.o)  $(PROGRAM) -f
