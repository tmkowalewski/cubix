include(CMakeParseArguments)

#---------------------------------------------------------------------------------------------------
#---GENERATE_ROOT_DICTIONARY(libname LINKDEF [name of LinkDef.h] HEADERS [toto.h titi.h ...] DEPENDENCIES [lib1 lib2])
#
#---Generate ROOT dictionary & rootmap.
#   For ROOT6 we generate the .pcm file too.
#   rootmap and pcm will be installed in ${CMAKE_INSTALL_LIBDIR}
#---------------------------------------------------------------------------------------------------

function(GENERATE_ROOT_DICTIONARY libname)
    CMAKE_PARSE_ARGUMENTS(ARG "" "LINKDEF" "HEADERS;SOURCES;DEPENDENCIES" ${ARGN})

    #--remove source path from header filenames to be given to dictionary generator
    set(dictgen_headers)
    foreach(head ${ARG_HEADERS})
      get_filename_component(no_dir_head ${head} NAME)
      set(dictgen_headers ${dictgen_headers} ${no_dir_head})
    endforeach()

    ROOT_GENERATE_DICTIONARY(G__${libname} ${dictgen_headers} MODULE ${libname} LINKDEF ${ARG_LINKDEF} OPTIONS -noIncludePaths)
    set_source_files_properties(${libname}_rdict.pcm ${libname}.rootmap PROPERTIES GENERATED TRUE)
    set_source_files_properties(G__${libname}.cxx G__${libname}.h  PROPERTIES GENERATED TRUE)

    ROOT_LINKER_LIBRARY(${libname} ${ARG_SOURCES} G__${libname}.cxx DEPENDENCIES ${ARG_DEPENDENCIES})
endfunction()
