#底下有目录的，用此处
ifndef PDIR
UP_EXTRACT_DIR = ..
GEN_LIBS = lib_common.a
COMPONENTS_lib_common = Misc/lib_misc.a	\
                        Database/lib_database.a \
                        InfoSave/lib_infosave.a \
                        Q_Lib/lib_qlib.a \
                        tcp_conn/lib_tcp_conn.a \
                        http_server/lib_http_server.a 
endif

INCLUDES := $(INCLUDES) -I $(PDIR)include
INCLUDES += -I ./
PDIR := ../$(PDIR)
sinclude $(PDIR)Makefile

