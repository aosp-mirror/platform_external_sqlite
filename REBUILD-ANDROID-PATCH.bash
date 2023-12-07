#!/bin/bash
#

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
