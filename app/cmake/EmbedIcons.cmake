cmake_minimum_required(VERSION 3.20)

if(WIN32)
    target_sources(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../assets/icons/icon.rc")
elseif(APPLE)
    set(ICON_ICNS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../assets/icons/icon.icns")

    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_ICON_FILE icon.icns
    )

    target_sources(MyApp PRIVATE ${ICON_ICNS_PATH})

    set_source_files_properties(${ICON_ICNS_PATH} PROPERTIES
        MACOSX_PACKAGE_LOCATION "Resources"
    )
endif()
