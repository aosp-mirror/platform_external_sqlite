## Upgrading SQLite

This document lists the steps needed to upgrade the sources in this repository
with the latest revision from upstream.

## Source Directory

The upgrade takes place on `aosp-main`. The primary directory is below.

```text
external/sqlite
```

The upgrade steps are:

*   Select a version for the upgrade.  Note the year it was released by sqlite.org.
*   Find the autoconf amalgamation tarball. For release 3.42.0, the URL is
    [sqlite-autoconf-3420000.tar.gz](https://sqlite.org/2023/sqlite-autoconf-3420000.tar.gz).
*   Change to the directory `external/sqlite` in the workspace.
*   Run the script `UPDATE-SOURCE.bash`. This script is executable. The
    arguments are the sqlite release year and the version. Invoke the script without
    arguments for an example.

`UPDATE-SOURCE.bash` may fail if the Android patch cannot be applied cleanly. If
this happens, correct the patch failures by hand and rebuild the Android patch
file. Use the script `REBUILD-ANDROID-PATCH.bash` to rebuild the patch file.
This script takes a single argument which is the same version number that was
given to `UPDATE-SOURCE.bash`.  Then rerun `UPDATE-SOURCE.bash`. It is important
that `UPDATE-SOURCE.bash` run without errors.

Once the scripts have completed, there will be a directory containing the new
source files.  The directory is named after the sqlite release and exists in
parallel with other sqlite release directories.  For release 3.42.0, the
directory name is `external/sqlite/dist/sqlite-autoconf-3420000`.

## Flagging

The release of sqlite can be controlled by trunk-stable build flags.  The flag
is `RELEASE_PACKAGE_LIBSQLITE3`.  The value of that flag is the 7-digit sqlite
release number (e.g., 3420000).  Any target that respects trunk-stable flags
will use the source in `external/sqlite/dist/sqlite-autoconf-FLAG`.  Not all
targets respect the trunk-stable flags, however.  Such targets use the directory
`external/sqlite/dist/sqlite-default`.

A new release of sqlite can be promoted to `trunk` by setting the flag to the
proper release string.  Once a new release of sqlite has been promoted to
`next`, it is best practice to change the symbolic link `sqlite-default` to
point to the new release.  This ensures that any target that does not honor
build flags will use the newly promoted release.

Finally, after the new sqlite release has been delivered in an Android update,
old sqlite release directories can be deleted.

## LICENSE

This file contains the license that allows Android to use the third-party
software. SQLite is unusual because it has no license: it is in the public
domain. The current file content is below.

```text
The author disclaims copyright to this source code.  In place of
a legal notice, here is a blessing:

   May you do good and not evil.
   May you find forgiveness for yourself and forgive others.
   May you share freely, never taking more than you give.
```

## CTS Version Test

There is a CTS test that verifies the SQLite version. This must be updated as
well and it must be updated within `aosp-main` at the same time as the source.

```text
cts/tests/tests/database/src/android/database/sqlite/cts/SQLiteDatabaseTest.java
```

Find and update the function `testSqliteLibraryVersion()`.  This function must
be updated as soon as a new sqlite release is installed in `trunk_staging`, and
the function must accept old and new sqlite releases. Once a new release has
been committed to `next`, old releases can be rejected by this function.

Note that the CTS test is sometimes propagated to branches for older Android
releases, which do not use the latest sqlite release.  If that happens, the CTS
test will have to accept both the old and new sqlite releases.

## package.html

```text
frameworks/base/core/java/android/database/sqlite/package.html
```

This is a documentation file that, among other things, maps SQLite versions to
Android API levels. It should be updated every time SQLite is upgraded and on
every Android API bump. There is a small problem with API names: Android usually
prefers not to publicize the new API level until it is official, so care must be
taken not to expose unofficial API levels through this file.

The current plan for modifying this file when SQLite is upgraded is to add a new
row, using "latest" as the API level. The file should also be updated
appropriately when a new API level is formally announced.

This file probably should be modified in an Android branch (not AOSP). That way
it enters public view with the official release process.

## Recommended Tests

The following tests are recommended before committing an update:

```text
CtsScopedStorageDeviceOnlyTest
CtsScopedStorageBypassDatabaseOperationsTest
CtsScopedStorageGeneralTest
CtsScopedStorageRedactUriTest
CtsDatabaseTestCases
FrameworksCoreTests:android.database
```

At the time of writing, there are no test failures in the baseline code.
