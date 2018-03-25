LDLIBS=-I/usr/include -lXm -lXtst -lXt -lX11

EXES=xmcommand

all: $(EXES)

clean:
	$(RM) $(EXES)
