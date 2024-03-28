#!/bin/bash
#
# Copyright (C) 2024 The Android Open Source Project
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

# Some common functions used by the source-update scripts.
#

die() {
    echo "$script_name: $*"
    exit 1
}

echo_and_exec() {
    echo "  Running: $@"
    "$@"
}

validate_year() {
  local year=$1
  if [[ "$year" =~ ^2[0-9][0-9][0-9]$ ]]; then
    return 0;
  else
    return 1;
  fi
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
  if [[ ${#fields[*]} -lt 2 || ${#fields[*]} -gt 4 ]]; then
    echo "cannot parse version: $version"
    return 1
  fi
  if [[ ${#fields[*]} -eq 2 ]]; then fields+=(0); fi
  if [[ ${#fields[*]} -eq 3 ]]; then fields+=(0); fi
  printf "%d%02d%02d%02d" ${fields[*]}
  return 0
}

function prettify_release {
  local version=$1
  local patch=$((version % 100))
  version=$((version / 100))
  local minor=$((version % 100))
  version=$((version / 100))
  local major=$((version % 100))
  version=$((version / 100))
  # version now contains the generation number.
  printf "%d.%d.%d" $version $major $minor
}
