cmake_minimum_required (VERSION 3.5.1)

project (dvb-ssu-generator)

set(ISSU_GENERATOR_SRC
	ISSU_Generator/MainApp.cpp
    ISSU_Generator/MainConf.cpp
    ISSU_Generator/MainFrame.cpp
    )
set(ISSU_GENERATOR_SRC_CLI
	ISSU_Generator/MainCliApp.cpp
    ISSU_Generator/MainConf.cpp
    ISSU_Generator/MainFrame.cpp
	)

set(LIB_DATA_CAROUSEL_SRC
    DataCarousel/DataCarousel.cpp
    DataCarousel/DDB.cpp
    DataCarousel/DESC.cpp
    DataCarousel/DII.cpp
    DataCarousel/DSI.cpp
    DataCarousel/DSMCC.cpp
    DataCarousel/Group.cpp
    DataCarousel/Module.cpp
    DataCarousel/ModuleData.cpp
    DataCarousel/SuperGroup.cpp
    )

set(LIB_GENIUS_CAROUSEL_SRC
    GeniusCarousel/GeniusCarousel.cpp
    GeniusCarousel/GeniusImagesData.cpp
    GeniusCarousel/GeniusSignTool.cpp
    )

set(LIB_OBJECT_CAROUSEL_SRC
    ObjectCarousel/BIOP.cpp
    ObjectCarousel/CORBA.cpp
    ObjectCarousel/ModuleData.cpp
    ObjectCarousel/ObjectCarousel.cpp
    )

set(LIB_DVB_SRC
    DVB/BAT.cpp
    DVB/Date.cpp
    DVB/DESC.cpp
    DVB/NIT.cpp
    DVB/SDT.cpp
    DVB/TDT.cpp
    )

set(LIB_MPEG_SRC
    MPEG/DESC.cpp
    MPEG/MPEG.cpp
    MPEG/Multiplexer.cpp
    MPEG/MuxStream.cpp
    MPEG/PAT.cpp
    MPEG/PMT.cpp
    )

set(LIB_SSU_SRC
    SSU/DESC.cpp
    SSU/UNT.cpp
    )

set(LIB_TOOLS_SRC
    Tools/Config.cpp
    Tools/Conversion.cpp
    Tools/CRC.cpp
    Tools/Event.cpp
    Tools/File.cpp
    Tools/FileManager.cpp
    Tools/FileMirror.cpp
    Tools/Header.cpp
    Tools/Log.cpp
    Tools/Scheduler.cpp
    Tools/Tlv.cpp
    Tools/Tools.cpp
    Tools/Validator.cpp
    Tools/ValidatorStd.cpp
    Tools/Xml.cpp
    Tools/Expat/xmlparse/xmlparse.c
    Tools/Expat/xmltok/xmltok.c
    Tools/Expat/xmltok/xmlrole.c
    )

set(LIB_TOOLS_GUI_SRC
    ToolsGui/Dialog.cpp
    ToolsGui/DialogStack.cpp
    ToolsGui/Log.cpp
    ToolsGui/Window.cpp
    )

set(LIB_TS_WRITER_SRC
    TS_Writer/Config.cpp
    TS_Writer/TS_Writer.cpp
    )

find_package(wxWidgets REQUIRED core base adv)
include(${wxWidgets_USE_FILE})

find_package(ZLIB REQUIRED)
find_package(OpenSSL REQUIRED)

if(OPENSSL_VERSION VERSION_LESS "1.1.0")
set(LIB_SSL_COMPATIBILITY
    LibCriptoCompat/libcrypto-compat.c
    )
endif()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${ZLIB_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
)

add_definitions(-D__UNIX__)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -g3 -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_C_FLAGS}")

set (RESOURCES_SRC "${CMAKE_SOURCE_DIR}/Resources")
set (RESOURCES_DST "${CMAKE_CURRENT_BINARY_DIR}/Resources")

add_executable(${PROJECT_NAME}
    ${ISSU_GENERATOR_SRC}
    ${LIB_DATA_CAROUSEL_SRC}
    ${LIB_GENIUS_CAROUSEL_SRC}
    ${LIB_OBJECT_CAROUSEL_SRC}
    ${LIB_DVB_SRC}
    ${LIB_MPEG_SRC}
    ${LIB_SSU_SRC}
    ${LIB_TOOLS_SRC}
    ${LIB_TOOLS_GUI_SRC}
    ${LIB_TS_WRITER_SRC}
    ${LIB_SSL_COMPATIBILITY}
    )
add_executable(dvb-ssu-generator-cli
    ${ISSU_GENERATOR_SRC_CLI}
    ${LIB_DATA_CAROUSEL_SRC}
    ${LIB_GENIUS_CAROUSEL_SRC}
    ${LIB_OBJECT_CAROUSEL_SRC}
    ${LIB_DVB_SRC}
    ${LIB_MPEG_SRC}
    ${LIB_SSU_SRC}
    ${LIB_TOOLS_SRC}
    ${LIB_TOOLS_GUI_SRC}
    ${LIB_TS_WRITER_SRC}
    ${LIB_SSL_COMPATIBILITY}
	)



target_link_libraries(${PROJECT_NAME}
    ${wxWidgets_LIBRARIES}
    ${ZLIB_LIBRARIES}
    ${OPENSSL_CRYPTO_LIBRARY}
    )

target_link_libraries(dvb-ssu-generator-cli
    ${wxWidgets_LIBRARIES}
    ${ZLIB_LIBRARIES}
    ${OPENSSL_CRYPTO_LIBRARY}
    )
add_custom_command(
 TARGET ${PROJECT_NAME} POST_BUILD
 COMMAND ${CMAKE_COMMAND} -E create_symlink ${RESOURCES_SRC} ${RESOURCES_DST}
 DEPENDS ${RESOURCES_DST}
 COMMENT "symbolic link resources folder from ${RESOURCES_SRC} => ${RESOURCES_DST}"
)
add_custom_command(
 TARGET dvb-ssu-generator-cli POST_BUILD
 COMMAND ${CMAKE_COMMAND} -E create_symlink ${RESOURCES_SRC} ${RESOURCES_DST}
 DEPENDS ${RESOURCES_DST}
 COMMENT "symbolic link resources folder from ${RESOURCES_SRC} => ${RESOURCES_DST}"
)
file(COPY ${CMAKE_SOURCE_DIR}/Mkfile 
		DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
