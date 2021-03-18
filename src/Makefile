PROG=json2oab
CPPFLAGS=-O3 -Werror -Wall -Wno-unused-result
BUILDDIR=build
LDFLAGS=-lboost_system -lboost_filesystem -lexpat -lboost_iostreams -lboost_regex -lkmlbase -lkmlconvenience -lkmlengine -lkmldom
OBJS=activationtime.o coord.o json2oab.o jsonparser.o kmlcreator.o oab.o otb.o

all: $(OBJS)
	mkdir -p $(BUILDDIR)
	$(CXX) -o $(BUILDDIR)/$(PROG) -I/usr/include $(OBJS) $(LDFLAGS)

clean:
	rm -rf *.o $(BUILDDIR)

install:
	cp $< build/json2oab /usr/local/bin

uninstall:
	rm -f /usr/local/bin/json2oab
