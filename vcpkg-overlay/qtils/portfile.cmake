vcpkg_check_linkage(ONLY_STATIC_LIBRARY)
vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO qdrvm/qtils
  REF 198a40443b838a421db3ad8fc8dbf72e8ec4a665
  SHA512 fb0af672ceddc58a65133a132e4d3b71efe419ac647b770204dd6da338c10fe364f13c2c7d6ae3fa3866194bea78a51cd5fc085ddacbc4e49be95b22054880e1
)
vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}")
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME "qtils")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
