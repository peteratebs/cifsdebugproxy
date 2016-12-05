# GNU Makefile for RTSMB Server Application

### Usage ##############################################################
### :make clean
### :make
### :make DEBUG=1              ------------------ For debugging purposes
########################################################################
incpath = .

cflags+= -D SUPPORT_SMB2

## SET SMB_PATH
HTTP_PATH = ../../..

include ../mkgnu.inc

lib_to_http_common_path      = $(HTTP_PATH)/source
lib_to_http_example_path     = $(HTTP_PATH)/examples
lib_to_http_include_path     = $(HTTP_PATH)/include
#include path

includes = . $(lib_to_http_include_path) $(lib_to_rtptarget_include) $(lib_to_rtpgeneric_include)

########################################################################
# SOURCE PATHS
########################################################################

vpath %.c   . $(lib_to_http_common_path) $(lib_to_http_example_path) $(lib_to_rtptarget) $(lib_to_rtpgeneric)

vpath %.h   .. $(lib_to_http_include_path)  $(lib_to_rtptarget_include) $(lib_to_rtpgeneric_include)

########################################################################
# OPERATING SYSTEM/ENVIRONMENT SPECIFIC MODULES
########################################################################
# The only files that should go here are files that are specific to one
# particular target, operating system, compiler, etc. and must go into
# the library.  Make sure your conditional(s) are as general as possible.

ifeq ($(os),LINUX)
#modules +=
endif

########################################################################
# EBS PACKAGES
########################################################################

ifeq ($(dortsmb),"Y")
defines +=
endif


#############################################################################
# SETUP
#############################################################################

# The basic required list of modules, defines, includes.

# clean up the modules, includes, defines

dirs = $(lib_to_rtptarget) $(lib_to_rtpgeneric) $(lib_to_http_example_path) $(lib_to_http_common_path)

#cpfiles := $(foreach dir,$(dirs),$(wildcard $(dir)/*.cpp))
#cpmodules := $(patsubst %.cpp,%,$(cpfiles))
#cfiles := $(foreach dir,$(dirs),$(wildcard $(dir)/*.c))

#HTTP COMMON FILES USED


cfiles += $(lib_to_http_common_path)/fileext.c
cfiles += $(lib_to_http_common_path)/hcookie.c
### cfiles += $(lib_to_http_common_path)/msgqueue.c
cfiles += $(lib_to_http_common_path)/urlparse.c
cfiles += $(lib_to_http_common_path)/httpauth.c
cfiles += $(lib_to_http_common_path)/httpsrv.c
cfiles += $(lib_to_http_common_path)/filetype.c
#### cfiles += $(lib_to_http_common_path)/httptest.c
cfiles += $(lib_to_http_common_path)/httpsrvframework.c
cfiles += $(lib_to_http_common_path)/httpcli.c
cfiles += $(lib_to_http_common_path)/httmlutils.c
cfiles += $(lib_to_http_common_path)/md5.c
cfiles += $(lib_to_http_common_path)/httpscli.c
cfiles += $(lib_to_http_common_path)/httpmcli.c
cfiles += $(lib_to_http_common_path)/httpp.c
cfiles += $(lib_to_http_common_path)/httpssrv.c


#HTTP EXAMPLE FILES USED

cfiles += $(lib_to_http_example_path)/httpssrv.c
cfiles += $(lib_to_http_example_path)/example_server.c
cfiles += $(lib_to_http_example_path)/example_client.c
cfiles += $(lib_to_http_example_path)/example_server_advanced.c
cfiles += $(lib_to_http_example_path)/smbdiagproxy.c




#RTPLATFORM FILES USED
cfiles += $(lib_to_rtpgeneric)/rtpchar.c
cfiles += $(lib_to_rtpgeneric)/rtpmtxdb.c
cfiles += $(lib_to_rtpgeneric)/rtprand.c
cfiles += $(lib_to_rtpgeneric)/rtprot.c
cfiles += $(lib_to_rtpgeneric)/rtpstr.c
cfiles += $(lib_to_rtpgeneric)/rtpscnv.c
cfiles += $(lib_to_rtpgeneric)/rtpwchar.c
cfiles += $(lib_to_rtpgeneric)/rtpwcs.c
cfiles += $(lib_to_rtpgeneric)/rtpwscnv.c
cfiles += $(lib_to_rtpgeneric)/rtphelper.c
cfiles += $(lib_to_rtpgeneric)/rtpdutil.c
cfiles += $(lib_to_rtpgeneric)/rtplog.c
cfiles += $(lib_to_rtptarget)/rtpdate.c
cfiles += $(lib_to_rtptarget))/rtpdebug.c
cfiles += $(lib_to_rtptarget)/rtpdobj.c
cfiles += $(lib_to_rtptarget)/rtpfile.c
cfiles += $(lib_to_rtptarget)/rtpmem.c
cfiles += $(lib_to_rtptarget)/rtpnet.c
cfiles += $(lib_to_rtptarget)/rtpsignl.c
cfiles += $(lib_to_rtptarget)/rtpterm.c
cfiles += $(lib_to_rtptarget)/rtpthrd.c
cfiles += $(lib_to_rtptarget)/rtptime.c
cfiles += $(lib_to_rtptarget)/rtpwfile.c

#DEMO FILE
cfiles +=  httpexampleapplication.c

cmodules := $(patsubst %.c,%,$(cfiles))

modules += $(cpmodules)
modules += $(cmodules)

modules  := $(sort $(modules))
#includes := $(sort $(includes))
defines  := $(sort $(defines))

# now set up the object list
blah1 = $(modules:%=%.$(obj))
objects = $(notdir $(blah1))
OBJDIR=./bin
OUT_OBJS = $(addprefix $(OBJDIR)/,$(objects))

#############################################################################
# RULES
#############################################################################
all: httpexampleapplication.out

$(OBJDIR)/httpexampleapplication.a: $(OUT_OBJS)
$(OBJDIR)/httpexampleapplication.o: httpexampleapplication.c

httpexampleapplication.out: $(OBJDIR)/httpexampleapplication.o $(OBJDIR)/httpexampleapplication.a

include ../rulesgnu.inc
#############################################################################
