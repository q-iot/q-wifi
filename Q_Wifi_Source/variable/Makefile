#底下没目录的，此处填lib+目录名.a
ifndef PDIR
GEN_LIBS = lib_variable.a
endif

INCLUDES := $(INCLUDES) -I $(PDIR)include
INCLUDES += -I ./
PDIR := ../$(PDIR)
sinclude $(PDIR)Makefile

