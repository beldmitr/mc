cmake_minimum_required(VERSION 3.15)

project(mcpp)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++2a -lpcrecpp -lslang -lgmodule-2.0")


file(GLOB SRC
        "src/*.cpp"
#        "src/consaver/*.cpp"   # SECOND MAIN FUNCTION ???
        "src/diffviewer/*.cpp"
        "src/editor/*.cpp"
        "src/filemanager/*.cpp"
        "src/man2hlp/*.cpp"
        "src/subshell/*.cpp"
        "src/vfs/*.cpp"
        "src/vfs/cpio/*.cpp"
        "src/vfs/extfs/*.cpp"
        "src/vfs/fish/*.cpp"
        "src/vfs/ftpfs/*.cpp"
        "src/vfs/local/*.cpp"
        "src/vfs/sfs/*.cpp"
        "src/vfs/sftpfs/*.cpp"
        "src/vfs/smbfs/*.cpp"
        "src/vfs/smbfs/helpers/lib/*.cpp"
        "src/vfs/smbfs/helpers/libsmb/*.cpp"
        "src/vfs/smbfs/helpers/param/*.cpp"
        "src/vfs/tar/*.cpp"
        "src/vfs/undelfs/*.cpp"
        "src/viewer/*.cpp"
        )

file(GLOB LIB
        "lib/*.cpp"
        "lib/event/*.cpp"
        "lib/filehighlight/*.cpp"
        "lib/mcconfig/*.cpp"
        "lib/search/*.cpp"
        "lib/skin/*.cpp"
        "lib/strutil/*.cpp"
        "lib/tty/*.cpp"
        "lib/vfs/*.cpp"
        "lib/widget/*.cpp"
        )

add_executable(${PROJECT_NAME} ${SRC} ${LIB})

target_include_directories(${PROJECT_NAME} PUBLIC
        "src/"
        "src/consaver"
        "src/diffviewer"
        "src/editor"
        "src/filemanager"
        "src/subshell"
        "src/vfs"
        "src/vfs/cpio"
        "src/vfs/extfs"
        "src/vfs/fish"
        "src/vfs/ftpfs"
        "src/vfs/local"
        "src/vfs/sfs"
        "src/vfs/sftpfs"
        "src/vfs/smbfs"
        "src/vfs/smbfs/helpers/include/"
        "src/vfs/tar"
        "src/vfs/undelfs"
        "src/viewer"

        "./"
#        "lib/"  # in code there arereferences lib/*.h, so it ain't need to include lib dir
        "lib/event"
        "lib/filehighlight"
#        "lib/mcconfig" # no headers here
        "lib/search"
        "lib/skin"
#        "lib/strutil" # no headers here
        "lib/tty"
        "lib/vfs"
        "lib/widget"
        )

# maint && tests are missing

# PKG-CONFIG
include(FindPkgConfig)
find_package(PkgConfig REQUIRED)

# GLIB 2.0
pkg_check_modules(GLIB2 REQUIRED IMPORTED_TARGET glib-2.0)
target_include_directories(${PROJECT_NAME} PUBLIC ${GLIB2_INCLUDE_DIRS})
link_directories(${GLIB_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} ${GLIB2_LIBRARIES})

# Gmodule
pkg_check_modules(GMODULE2 REQUIRED IMPORTED_TARGET gmodule-2.0)
target_include_directories(${PROJECT_NAME} PUBLIC ${GMODULE2_INCLUDE_DIRS})
link_directories(${GMODULE2_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} ${GMODULE2_LIBRARIES})

# EXT2FS
pkg_check_modules(EXT2FS REQUIRED IMPORTED_TARGET ext2fs)
target_include_directories(${PROJECT_NAME} PUBLIC ${EXT2FS_INCLUDE_DIRS})
link_directories(${EXT2FS_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} ${EXT2FS_LIBRARIES})

# CURSES FIXME DO I NEED IT ??
find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${CURSES_LIBRARIES})

