get_target_property(_qmake_executable Qt5::qmake IMPORTED_LOCATION)
get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
get_target_property(QtCore_location_Debug Qt5::Core LOCATION_Debug)
get_target_property(QtCore_location_Release Qt5::Core LOCATION_Release)
get_filename_component(QtFolder ${QtCore_location_Debug} DIRECTORY)

message("Path of QtFolder : " ${QtFolder})

function(osdeployqt target debug)

    # POST_BUILD step
    # - after build, we have a bin/lib for analyzing qt dependencies
    # - we run windeployqt on target and deploy Qt libs
    if(debug)
        set(COMPILE_MODE "--debug")
        message("Path of QtCore_location_Debug : " ${QtCore_location_Debug})
    else()
        set(COMPILE_MODE "--release")
        message("Path of QtCore_location_Release : " ${QtCore_location_Release})
    endif()

    message("")

    if(WIN32)
        set(OS_DEPLOY_APP_NAME "windeployqt.exe")
    elseif(LINUX)
        set(OS_DEPLOY_APP_NAME "linuxdeployqt")
    elseif(MACOS)
        set(OS_DEPLOY_APP_NAME "macdeployqt")
    endif()
        
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND "${_qt_bin_dir}/${OS_DEPLOY_APP_NAME}"
                --verbose 2
                --dir ${QtFolder}
                $<IF:$<CONFIG:Debug>,--debug,--release>
                #--no-svg
                #--no-angle
                #--no-opengl
                #--no-opengl-sw
                #--no-compiler-runtime
                #--compiler-runtime
                #--no-system-d3d-compiler
                \"$<TARGET_FILE:${target}>\"
        COMMENT "Deploying Qt libraries using windeployqt for compilation target '${target}' ..."
    )

endfunction()