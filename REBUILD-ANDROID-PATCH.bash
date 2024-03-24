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
# Usage: REBUILD-ANDROID_PATCH.bash <release>
#
# This script must be executed in $ANDROID_BUILD_TOP/external/sqlite/
#

set -e

script_name="$(basename "$0")"
script_dir=$(dirname $(realpath ${BASH_SOURCE[0]}))

die() {
    echo "$script_name: $*"
    exit 1
}

echo_and_exec() {
    echo "  Running: $@"
    "$@"
}

# This function converts a release string like "3.42.0" to the canonical 7-digit
# format used by sqlite.org for downloads: "3420000".  A hypothetical release
# number of 3.45.6 is converted to "3450600".  A hypothetical release number of
# 3.45.17 is converted to "3451700".  The last two digits are assumed to be
# "00" for now, as there are no known counter-examples.
function normalize_release {
  local version=$1
  local -a fields
  fields=($(echo "$version" | sed 's/\./ /g'))
  if [[ ${#fields[*]} -lt 2 || ${#fields[*]} -gt 3 ]]; then
    echo "cannot parse version: $version"
    return 1
  elif [[ ${#fields[*]} -eq 2 ]]; then
    fields+=(0)
  fi
  printf "%d%02d%02d00" ${fields[*]}
  return 0
}

if [[ $# -lt 1 ]]; then
  die "missing required arguments"
elif [[ $# -gt 1 ]]; then
  die "extra arguments on command line"
fi
sqlite_release=$(normalize_release "$1") || die "invalid release"
sqlite_base="sqlite-autoconf-${sqlite_release}"

export patch_dir=${script_dir}/dist
echo
echo "# Regenerating Android.patch ..."
(
    cd dist/$sqlite_base || die "release directory not found"
    echo_and_exec bash -c '(for x in orig/*; do diff -u -d $x ${x#orig/}; done) > Android.patch'
    echo_and_exec cp Android.patch ${patch_dir}/
) 
