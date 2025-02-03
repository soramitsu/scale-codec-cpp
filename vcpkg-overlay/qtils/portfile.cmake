vcpkg_check_linkage(ONLY_STATIC_LIBRARY)
vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO qdrvm/qtils
  REF 609dbecd1d937816ac50fa9b70a575da6f7dabf5
  SHA512 2b0e531012682088f5c93fbde36600db7b2a2c7b483826d66bd939132404cd34abfcde55b38688162824edbb6cc809e1c2d93a5b138feed7dca3678e8aa501fb
)
vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}")
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME "qtils")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
