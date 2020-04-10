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