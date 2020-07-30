include(vcpkg_common_functions)

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO "bitcoin-core/secp256k1"
    REF "214cb3c321c8f240a456ba2861f84b72141e0584"
    SHA512 "1b5fcc08658356bfe6353e7288559df8426b3d20a905129e4150c5120426c115ca233088eb3b2bfd79104e27496461f8bed64732c6d18449f00b360d00e1f2db"
)

file(COPY ${CURRENT_PORT_DIR}/libsecp256k1-config.h DESTINATION ${SOURCE_PATH})
file(COPY ${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt DESTINATION ${SOURCE_PATH})

vcpkg_configure_cmake(
    SOURCE_PATH ${SOURCE_PATH}
    PREFER_NINJA
    OPTIONS_DEBUG
        -DINSTALL_HEADERS=OFF
)

vcpkg_install_cmake()
vcpkg_fixup_cmake_targets(CONFIG_PATH share/unofficial-${PORT} TARGET_PATH share/unofficial-${PORT})

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share")
configure_file(${CMAKE_CURRENT_LIST_DIR}/secp256k1-config.cmake ${CURRENT_PACKAGES_DIR}/share/unofficial-${PORT}/unofficial-secp256k1-config.cmake @ONLY)
file(INSTALL ${SOURCE_PATH}/COPYING DESTINATION ${CURRENT_PACKAGES_DIR}/share/${PORT} RENAME copyright)
