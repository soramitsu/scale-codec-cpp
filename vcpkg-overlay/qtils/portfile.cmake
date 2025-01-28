vcpkg_check_linkage(ONLY_STATIC_LIBRARY)
vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO qdrvm/qtils
  REF e8dc3b521293f477f4e9a80602c1ad85b8700575
  SHA512 69c3444399df1f22a63f6ef9882f78a04359f431920cc1a18cb20ec9750f6b120f9682302f4c1b1caade3719b047899c5c0a4b533742d1fc8959c97edffd59dc
)
vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}")
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME "qtils")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
