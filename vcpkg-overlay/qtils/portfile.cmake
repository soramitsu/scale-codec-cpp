vcpkg_check_linkage(ONLY_STATIC_LIBRARY)
vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO qdrvm/qtils
  REF e8c5a1d5f3c808c0146698311443f8eb5a000835
  SHA512 4afe92b3657b60977f0a6c81b2eb002c74bf806bc7dcb0c5d287d2d43f1fcb688f2bcb17a433c971038c93eb3c896d086808e74132d710092a4c908a9c4a4ce2
)
vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}")
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME "qtils")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
