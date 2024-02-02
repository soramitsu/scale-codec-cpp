#!/usr/bin/env python3
#
# Copyright Quadrivium LLC
# All Rights Reserved
# SPDX-License-Identifier: Apache-2.0
#

import sys

N = int(sys.argv[1])

print("    %s" % "\n    ".join("else _SCALE_TIE(%s)" % ", ".join("v%d" % j for j in range(i+1)) for i in range(N)))
