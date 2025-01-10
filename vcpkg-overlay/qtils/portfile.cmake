vcpkg_check_linkage(ONLY_STATIC_LIBRARY)
vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO qdrvm/qtils
  REF 58dd0cbd7a1ae1cad3b30cb5390080931d5b3368
  SHA512 47dbac166beaeaa91dc74687540a87f85b8645cb0dfb9940abbfc4c5fdfe9fd7888ee8948bd7d0afe6ea8b3842ddc50a2e344704e12173dddc1c587ce4cd7f04
)
vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}")
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME "qtils")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
