# configure.mk

include etc/config/native.mk
include etc/config/tiny.mk

CC_NATIVE+=$(foreach U,$(OPT_ENABLE_NATIVE),-DPO_USE_$U=1)
