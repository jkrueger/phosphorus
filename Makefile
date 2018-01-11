include build/core.mk

CC  := clang 
CXX := clang++ 

$(eval $(call SUBDIR, vendor/rply))
$(eval $(call SUBDIR, .))

include build/targets.mk
