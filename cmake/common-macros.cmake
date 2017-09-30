
macro(add_project name)
    set(PROJECT_NAME "${name}")
    message(STATUS "Creating project ${PROJECT_NAME}")

    project(${name} LANGUAGES CXX C)

endmacro()