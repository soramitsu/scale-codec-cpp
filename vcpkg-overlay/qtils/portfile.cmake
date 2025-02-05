vcpkg_check_linkage(ONLY_STATIC_LIBRARY)
vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO qdrvm/qtils
  REF 5ef58d9d9f8bf3d53c15f12b12f9d5714128d8b9
  SHA512 265506698dbbb6f52394b86b54aa9145512cbede2270f39085bd1cbf290701e02f01333851a7885b19f19b712b561a2bdc7fd5e2c59f6c45ea6494694085c4c0
)
vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}")
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME "qtils")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
