cmake_minimum_required(VERSION 3.24)
project(WinService LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(CXX_SOURCE_FILES
    main.cpp
    ThreadPool.h
    ServiceBase.h ServiceBase.cpp
    SampleService.h SampleService.cpp
    ServiceInstaller.h ServiceInstaller.cpp
)

add_executable(${PROJECT_NAME} ${CXX_SOURCE_FILES})