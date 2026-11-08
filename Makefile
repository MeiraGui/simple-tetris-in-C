SRCNAME = tetris
TARGETNAME = $(SRCNAME)

ifdef OS
	GCC = gcc
	TARGET = $(TARGETNAME).exe
	LIBS = -lwinmm
	RM = del /Q
else
	# I'm using musl-gcc on linux to make a static binary that's much smaller while still being compatible
	GCC = musl-gcc
	TARGET = $(TARGETNAME)
	LIBS =
	RM = rm -f
endif

SRC = $(SRCNAME).c $(SRCNAME).h

all = $(TARGET)

$(TARGET) : $(SRC)
		$(GCC) $(SRC) -o $(TARGET) $(LIBS) --static
		strip --strip-all $(TARGET)

clean:
	$(RM) $(TARGET)

.PHONY: all clean
