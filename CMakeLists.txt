cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)

project(voip_gw)

#set(CMAKE_FRAMEWORK_PATH
#    /home/user/opal/lib)

find_package(PkgConfig REQUIRED)
pkg_search_module(PTLIB REQUIRED ptlib)
pkg_search_module(OPAL REQUIRED opal)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -W -Wall -Wno-unused-parameter -Werror=return-type -Werror=reorder -Wno-switch -Wno-deprecated-declarations")

include_directories(
    ${PROJECT_SOURCE_DIR}
    ${PTLIB_INCLUDE_DIRS}
    ${OPAL_INCLUDE_DIRS}
    )
    
link_directories(
    ${PTLIB_LIBRARY_DIRS}
    ${OPAL_LIBRARY_DIRS}
    )

set(SOURCES
    main.cpp
    stream_metrics.cpp
    video_frame_info.cpp
    audio_frame_info.cpp
    smart_buffer.cpp
    const_data_buffer.cpp
    mutable_data_buffer.cpp
    audio_frame_impl.cpp
    video_frame_impl.cpp
    voip_control.cpp
    codec_info.cpp
    call_manager_config.cpp
    engine_impl.cpp
    utils/frame_queue.cpp
    utils/adaptive_delay.cpp
    utils/common_utils.cpp
    opal/opal_utils.cpp
    opal/opal_types.cpp
    opal/opal_video_frame.cpp
    opal/opal_factory.cpp
    opal/opal_call.cpp
    opal/opal_media_session.cpp
    opal/opal_media_stream.cpp
    opal/opal_manager_config.cpp
    c_api/voip_api.h
)

set(PUBLIC_HEADERS
    common_types.h
    voip_types.h
    media_types.h
    i_engine.h
    i_data_buffer.h
    i_call.h
    i_call_manager.h
    i_call_factory.h
    i_media_session.h
    i_media_stream.h
    i_media_format.h
    i_media_frame.h
    i_audio_frame.h
    i_video_frame.h
    codec_info.h
    call_manager_config.h
    audio_frame_info.h
    video_frame_info.h
    stream_metrics.h
    opal/opal_types.h
    opal/opal_manager_config.h
)

set(PRIVATE_HEADERS
    smart_buffer.h
    const_data_buffer.h
    mutable_data_buffer.h
    audio_frame_impl.h
    video_frame_impl.h
    voip_control.h
    engine_impl.h
    c_api/voip_api_impl.cpp
    utils/frame_queue.h
    utils/adaptive_delay.h
    utils/common_utils.h
    opal/opal_utils.h
    opal/opal_video_frame.h
    opal/opal_factory.h
    opal/opal_call.h
    opal/opal_media_session.h
    opal/opal_media_stream.h
)

add_library(${PROJECT_NAME}
    ${SOURCES}
    ${PRIVATE_HEADERS}
    ${PUBLIC_HEADERS}
)

target_link_libraries(${PROJECT_NAME}
    ${PTLIB_LIBRARIES}
    ${OPAL_LIBRARIES}
    )

add_subdirectory(test)


    




