if (NOT EXISTS "${PROJECT_SOURCE_DIR}/deps")
    file(MAKE_DIRECTORY "${PROJECT_SOURCE_DIR}/deps")
endif()

SET(TENSORFLOW_ARCHIVE "${PROJECT_SOURCE_DIR}/deps/libtensorflow_cc.tar.gz")
SET(TENSORFLOW_ARCHIVE_HASH "${PROJECT_SOURCE_DIR}/deps/libtensorflow_cc.tar.gz.sha256")
if (NOT EXISTS "${PROJECT_SOURCE_DIR}/deps/libtensorflow_cc")
    message("Downloading Tensorflow...")
    file(DOWNLOAD "https://share.jtcat.com/libtensorflow_cc.tar.gz.sha256" ${TENSORFLOW_ARCHIVE_HASH}
            TIMEOUT 15)
    file(STRINGS ${TENSORFLOW_ARCHIVE_HASH} TENSORFLOW_HASH)
    file(DOWNLOAD https://share.jtcat.com/libtensorflow_cc.tar.gz ${TENSORFLOW_ARCHIVE}
            TIMEOUT 60
            EXPECTED_HASH SHA256=${TENSORFLOW_HASH}
            TLS_VERIFY ON
            )

    if (NOT EXISTS "${PROJECT_SOURCE_DIR}/deps/libtensorflow_cc")
        file(MAKE_DIRECTORY "${PROJECT_SOURCE_DIR}/deps/libtensorflow_cc")
    endif()
    execute_process(COMMAND ${CMAKE_COMMAND} -E tar "xzvf" ${TENSORFLOW_ARCHIVE}
            WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/deps/libtensorflow_cc")
endif()