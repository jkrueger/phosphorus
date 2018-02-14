include $(TOP)/build/header.mk

products_$(d) := rayray

rayray_sources_$(d) += \
	core.cpp \
        material.cpp \
	codec/image/bmp.cpp \
	codec/image/exr.cpp \
        codec/mesh/ply.cpp \
	math/parametric/sphere.cpp \
        things/mesh.cpp \
        things/scene.cpp \
        traversal/bvh.cpp \
        material/diffuse.cpp \
        material/plastic.cpp \
        material/mirror.cpp \
        material/glass.cpp

rayray_precompiled_$(d) :=
rayray_target_dir_$(d)  := bin
ifdef DEBUG
rayray_cxx_flags_$(d)   := -std=c++14 -Isrc/ -g -Ivendor/rply/src -fno-inline -march=native
else
rayray_cxx_flags_$(d)   := -std=c++14 -Isrc/ -Ivendor/rply/src -I/usr/local/include -O3 -march=native -flto -funroll-loops
endif
rayray_ld_flags_$(d)    := -L/usr/local/lib -lIlmImf -lHalf -lIex -L$(BUILD_DIR)/lib -lrply -flto -Wl,-stack_size,1000000

include $(TOP)/build/footer.mk
