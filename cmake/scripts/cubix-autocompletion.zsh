#!/bin/zsh

# Init completion system
autoload -U compinit
compinit -D

_cubix_config_completion() {
    _arguments \
  	'--version[Print the Cubix version]' \
  	'--gitinfos[Print the git branch and commit]' \
  	'--bindir[Print the executable directory]' \
  	'--libdir[Print the library directory]' \
  	'--incdir[Print the header directory]' \
  	'--srcdir[Print the source directory]' \
  	'--builddir[Print the CMake build directory]' \
  	'--libs[Print linker directives for all libraries]' \
  	'--cflags[Print all flags for compiling (including ROOT flags)]' \
  	'--help[print help]' \
}
compdef _cubix_config_completion cubix-config