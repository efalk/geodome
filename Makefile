#	$Id$

# Note: version number in Makefile, geodome.lsm, geodome.spec

VERSION = 1.0

#OPT = -g -Wall -DDEBUG
OPT = -O

#LDFLAGS = -g
LDFLAGS = -s


# Probably no need to edit anything below this line.


PGMS = dome dome_struts dome_layout dome_3ds

DOC = AUTHORS INSTRUCTIONS README

CFLAGS = $(OPT)

LIBS = -L/usr/X11R6/lib -lGL -lGLU -lglut -lXi -lXmu -lX11 -lm


prefix = /usr/local
bindir = $(prefix)/bin
docdir = $(prefix)/doc/geodome-$(VERSION)
mandir = $(prefix)/man
man1dir = $(prefix)/man1



all: $(PGMS)


HDRS = 3ds_utils.h dome.h utils.h

DOME_SRCS =	main.c dome.c dome_math.c dome_file.c utils.c
DOME_OBJS =	$(DOME_SRCS:.c=.o)

DOME_3DS_SRCS =	dome_3ds.c dome_file.c dome_math.c 3ds_utils.c utils.c
DOME_3DS_OBJS =	$(DOME_3DS_SRCS:.c=.o)

DOME_STRUTS_SRCS = dome_struts.c dome_file.c dome_math.c utils.c
DOME_STRUTS_OBJS = $(DOME_STRUTS_SRCS:.c=.o)

DOME_LAYOUT_SRCS = dome_layout.c dome_file.c dome_math.c utils.c
DOME_LAYOUT_OBJS = $(DOME_LAYOUT_SRCS:.c=.o)

ALL_SRCS = 3ds_utils.c dome_3ds.c dome.c dome_file.c dome_layout.c \
	dome_math.c dome_struts.c main.c utils.c



dome:	$(DOME_OBJS)
	$(CC) $(LDFLAGS) -o $@ $(DOME_OBJS) $(LIBS)

dome_struts: $(DOME_STRUTS_OBJS)
	$(CC) $(LDFLAGS) -o $@ $(DOME_STRUTS_OBJS) -lm

dome_layout: $(DOME_LAYOUT_OBJS)
	$(CC) $(LDFLAGS) -o $@ $(DOME_LAYOUT_OBJS) -lm

dome_3ds: $(DOME_3DS_OBJS)
	$(CC) $(LDFLAGS) -o $@ $(DOME_3DS_OBJS) -l3ds -lm

icosahedron: icosahedron.o
	$(CC) $(LDFLAGS) -o $@ icosahedron.o -lm

install: all $(bindir) $(docdir)
	install -s $(PGMS) $(bindir)
	install -m 0444 $(DOC) $(docdir)

$(bindir) $(docdir):
	mkdir -p $@


tags:	$(ALL_SRCS) $(HDRS)
	ctags *.[ch] /usr/include/GL/*.h ../../Lib3ds/lib3ds/*.[ch]


# Build the tarball

DIST_NAME = geodome-$(VERSION)
DIST_DIR = /tmp/$(DIST_NAME)
DIST_FILES = Makefile geodome.spec geodome.lsm $(DOC) $(HDRS) $(ALL_SRCS)

dist: clobber $(DIST_FILES)
	rm -rf $(DIST_DIR)
	mkdir $(DIST_DIR)
	cp -pr $(DIST_FILES) $(DIST_DIR)
	chmod -R a+r $(DIST_DIR)
	chmod -R u+w $(DIST_DIR)
	chmod -R go-w $(DIST_DIR)
	cd $(DIST_DIR)/.. ; tar cvf $(DIST_NAME).tar $(DIST_NAME)
	mv $(DIST_DIR).tar .
	gzip $(DIST_NAME).tar
	rm -rf $(DIST_DIR)


# Build the .rpm

RPM_SOURCE_DIR = /tmp/rpmsource-$(DIST_NAME)
RPM_BUILD_DIR = /tmp/rpmbuild-$(DIST_NAME)

rpm: $(DIST_NAME).tar.gz
	rm -rf $(RPM_SOURCE_DIR)
	rm -rf $(RPM_BUILD_DIR)
	mkdir -p $(RPM_SOURCE_DIR)
	mkdir -p $(RPM_BUILD_DIR)
	cp $(DIST_NAME).tar.gz $(RPM_SOURCE_DIR)

	rpmbuild -bb geodome.spec \
	  --define "_sourcedir ${RPM_SOURCE_DIR}" \
	  --define "_builddir ${RPM_BUILD_DIR}" \
	  --define "_rpmdir ${RPM_SOURCE_DIR}"

	mv ${RPM_SOURCE_DIR}/i386/geodome-*.rpm .
	rm -rf ${RPM_SOURCE_DIR} ${RPM_BUILD_DIR}


clean:
	rm -f *.o *.i

clobber: clean
	rm -f tags $(PGMS)

depend:
	makedepend -- $(CFLAGS) -- $(ALL_SRCS)


# DO NOT DELETE

