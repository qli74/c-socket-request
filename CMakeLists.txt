cmake_minimum_required(VERSION 3.2)

PROJECT(geturl)

INCLUDE_DIRECTORIES(
include
)

AUX_SOURCE_DIRECTORY(src DIR_SRCS)

SET(GET_URL
${DIR_SRCS}
)

ADD_EXECUTABLE(${PROJECT_NAME} ${GET_URL})


TARGET_LINK_LIBRARIES(${PROJECT_NAME} m)