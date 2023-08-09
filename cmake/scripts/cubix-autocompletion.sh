#!/bin/bash

_cubix_config_app="cubix-config"

_cubix_config_completion()
{
  cur=${COMP_WORDS[COMP_CWORD]}

  # List of options
  options=""
  options="$options --version"
  options="$options --gitinfos"
  options="$options --bindir"
  options="$options --libdir"
  options="$options --incdir"
  options="$options --srcdir"
  options="$options --builddir"
  options="$options --libs"
  options="$options --cflags"
  options="$options --help"

  local prev
  prev=${COMP_WORDS[COMP_CWORD-1]}

  COMPREPLY=( $( compgen -W "$options" -- $cur ) )

}

complete -o default -o nospace -F _cubix_config_completion $_cubix_config_app
