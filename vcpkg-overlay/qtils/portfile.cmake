vcpkg_check_linkage(ONLY_STATIC_LIBRARY)
vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO qdrvm/qtils
  REF 21ad4b22ce0d064b952df98bb938295b474ee0d0
  SHA512 6082061372048d140b2004cf605ff7e7089776d282207f83d57ae8c5ca1c78a5ef4aa568e4871a0ca121b191968584b9232485d484983ee64b3412ce27b1f2af
)
vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}")
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME "qtils")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
