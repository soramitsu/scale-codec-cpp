vcpkg_check_linkage(ONLY_STATIC_LIBRARY)
vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO qdrvm/qtils
  REF 54e4ef981165ef0962564f3fa878193eb466d644
  SHA512 bd0d0dece343abfa0b40e0972bbcb2799d65be6a3339ed14a8cf17a50f32b66a71048b1f0c340928e8cc7bac739479c030798381951e9e6869144e2aa162a081
)
vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}")
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME "qtils")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
