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
