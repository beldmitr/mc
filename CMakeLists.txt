cmake_minimum_required(VERSION 3.15)

project(mcpp)

file(GLOB SRC
        "src/*.cpp"
        "src/consaver/*.cpp"
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
        "src/vfs/smbfs/helpers/lib*.cpp"
        "src/vfs/smbfs/helpers/libsmb*.cpp"
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
        "src/vfs/smbfs/helpers/include"
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