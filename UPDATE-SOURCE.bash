#!/bin/bash
#
# Copyright (C) 2018 The Android Open Source Project
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

# This script updates SQLite source files with a SQLite tarball.  The tarball is
# downloaded from the sqlite website.
#
# Usage: UPDATE-SOURCE.bash [-nF] <year> <sqlite-release>
#
# This script must be executed in $ANDROID_BUILD_TOP/external/sqlite/.  However,
# for testing it can run anywhere: use the -F switch.
#

set -e

script_name="$(basename "$0")"
script_dir=$(dirname $(realpath ${BASH_SOURCE[0]}))

source $script_dir/common-functions.sh

usage() {
  if [[ $# -gt 0 ]]; then echo "$*" >&2; fi
  echo "Usage: ${script_name} [-nF] [-u <url>] [-y <year>] <version>"
  echo "  version the sqlite version as <major>.<minor>[.<patch>]"
  echo "          the patch level defaults to 0"
  echo "  -n      dry-run: evaluate arguments but d not change anything"
  echo "  -u url  download the tarball from the specified url"
  echo "  -p file a tarball that has already been downloaded"
  echo "  -y year the 4-digit year the sqlite version was released - required"
  echo "          if a full url is not specified and the year is not this year"
  echo "  -F      force execution even if not in external/sqlite"
  echo 
  echo "Example:"
  echo "${script_name} 2023 3.42"
}

dry_run=
force=
src_tarball_url=
src_file=
year=$(date +%Y)
while getopts "hnFu:p:y:" option; do
  case $option in
    h) usage; exit 0;;
    n) dry_run=y;;
    u) src_tarball_url=$OPTARG;;
    p) src_file=$OPTARG;;
    y) year=$OPTARG;;
    F) force=y;;
    *) usage "unknown switch"; exit 1;;
  esac
done
shift $((OPTIND- 1))

if [[ $# -lt 1 ]]; then
  usage; die "missing required arguments"
elif [[ $# -gt 1 ]]; then
  die "extra arguments on command line"
fi
sqlite_release=$(normalize_release "$1") || die "invalid release"

# Find the source directory that is no later than the current target.  It may be
# the current target.  The purpose is to identify the starting patch file.
patch_source=$(previous_release $(pwd)/dist ${sqlite_release})
echo "# Previous release is $patch_source"

sqlite_base="sqlite-autoconf-${sqlite_release}"
sqlite_file="${sqlite_base}.tar.gz"
if [[ -z $src_tarball_url ]]; then
  validate_year "$year" || die "invalid year"
  src_tarball_url="https://www.sqlite.org/$year/${sqlite_file}"
fi

if [[ -n $dry_run ]]; then
  echo "fetching $src_tarball_url"
  echo "installing in dist/$sqlite_base"
  exit 0
fi

pwd="$(pwd)"
if [[ -z $force && ! "$pwd" =~ .*/external/sqlite/? ]] ; then
    die 'Execute this script in $ANDROID_BUILD_TOP/external/sqlite/'
fi

source_tgz=$(mktemp /tmp/sqlite-${sqlite_release}.zip.XXXXXX)
source_ext_dir="${source_tgz}.extracted"
trap "rm -rf ${source_tgz} ${source_ext_dir}" EXIT
if [[ -n ${src_file} ]]; then
  cp ${src_file} ${source_tgz}
else
  wget ${src_tarball_url} -O ${source_tgz}
fi

echo
echo "# Extracting the source tgz..."
echo_and_exec rm -fr "$source_ext_dir"
echo_and_exec mkdir -p "$source_ext_dir"
echo_and_exec tar xvf "$source_tgz" -C "$source_ext_dir" --strip-components=1

echo
echo "# Making file sqlite3.c in $source_ext_dir ..."
(
    cd "$source_ext_dir"
    echo_and_exec ./configure
    echo_and_exec make -j 4 sqlite3.c
)

echo "# Saving patches from $patch_source"
echo_and_exec cp $patch_source/Android.patch $source_ext_dir/Android.patch

export dist_dir="dist/${sqlite_base}"
echo
echo "# Copying the source files ..."
echo_and_exec rm -rf ${dist_dir}
echo_and_exec mkdir -p "${dist_dir}"
echo_and_exec mkdir -p "${dist_dir}/orig"
for to in ${dist_dir}/orig/ ${dist_dir}/ ; do
    echo_and_exec cp "$source_ext_dir/"{shell.c,sqlite3.c,sqlite3.h,sqlite3ext.h} "$to"
done

echo
echo "# Applying Android.patch from $(basename $patch_source)"
(
    cd ${dist_dir}
    echo "PATCHING IN $dist_dir" >&2
    echo_and_exec patch -i ${source_ext_dir}/Android.patch
)

echo
echo "# Regenerating Android.patch ..."
(
    cd ${dist_dir}
    echo_and_exec bash -c '(for x in orig/*; do diff -u -d $x ${x#orig/}; done) > Android.patch'
)

echo
echo "# Generating metadata ..."
(
    export SQLITE_URL=${src_tarball_url}
    export SQLITE_VERSION=$(prettify_release ${sqlite_release})
    export YEAR=$(date +%Y)
    export MONTH=$(date +%-m)
    export DAY=$(date +%-d)
    envsubst < README.version.TEMPLATE > ${dist_dir}/README.version
    envsubst < METADATA.TEMPLATE > ${dist_dir}/METADATA
)

cat <<EOF

=======================================================

  Finished successfully!

  Make sure to update README.version

=======================================================

EOF

