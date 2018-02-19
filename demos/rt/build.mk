include $(TOP)/build/header.mk

products_$(d) := rt_scene

rt_scene_sources_$(d) += core.cpp

rt_scene_precompiled_$(d) :=
rt_scene_target_dir_$(d)  := bin
ifdef DEBUG
rt_scene_cxx_flags_$(d)   := -std=c++14 -Isrc/ -g -Ivendor/rply/src -fno-inline -march=native
else
rt_scene_cxx_flags_$(d)   := -std=c++14 -Isrc/ -Ivendor/rply/src -I/usr/local/include -O3 -flto -march=native
endif
rt_scene_ld_flags_$(d)    := -L/usr/local/lib -lIlmImf -lHalf -lIex -L$(BUILD_DIR)/lib -lrply -flto -Wl,-stack_size,2000000

include $(TOP)/build/footer.mk
