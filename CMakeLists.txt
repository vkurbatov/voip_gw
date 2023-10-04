cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)

project(voipgw
    DESCRIPTION "voipgw library"
    VERSION 1.0.0)

#set(CMAKE_FRAMEWORK_PATH
#    /home/user/opal/lib)

find_package(PkgConfig REQUIRED)
pkg_search_module(PTLIB REQUIRED ptlib)
pkg_search_module(OPAL REQUIRED opal)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -W -Wall -Wno-unused-parameter -Werror=return-type -Werror=reorder -Wno-switch -Wno-deprecated-declarations")

include_directories(
    ${PROJECT_SOURCE_DIR}/src
    ${PTLIB_INCLUDE_DIRS}
    ${OPAL_INCLUDE_DIRS}
    )
    
link_directories(
    ${PTLIB_LIBRARY_DIRS}
    ${OPAL_LIBRARY_DIRS}
    )

set(SOURCES
    src/main.cpp
    src/stream_metrics.cpp
    src/video_frame_info.cpp
    src/audio_frame_info.cpp
    src/smart_buffer.cpp
    src/const_data_buffer.cpp
    src/mutable_data_buffer.cpp
    src/audio_frame_impl.cpp
    src/video_frame_impl.cpp
    src/voip_control.cpp
    src/codec_info.cpp
    src/call_manager_config.cpp
    src/engine_impl.cpp
    src/utils/frame_queue.cpp
    src/utils/adaptive_delay.cpp
    src/utils/common_utils.cpp
    src/opal/opal_utils.cpp
    src/opal/opal_types.cpp
    src/opal/opal_video_frame.cpp
    src/opal/opal_factory.cpp
    src/opal/opal_call.cpp
    src/opal/opal_media_session.cpp
    src/opal/opal_media_stream.cpp
    src/opal/opal_manager_config.cpp
    src/c_api/voip_api_impl.cpp
)

set(PUBLIC_HEADERS
    src/common_types.h
    src/voip_types.h
    src/media_types.h
    src/i_engine.h
    src/i_data_buffer.h
    src/i_call.h
    src/i_call_manager.h
    src/i_call_factory.h
    src/i_media_session.h
    src/i_media_stream.h
    src/i_media_format.h
    src/i_media_frame.h
    src/i_audio_frame.h
    src/i_video_frame.h
    src/codec_info.h
    src/call_manager_config.h
    src/audio_frame_info.h
    src/video_frame_info.h
    src/stream_metrics.h
    src/voip_control.h
    src/c_api/voip_api.h
    src/opal/opal_types.h
    src/opal/opal_manager_config.h
)

set(UTILS_HEADER
    src/utils/frame_queue.h
    src/utils/adaptive_delay.h
    src/utils/common_utils.h
    )

set(PRIVATE_HEADERS
    src/smart_buffer.h
    src/const_data_buffer.h
    src/mutable_data_buffer.h
    src/audio_frame_impl.h
    src/video_frame_impl.h
    src/engine_impl.h
    src/opal/opal_utils.h
    src/opal/opal_video_frame.h
    src/opal/opal_factory.h
    src/opal/opal_call.h
    src/opal/opal_media_session.h
    src/opal/opal_media_stream.h
)

add_library(${PROJECT_NAME} SHARED
    ${SOURCES}
    ${UTILS_HEADER}
    ${PRIVATE_HEADERS}
    ${PUBLIC_HEADERS}
)

target_link_libraries(${PROJECT_NAME}
    ${PTLIB_LIBRARIES}
    ${OPAL_LIBRARIES}
    )

configure_file(pc/voipgw.pc.in voipgw.pc @ONLY)

set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS};${UTILS_HEADER}")

install(TARGETS ${PROJECT_NAME}
        LIBRARY
        PUBLIC_HEADER DESTINATION include/${PROJECT_NAME})
install(
        FILES ${CMAKE_BINARY_DIR}/voipgw.pc DESTINATION lib/pkgconfig)


    




