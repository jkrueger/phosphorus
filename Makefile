include build/core.mk

CC  := clang 
CXX := clang++ 

$(eval $(call SUBDIR, .))

include build/targets.mk
