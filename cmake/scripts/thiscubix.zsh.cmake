#
# Source this file to set up the environment to work with Cubix
#

if [ -z "${PATH}" ]; then
   PATH=@CX_BIN_DIR@; export PATH
else
   PATH=@CX_BIN_DIR@:$PATH; export PATH
fi

# Linux, ELF HP-UX
if [ -z "${LD_LIBRARY_PATH}" ];  then
  LD_LIBRARY_PATH=@CX_LIB_DIR@; export LD_LIBRARY_PATH
else
  LD_LIBRARY_PATH=@CX_LIB_DIR@:$LD_LIBRARY_PATH ; export LD_LIBRARY_PATH  # Linux, ELF HP-UX
fi

# Mac OS X
if [ -z "${DYLD_LIBRARY_PATH}" ];  then
  DYLD_LIBRARY_PATH=@CX_LIB_DIR@; export DYLD_LIBRARY_PATH
else
  DYLD_LIBRARY_PATH=@CX_LIB_DIR@:$DYLD_LIBRARY_PATH  ; export DYLD_LIBRARY_PATH
fi

CUBIX_SYS=@CX_INSTALL_DIR@ ; export CUBIX_SYS

# for ROOT6: need to set ROOT_INCLUDE_PATH environment variable
# otherwise class dictionaries will not load correctly
if [ -z "${ROOT_INCLUDE_PATH}" ]; then
   ROOT_INCLUDE_PATH=`$CUBIX_SYS/bin/cubix-config --incdir`; export ROOT_INCLUDE_PATH
else
   ROOT_INCLUDE_PATH=`$CUBIX_SYS/bin/cubix-config --incdir`:$ROOT_INCLUDE_PATH; export ROOT_INCLUDE_PATH
fi
if [ ! -z "@EXTRA_ROOT_INCLUDE_PATH@" ]; then
   ROOT_INCLUDE_PATH=@EXTRA_ROOT_INCLUDE_PATH@$ROOT_INCLUDE_PATH; export ROOT_INCLUDE_PATH
fi

source @CX_BIN_DIR@/cubix-autocompletion.zsh
source @TKN_SYS_DIR@/bin/thistkn.zsh
echo ' ---> You are working now with Cubix installed in ' @CX_INSTALL_DIR@
