vcpkg_check_linkage(ONLY_STATIC_LIBRARY)
vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO qdrvm/qtils
  REF 446d6e12e5ad44f48de427e5b79d3372c309179e
  SHA512 3f0b4436512f8984a9faffcedbe9b76ead63d7f3b662506b2fc71738a1bd4d3bf8b8a1b2fc5e5c5cd386f007c02da9c1628562db24c21217b90e9f5b640d1f0b
)
vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}")
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME "qtils")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
