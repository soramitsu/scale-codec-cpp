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
    URL  https://github.com/qdrvm/qtils/archive/54e4ef981165ef0962564f3fa878193eb466d644.tar.gz
    SHA1 d2e111301e319225ab45d4ced29cb366987caa8f
)
