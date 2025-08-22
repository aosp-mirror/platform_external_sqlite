#!/bin/bash
#
# Copyright (C) 2025 The Android Open Source Project
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

# This script checks the symlink dist/sqlite-default.  The symlink must point
# to a valid directory and should point to the the release used by next in the
# current workspace.  If executed without any arguments, the script just reports
# if the symlink is up to date.  If executed with the '-u' (update) switch, the
# script will update the symlink.

# This script must be executed in $ANDROID_BUILD_TOP/external/sqlite/.

set -e
 
script_name="$(basename "$0")"
script_dir=$(dirname $(realpath ${BASH_SOURCE[0]}))
 
source $script_dir/common-functions.sh

if [[ ! $(pwd) =~ .*/external/sqlite/? ]] ; then
  echo 'Execute this script in $ANDROID_BUILD_TOP/external/sqlite/'
  exit 1
fi

function help {
  local msg="$1" prog=$(basename $0)
  if [[ -n $msg ]]; then echo "$msg"; fi
  echo "usage: $prog [-u] [-r <release>]"
}
 
release=next
check=y
while getopts "ur:" option; do
  case $option in
    r) release=$OPTARG;;
    u) check=;;         # Update the link
    *) help; exit 1
  esac
done
shift $((OPTIND- 1))

if [[ $# -gt 0 ]]; then
  help "extra arguments on command line"
  exit 1
fi

export TARGET_RELEASE="$release"
sqlite_next=$(get_build_var RELEASE_PACKAGE_LIBSQLITE3)
if [[ -z $sqlite_next ]]; then
  die "unable to determine next release" >&2
fi

# Report if the link is set properly.
sqlite_current=$(readlink dist/sqlite-default)
sqlite_current=${sqlite_current#sqlite-autoconf-}
status=1
if [[ -z $sqlite_current ]]; then
  echo "default is not set"
elif [[ ! "$sqlite_current" = "$sqlite_next" ]]; then
  echo "current default $sqlite_current does not match nextfood $sqlite_next"
else
  echo "current value $sqlite_current is up to date"
  status=0
fi

# If only checking was requested, exit now.
if [[ -n $check || $status -eq 0 ]]; then exit $status; fi

# Update the link, if possible.

sqlite_dist="sqlite-autoconf-$sqlite_next"
if [[ ! -d dist/$sqlite_dist ]]; then
  die "no dist directory for $sqlite_next"
fi

echo -n "Publishing $sqlite_next..."

( 
  cd dist
  ln -sfn $sqlite_dist sqlite-default
)

cp dist/$sqlite_dist/METADATA .
# Do not copy README.version.

echo "done"
