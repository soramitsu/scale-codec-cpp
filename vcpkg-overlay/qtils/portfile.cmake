vcpkg_check_linkage(ONLY_STATIC_LIBRARY)
vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO qdrvm/qtils
  REF df89ca404e0e251c89490030a6be57901d9e136a
  SHA512 5621274e69780a869890edc97692cb974057c90cb8ef9ed74db5a7c1d97add352c92ad91bb527497243e44bb29d99b656c23312404e1e7572116a3a3d811c29b
)
vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}")
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME "qtils")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
