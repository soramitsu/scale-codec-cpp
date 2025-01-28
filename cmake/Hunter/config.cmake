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
    URL  https://github.com/qdrvm/qtils/archive/198a40443b838a421db3ad8fc8dbf72e8ec4a665.tar.gz
    SHA1 da91eaeec936ee572a1310024dfcd7b91ee5e5bf
)
