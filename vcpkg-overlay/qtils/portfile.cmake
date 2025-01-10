vcpkg_check_linkage(ONLY_STATIC_LIBRARY)
vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO qdrvm/qtils
  REF de62454d12e250574577232ee7ede4406c4c22cb
  SHA512 7b91ea268fb2c9204c1b584d2b433bd93d8d1946ba5d05ddf119b955fdfdb76abbe47c0009a4b186f7968320b99044abfd58315a128c98cf65f57c870ba51112
)
vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}")
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME "qtils")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
