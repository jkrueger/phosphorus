# Generic build rules

define KAPUTTES_SYSTEM
@echo A dependency graph inconsistency has been detected while building $@
@echo from $<. Aborting
@exit -1
endef

define C_RECIPE
	@echo [C  ] $$<
	$(VERBOSE)mkdir -p $$(dir $$@)
	$(VERBOSE)$(CC) $(C_FLAGS) $$(c_local_flags) -c -MM -MT $$^ -MF $$(patsubst %.o,%.d,$$@) $$<
	$(VERBOSE)$(CC) $(C_FLAGS) $$(c_local_flags) -c -o $$@ $$<
endef

define CXX_RECIPE
	@echo [C++] $$<
	$(VERBOSE)mkdir -p $$(dir $$@)
	$(VERBOSE)$(CXX) $(CXX_FLAGS) $$(cxx_local_flags) $$(cxx_local_pch) -c -MM -MT $$@ -MF $$(patsubst %.o,%.d,$$@) $$<
	$(VERBOSE)$(CXX) $(CXX_FLAGS) $$(cxx_local_flags) $$(cxx_local_pch) -c -o $$@ $$<
endef

define CXX_LD_RECIPE
	@echo [LD ] $$@
	$(VERBOSE)$(CXX) $$^ $(LD_FLAGS) $$(ld_local_flags) -o $$@
endef

define PCH_RECIPE
	@echo [PCH] $$@
	$(VERBOSE)$(CXX) $(CXX_FLAGS) $$(cxx_local_flags) -MM -MT $$@ -MF $$(patsubst %.hpp.pch,%.d,$$@) $$<
	$(VERBOSE)$(CXX) $(CXX_FLAGS) $$(cxx_local_flags) -x c++-header $$< -o $$@
endef

%.o: %.cpp
	$(KAPUTTES_SYSTEM)

%.o: %.c
	$(KAPUTTES_SYSTEM)
