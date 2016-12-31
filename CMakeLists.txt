cmake_minimum_required (VERSION 2.6)

project (Srain)
set(Srain_AUTHOR "Shengyu Zhang")
set(Srain_EMAIL silverrainz@outlook.com)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

string(TOLOWER ${PROJECT_NAME} PROJECT_NAME_LOWER)

include(GetGitRevisionDescription)
git_describe(Srain_VERSION "--tags")

add_definitions(
	-DPACKAGE="${PROJECT_NAME_LOWER}"
	-DPACKAGE_NAME="${PROJECT_NAME}"
	-DPACKAGE_VERSION="${Srain_VERSION}"
	-DPACKAGE_AUTHOR="${Srain_AUTHOR}"
	-DPACKAGE_EMAIL="${Srain_EMAIL}"
	-DPACKAGE_DESC="It does not look like an IRC client."
	-DPACKAGE_WEBSITE="https://github.com/SilverRainZ/srain"
	-DPACKAGE_DATA_DIR="${CMAKE_INSTALL_PREFIX}")

set(GENERATE_RESOURCES)

# gresource
include(gresource)
find_package(gresources REQUIRED)
gresource("${CMAKE_CURRENT_SOURCE_DIR}/data/ui" srain.gresource.xml "${CMAKE_CURRENT_BINARY_DIR}" GENERATE_GRESOURCE)
list(APPEND GENERATE_RESOURCES ${GENERATE_GRESOURCE})


# sources
include_directories(src/inc)
include_directories("${CMAKE_CURRENT_BINARY_DIR}")
aux_source_directory(src DIR_SRCS)
aux_source_directory(src/srv DIR_SRCS)
aux_source_directory(src/ui DIR_SRCS)


# libs
FIND_PACKAGE(PkgConfig REQUIRED)

pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
add_definitions(${GTK3_CFLAGS_OTHER})
include_directories(${GTK3_INCLUDE_DIRS})
link_libraries(${GTK3_LIBRARIES})

find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIRS})
link_libraries(${CURL_LIBRARIES})

find_package(PythonLibs 3.2 REQUIRED)
include_directories(${PYTHON_INCLUDE_DIRS})
link_libraries(${PYTHON_LIBRARIES})

find_package(LibIRCClient 1.8 REQUIRED)
include_directories(${LibIRCClient_INCLUDE_DIRS})
link_libraries(${LibIRCClient_LIBRARY})

pkg_check_modules(Libnotify REQUIRED libnotify)
add_definitions(${Libnotify_CFLAGS_OTHER})
include_directories(${Libnotify_INCLUDE_DIRS})
link_libraries(${Libnotify_LIBRARIES})


# compile
set(GENERATE_CMD_LIST "${CMAKE_CURRENT_BINARY_DIR}/cmd_list.autogen")
add_custom_command(
	OUTPUT ${GENERATE_CMD_LIST}
	DEPENDS src/srv/srv_session_cmd.c
	WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
	COMMAND sed -r -n "'s/\(\"\\/[^\"]+\).*/\\1 \",/p'" src/srv/srv_session_cmd.c > ${GENERATE_CMD_LIST}
)
list(APPEND GENERATE_RESOURCES ${GENERATE_CMD_LIST})

add_definitions(-Os -Wall -Wno-deprecated-declarations)
add_executable(srain src/main.c ${GENERATE_RESOURCES} ${DIR_SRCS})


# install
add_subdirectory(data)
add_subdirectory(po)
install(TARGETS srain DESTINATION bin)
