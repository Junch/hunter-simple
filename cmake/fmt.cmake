FetchContent_Declare(
        fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt.git
        GIT_TAG        5.2.1
)

FetchContent_GetProperties(fmt)
if (NOT fmt_POPULATED)
    message(STATUS "Populating build dependency: fmt")
    FetchContent_Populate(fmt)
    add_subdirectory(${fmt_SOURCE_DIR} ${fmt_BINARY_DIR})
endif ()

FetchContent_Declare(
        sigslot
        GIT_REPOSITORY https://github.com/palacaze/sigslot.git
        GIT_TAG        v1.0.1
)

FetchContent_GetProperties(sigslot)
if (NOT sigslot_POPULATED)
    message(STATUS "Populating build dependency: sigslot")
    FetchContent_Populate(sigslot)
    add_subdirectory(${sigslot_SOURCE_DIR} ${sigslot_BINARY_DIR})
endif ()
