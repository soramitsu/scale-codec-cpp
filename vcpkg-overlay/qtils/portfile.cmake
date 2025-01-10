vcpkg_check_linkage(ONLY_STATIC_LIBRARY)
vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO qdrvm/qtils
  REF 80d80fa842fae36ed5926254cee032d5b0f37c7d
  SHA512 136c9192cd8d503bc32a7bd541f82f5194de94c7684d09407ca6a506419ae160e42a479fe4ceb64d14cd2a4c8fc85eb3390ce089a4054bbad55e2c9fd0820c8e
)
vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}")
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME "qtils")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
