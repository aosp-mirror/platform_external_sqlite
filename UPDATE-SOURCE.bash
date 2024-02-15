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

# This script updates SQLite source files with a SQLite tarball.
#
# Usage: UPDATE-SOURCE.bash SQLITE-SOURCE.tgz
#
# This script must be executed in $ANDROID_BUILD_TOP/external/sqlite/
#

set -e

script_name="$(basename "$0")"

if [ $# -eq 0 ]; then
  echo "Usage: ${script_name} [src_tarball_url] [sqlite_version]"
  echo "Example:"
  echo "${script_name} https://sqlite.org/2023/sqlite-autoconf-3420000.tar.gz 3.42.0"
  exit 1
fi

die() {
    echo "$script_name: $*"
    exit 1
}

echo_and_exec() {
    echo "  Running: $@"
    "$@"
}

pwd="$(pwd)"
if [[ ! "$pwd" =~ .*/external/sqlite/? ]] ; then
    die 'Execute this script in $ANDROID_BUILD_TOP/external/sqlite/'
fi

src_tarball_url="$1"
sqlite_version="$2"

source_tgz=$(mktemp /tmp/sqlite-${sqlite_version}.zip.XXXXXX)
wget ${src_tarball_url} -O ${source_tgz}

echo
echo "# Extracting the source tgz..."
source_ext_dir="${source_tgz}.extracted"
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

dist_dir="dist-${sqlite_version}"
echo
echo "# Copying the source files ..."
echo_and_exec mkdir -p "${dist_dir}"
echo_and_exec mkdir -p "${dist_dir}/orig"
for to in ${dist_dir}/orig/ ${dist_dir}/ ; do
    echo_and_exec cp "$source_ext_dir/"{shell.c,sqlite3.c,sqlite3.h,sqlite3ext.h} "$to"
done

echo
echo "# Applying Android.patch ..."
(
    cd ${dist_dir}
    echo_and_exec patch -i ../Android.patch
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
    export SQLITE_VERSION=${sqlite_version}
    export YEAR=$(date +%Y)
    export MONTH=$(date +%M)
    export DAY=$(date +%D)
    envsubst < README.version.TEMPLATE > ${dist_dir}/README.version
    envsubst < METADATA.TEMPLATE > ${dist_dir}/METADATA
)

cat <<EOF

=======================================================

  Finished successfully!

  Make sure to update README.version

=======================================================

EOF

