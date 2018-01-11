include $(TOP)/build/header.mk

products_$(d) := rayray

rayray_sources_$(d) += \
	core.cpp \
	codec/image/bmp.cpp \
	codec/image/exr.cpp \
        codec/mesh/ply.cpp \
	things/sphere.cpp \
        things/plane.cpp \
        things/mesh.cpp \
        things/bvh.cpp \
        material/diffuse.cpp \
        material/plastic.cpp \
        material/mirror.cpp \
        material/glass.cpp

rayray_precompiled_$(d) :=
rayray_target_dir_$(d)  := bin
ifdef DEBUG
rayray_cxx_flags_$(d)   := -std=c++14 -Isrc/ -g -Ivendor/rply/src
else
rayray_cxx_flags_$(d)   := -std=c++14 -Isrc/ -Ivendor/rply/src -O3
endif
rayray_ld_flags_$(d)    := -lIlmImf -lHalf -lIex -L$(BUILD_DIR)/lib -lrply

include $(TOP)/build/footer.mk
