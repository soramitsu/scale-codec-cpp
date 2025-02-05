# Template for a custom hunter configuration Useful when there is a need for a
# non-default version or arguments of a dependency, or when a project not
# registered in soramitsu-hunter should be added.
#
# hunter_config(
#     package-name
#     VERSION 0.0.0-package-version
#     CMAKE_ARGS
#      CMAKE_VARIABLE=value
# )
#
# hunter_config(
#     package-name
#     URL https://github.com/organization/repository/archive/hash.tar.gz
#     SHA1 1234567890abcdef1234567890abcdef12345678
#     CMAKE_ARGS
#       CMAKE_VARIABLE=value
# )

hunter_config(
    qtils
    URL  https://github.com/qdrvm/qtils/archive/5ef58d9d9f8bf3d53c15f12b12f9d5714128d8b9.tar.gz
    SHA1 f4e62483408ef9b901c6d05ae879d2714170ecb4
)
