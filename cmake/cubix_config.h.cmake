#ifndef CX_CONFIG_H
#define CX_CONFIG_H

#define Cubix_Version @Cubix_VERSION@
#define Cubix_Version_Major @Cubix_VERSION_MAJOR@
#define Cubix_Version_Minor @Cubix_VERSION_MINOR@

/////////////////////////////
/////// Define Paths ////////
/////////////////////////////

#define CUBIX_SYS getenv("CUBIX_SYS")

/////////////////////////////
////// Define OS_TYPE ///////
/////////////////////////////

#define OS_LINUX   1
#define OS_WINDOWS 2
#define OS_APPLE   3

#define OS_TYPE @CMAKE_OS_TYPE@


/////////////////////////////
////// Define OPTIONS ///////
/////////////////////////////

#cmakedefine HAS_MATHMORE

#endif
