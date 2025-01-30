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
    URL  https://github.com/qdrvm/qtils/archive/194f439f718d94d391a3ec20b5c51a825d64685b.tar.gz
    SHA1 28a5143f168d24a61bb639e7fbce856cad21087c
)
