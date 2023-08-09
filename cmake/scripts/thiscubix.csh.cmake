#
# Source this file to set up the environment to work with Cubix
#
if ($?PATH) then
        setenv PATH ${PATH}:@CX_BIN_DIR@
else
        setenv PATH @CX_BIN_DIR@
endif

# Linux, ELF HP-UX
if ($?LD_LIBRARY_PATH) then
        setenv LD_LIBRARY_PATH ${LD_LIBRARY_PATH}:@CX_LIB_DIR@
else
        setenv LD_LIBRARY_PATH @CX_LIB_DIR@
endif

# Mac OS X
if ($?DYLD_LIBRARY_PATH) then
        setenv DYLD_LIBRARY_PATH ${DYLD_LIBRARY_PATH}:@TCX_LIB_DIR@
else
        setenv DYLD_LIBRARY_PATH @CX_LIB_DIR@
endif

setenv CUBIX_SYS @CX_INSTALL_DIR@

source @TKN_SYS_DIR@/bin/thistkn.csh
echo ' ---> You are working now with Cubix installed in ' @CX_INSTALL_DIR@
