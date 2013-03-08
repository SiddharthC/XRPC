# Since the programs in this directory are examples for the user, this make
# file should be as ordinary as possible.  It should not rely heavily on
# included make files or configuration parameters.  Also, we don't try to
# build or rebuild the libraries on which these programs depend or even
# recognize that they've changed on their own.


ifeq ($(SRCDIR),)
SRCDIR = $(CURDIR)/..
BLDDIR = $(SRCDIR)
endif
SUBDIR = assignment_3

include $(BLDDIR)/config.mk

default: all

CFLAGS = $(CFLAGS_PERSONAL) $(CADD)
LDFLAGS += $(LADD)

# If this were a real application, working from an installed copy of
# Xmlrpc-c, XMLRPC_C_CONFIG would just be 'xmlrpc-c-config'.  It would be
# found in the user's PATH.
XMLRPC_C_CONFIG = $(BLDDIR)/xmlrpc-c-config.test

CLIENTPROGS = \
  client \

SERVERPROGS_ABYSS = \
  server \

# Build up PROGS:
PROGS = 

PROGS += $(BASIC_PROGS)

ifeq ($(ENABLE_ABYSS_SERVER),yes)
  PROGS += $(SERVERPROGS_ABYSS)
endif

ifeq ($(MUST_BUILD_CLIENT),yes)
  PROGS += $(CLIENTPROGS)
endif

INCLUDES = -I. $(shell $(XMLRPC_C_CONFIG) client abyss-server --cflags)

LIBS_CLIENT = \
  $(shell $(XMLRPC_C_CONFIG) client --libs)

LIBS_SERVER_ABYSS = \
  $(shell $(XMLRPC_C_CONFIG) abyss-server --libs)

LIBS_SERVER_CGI = \
  $(shell $(XMLRPC_C_CONFIG) cgi-server --libs)

LIBS_BASE = \
  $(shell $(XMLRPC_C_CONFIG) --libs)

all: $(PROGS)

ifeq ($(ENABLE_CPLUSPLUS),yes)
  all: cpp/all
endif

.PHONY: cpp/all

# When building in separate tree, directory won't exist yet
$(BLDDIR)/assignment_3/cpp:
	mkdir $@

$(CLIENTPROGS):%:%.o
	$(CCLD) -o $@ $^ $(LIBS_CLIENT) $(LDFLAGS)
	mkdir bin
	mv ./server ./bin/server
	cp ./service ./bin/service
	mv ./client ./bin/client

$(SERVERPROGS_ABYSS):%:%.o
	$(CCLD) -o $@ $^ $(LIBS_SERVER_ABYSS) $(LDFLAGS)

$(BASIC_PROGS):%:%.o
	$(CCLD) -o $@ $^ $(LIBS_BASE) $(LDFLAGS) 


OBJECTS = $(patsubst %,%.o,$(patsubst %.cgi,%_cgi,$(PROGS)))

$(OBJECTS):%.o:%.c
	$(CC) -c $(INCLUDES) $(CFLAGS) $<

# config.h and xmlrpc_amconfig.h just describe the build environment.
# We use them so that the example programs will build in users'
# various environments.  If you're copying these examples, you can
# just remove these headers from the programs and hardcode whatever is
# right for your build environment.

$(OBJECTS): config.h xmlrpc_amconfig.h

config.h:
	$(LN_S) $(BLDDIR)/xmlrpc_config.h $@
xmlrpc_amconfig.h:
	$(LN_S) $(BLDDIR)/$@ .

.PHONY: clean
clean:
	rm -f $(PROGS) *.o config.h xmlrpc_amconfig.h
	rm -r $(CURDIR)/bin

.PHONY: distclean
distclean: clean

BINDIR=$(DESTDIR)$(bindir)

FILENAME_GENERATOR = "echo $$p|sed 's/$(EXEEXT)$$//'|sed '$(transform)'|sed 's/$$/$(EXEEXT)/'"

INSTCMD =  "$(LIBTOOL) --mode=install $(INSTALL_PROGRAM) $$p \
  $(BINDIR)/`$(FILENAME_GENERATOR)`"

.PHONY: install
install: $(PROGS)
	@$(NORMAL_INSTALL)
	$(MKINSTALLDIRS) $(BINDIR)
	@list='$(bin_PROGRAMS)'; for p in $$list; do \
	  if test -f $$p; then \
	    echo "$(INSTCMD)"; $(INSTCMD); \
	  else :; \
          fi; \
	done

.PHONY: check
check:

.PHONY: dep depend
dep depend:
