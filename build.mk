include $(TOP)/build/header.mk

products_$(d) := manta

manta_sources_$(d) += \
	core.cpp \
	codec/image/bmp.cpp \
	codec/image/exr.cpp \
	things/sphere.cpp \
        things/plane.cpp \
        things/bvh.cpp \
        material/diffuse.cpp \
        material/plastic.cpp \
        material/mirror.cpp \
        material/glass.cpp

manta_precompiled_$(d) :=
manta_target_dir_$(d)  := bin
ifdef DEBUG
manta_cxx_flags_$(d)   := -std=c++14 -Isrc/ -g
else
manta_cxx_flags_$(d)   := -std=c++14 -Isrc/ -O3
endif
manta_ld_flags_$(d)    := -lIlmImf -lHalf -lIex

include $(TOP)/build/footer.mk
