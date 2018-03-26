########################################################################
### Minimal Makefile; may require something like this:
###
###   CPPFLAGS=-I/usr/local/include
###
### to work on some systems
########################################################################

LDLIBS=-lXm -lXt -lX11

EXES=xmcommand

all: $(EXES)

clean:
	$(RM) $(EXES)
