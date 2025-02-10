vcpkg_check_linkage(ONLY_STATIC_LIBRARY)
vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO qdrvm/qtils
  REF 16e7c819dd50af2f64e2d319b918d0d815332266
  SHA512 02c613ee2870b8b5956f7a0494a5a49b9ba499801541dc16d5fc915480111da56e6dd998ca7d43803197006b9a74ec8201ae9952de97810849a7e208e8f4b0dc
)
vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}")
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME "qtils")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
