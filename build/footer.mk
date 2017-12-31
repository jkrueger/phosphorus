# Build description footer include

define RECIPES
    __t := $(strip $1)
    __odir := $(BUILD_DIR)/$(d)/$$(__t)
    __tdir := $(BUILD_DIR)/$($(strip $1)_target_dir_$(d))
    __qtgt := $$(__tdir)/$$(__t)
    $$(__t)_needs_$(d) := $$($$(__t)_needs_$(d):%=$(BUILD_DIR)/%)
    $$(__t)_pch_$(d) := $$($$(__t)_precompiled_$(d):%.hpp=%.hpp.pch)
    $$(__t)_pch_$(d) := $$($$(__t)_pch_$(d):%=$(BUILD_DIR)/$(d)/$$(__t)/%)
    $$(__t)_objects_$(d) := $$($$(__t)_sources_$(d):%.cpp=%.o)
    $$(__t)_objects_$(d) := $$($$(__t)_objects_$(d):%.c=%.o)
    $$(__t)_objects_$(d) := $$($$(__t)_objects_$(d):%=$(BUILD_DIR)/$(d)/$$(__t)/%)
    $$(__t)_deps_$(d)    := $$($$(__t)_sources_$(d):%.cpp=%.d)
		$$(__t)_deps_$(d)    := $$($$(__t)_deps_$(d):%.c=%.d)
    $$(__t)_deps_$(d)    += $$($$(__t)_precompiled_$(d):%.hpp=%.d)
    $$(__t)_deps_$(d)    := $$($$(__t)_deps_$(d):%=$(BUILD_DIR)/$(d)/$$(__t)/%)
    $$(__odir) $$(__tdir)::
	$(VERBOSE)mkdir -p $$@
    $$(__qtgt): $$($$(__t)_needs_$(d)) $$($$(__t)_objects_$(d)) | $$(__tdir)
	$(CXX_LD_RECIPE)
    $$(__qtgt): c_local_flags := $$($$(__t)_c_flags_$(d))
    $$(__qtgt): cxx_local_flags := $$($$(__t)_cxx_flags_$(d))
    $$(__qtgt): cxx_local_pch   := $$($$(__t)_pch_$(d):%=-include-pch %)
    $$(__qtgt): ld_local_flags  := $$($$(__t)_ld_flags_$(d))
    $$(__odir)/%.hpp.pch: $(d)/src/%.hpp
	$(PCH_RECIPE)
    $$(__odir)/%.o: $(d)/src/%.cpp | $$(__odir) $$($$(__t)_pch_$(d))
	$(CXX_RECIPE)
    $$(__odir)/%.o: $(d)/src/%.c | $$(__odir)
	$(C_RECIPE)
    -include $$($$(__t)_deps_$(d))
    all_targets += $$(__qtgt)
    all_objects += $$($$(__t)_objects_$(d))
    all_deps    += $$($$(__t)_deps_$(d))
    all_headers += $$($$(__t)_pch_$(d))
endef

$(foreach product,$(products_$(d)), \
    $(eval \
        $(call RECIPES, $(product)))\
)

d  := $(stack_$(sp))
sp := $(basename $(sp))
