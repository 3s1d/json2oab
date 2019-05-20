PROG=json2oab
CPPFLAGS=-g -Wall
BUILDDIR=build
LDFLAGS=-lboost_system -lboost_filesystem -lexpat -lboost_iostreams
OBJS=activationtime.o coord.o json2oab.o jsonparser.o kmlcreator.o oab.o

all: $(OBJS)
	mkdir -p $(BUILDDIR)
	$(CXX) -o $(BUILDDIR)/$(PROG) $(OBJS) $(LDFLAGS)

clean:
	        rm -rf *.o $(BUILDDIR)