# ASPELL
find_package(ASPELL REQUIRED)
include_directories(${ASPELL_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${ASPELL_LIBRARIES})

# PCRE
FIND_PATH(PCRE_INCLUDE_DIR NAMES pcre.h)
FIND_LIBRARY(PCRE_LIBRARY NAMES pcre)
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(PCRE DEFAULT_MSG PCRE_LIBRARY PCRE_INCLUDE_DIR)
IF(PCRE_FOUND)
    include_directories(${PCRE_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} ${PCRE_LIBRARY})
ENDIF(PCRE_FOUND)

# SLANG
FIND_PATH(SLANG_INCLUDE_DIR NAMES slang.h)
FIND_LIBRARY(SLANG_LIBRARY NAMES slang)
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(SLANG DEFAULT_MSG SLANG_LIBRARY SLANG_INCLUDE_DIR)
IF(SLANG_FOUND)
    include_directories(${SLANG_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} ${SLANG_LIBRARY})
ENDIF(SLANG_FOUND)

# LIBSSH2
pkg_check_modules(LIBSSH2 REQUIRED IMPORTED_TARGET libssh2)
target_include_directories(${PROJECT_NAME} PUBLIC ${LIBSSH2_INCLUDE_DIRS})
link_directories(${LIBSSH2_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} ${LIBSSH2_LIBRARIES})

# DEFINITIONS
add_compile_definitions(VERSION="0.0")
add_compile_definitions(HAVE_SLANG)
add_compile_definitions(ENABLE_BACKGROUND)
add_compile_definitions(HAVE_SYS_IOCTL_H)
add_compile_definitions(HAVE_FCNTL_H)
add_compile_definitions(SEARCH_TYPE_PCRE)
add_compile_definitions(USE_DIFF_VIEW)
add_compile_definitions(USE_INTERNAL_EDIT)
add_compile_definitions(HAVE_CHARSET)
add_compile_definitions(HAVE_ASPELL)
add_compile_definitions(ENABLE_SUBSHELL)
add_compile_definitions(USE_STATVFS=1)
add_compile_definitions(STAT_STATVFS)   # vs STAT_STATVFS64 ???
add_compile_definitions(HAVE_UTIME_H) # TODO ?? HAVE_UTIME_H vs. HAVE_UTIMENSAT
add_compile_definitions(ENABLE_VFS_NET)
add_compile_definitions(HAVE_SOCKLEN_T)
add_compile_definitions(HAVE_ARPA_INET_H)
add_compile_definitions(HAVE_SYS_PARAM_H)
add_compile_definitions(HAVE_SYS_SELECT_H)
add_compile_definitions(ENABLE_VFS_FTP)
add_compile_definitions(NO_CONFIG_H)
add_compile_definitions(HAVE_STDARG_H)
add_compile_definitions(HAVE_EXT2FS_EXT2_FS_H)
add_compile_definitions(HAVE_STRINGS_H)
add_compile_definitions(HAVE_STRING_H)  # FIXME !!! better use cstring instead
add_compile_definitions(HAVE_GETTIMEOFDAY_TZ)

#curses
#add_compile_definitions(HAVE_NCURSES_H)
#add_compile_definitions(USE_NCURSESW)
#add_compile_definitions(USE_NCURSES)

# SIG_ATOMIC_VOLATILE
# Some systems declare sig_atomic_t as volatile, some others -- no.
# TODO Choose correct
add_compile_definitions(SIG_ATOMIC_VOLATILE_T=sig_atomic_t)
#add_compile_definitions(SIG_ATOMIC_VOLATILE_T=volatile sig_atomic_t)

add_compile_definitions(HAVE_SYS_UCRED_H)


add_compile_definitions(SAVERDIR="/usr/local/libexec/mc/")
add_compile_definitions(SYSCONFDIR="/usr/local/etc/mc/")
add_compile_definitions(DATADIR="/usr/local/share/mc/")
add_compile_definitions(EXTHELPERSDIR="/usr/local/libexec/mc/ext.d/")
add_compile_definitions(LIBEXECDIR="/usr/local/libexec/mc/")
add_compile_definitions(DRIVERFILE="")  # FIXME !!!!! missing correct path: ?? $prefix/lib/printers.def
add_compile_definitions(CONFIGDIR="")   # FIXME figure out correct path
add_compile_definitions(PACKAGE="")        # FIXME figure out wtf is package
add_compile_definitions(LMHOSTSFILE="/usr/local/lib/lmhosts") # TODO check me
add_compile_definitions(CODEPAGEDIR="") # FIXME ???

add_compile_definitions(PROMOTED_MODE_T=mode_t)