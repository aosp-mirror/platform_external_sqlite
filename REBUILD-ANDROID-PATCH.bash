#!/bin/bash
#
# Copyright (C) 2023 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# This script updates SQLite source files with a SQLite tarball.
#
# Usage: REBUILD-ANDROID_PATCH.bash
#
# This script must be executed in $ANDROID_BUILD_TOP/external/sqlite/
#

set -e

script_name="$(basename "$0")"

source_tgz="$1"
source_ext_dir="$1.extracted"

die() {
    echo "$script_name: $*"
    exit 1
}

echo_and_exec() {
    echo "  Running: $@"
    "$@"
}

# Make sure the source tgz file exists.
pwd="$(pwd)"
if [[ ! "$pwd" =~ .*/external/sqlite/? ]] ; then
    die 'Execute this script in $ANDROID_BUILD_TOP/external/sqlite/'
fi

# No parameters are permitted
if [[ ! $# -eq 0 ]]; then
    die "Unexpected arguments on the command line"
fi

echo
echo "# Regenerating Android.patch ..."
(
    cd dist
    echo_and_exec bash -c '(for x in orig/*; do diff -u -d $x ${x#orig/}; done) > Android.patch'
)
