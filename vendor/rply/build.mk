include $(TOP)/build/header.mk

products_$(d) := librply.dylib

librply.dylib_sources_$(d) += \
        rply.c

librply.dylib_precompiled_$(d) :=
librply.dylib_target_dir_$(d)  := lib
librply.dylib_c_flags_$(d)     := -fPIC 
librply.dylib_ld_flags_$(d)    := -rdynamic -shared -undefined dynamic_lookup

include $(TOP)/build/footer.mk
