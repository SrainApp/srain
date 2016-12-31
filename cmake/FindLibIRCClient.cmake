# Once done, this will define
#
#  LibIRCClient_FOUND - system has LibIRCClient
#  LibIRCClient_INCLUDE_DIRS - the LibIRCClient include directories
#  LibIRCClient_LIBRARIES - link these to use LibIRCClient

include(LibFindMacros)

# Include dir
find_path(LibIRCClient_INCLUDE_DIR
  NAMES libircclient.h
  PATHS ${LibIRCClient_PKGCONF_INCLUDE_DIRS}
      /usr/include /sw/include /usr/local/include
      /usr/include/libircclient /sw/include/libircclient
      /usr/local/include/libircclient
)

# Finally the library itself
find_library(LibIRCClient_LIBRARY
  NAMES ircclient
  PATHS ${LibIRCClient_PKGCONF_LIBRARY_DIRS}
      /usr/lib /lib /sw/lib /usr/local/lib
)

set(LibIRCClient_PROCESS_INCLUDES LibIRCClient_INCLUDE_DIR)
set(LibIRCClient_PROCESS_LIBS LibIRCClient_LIBRARY)
libfind_process(LibIRCClient)
