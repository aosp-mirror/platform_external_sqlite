## Upgrading SQLite

This document lists the steps needed to upgrade the sources in this repository
with a new revision from upstream.  There are three scenarios for an
upgrade, with slightly different details:

* SQLite is upgraded in the next Android release.  The upgrade takes place on
  `main`.  This is called a flagged upgrade.
  
* SQLite is upgraded in an earlier Android release, starting with 24Q3.  These
  releases use trunk-stable flags.  This is called a flagged backport.

* SQLite is upgraded in an earlier Android release that does not use trunk-stable flags.
  This is called a legacy upgrade.

A word on SQLite versioning.  SQLite releases are identified by a public version
`M.mm.p`, where
* `M` is the major version ("3" since 2004, and unlikely to ever change).
* `mm` is the minor version.
* `p` is the patch level.  In practice, the patch level is a single digit but
  the naming scheme allows for two digits.
  
There is also a version *key*: `Mmmppxx`.  This uses the same fields
as the public version string except that the patch level is always two digits
(zero padded) and `xx` is a two-digit local patch revision.  (`xx` is almost
always "00".)

## Source Directory

The source directory for the sqlite library is `external/sqlite/`.

The source code for a legacy upgrade is placed in the single directory
`external/sqlite/dist`.  Upgrading to a new sqlite version overwrites the
previous sqlite source code.

The source code for a flagged upgrade or backport is stored in a directory that
is named after the version.  Multiple sqlite source code directories coexist.  A
trunk-stable build flag determines which version is active.  The source code
directories are named
`external/sqlite/dist/sqlite-autoconf-<key>/`.

## Upgrading

Begin the upgrade by changing to this directory: `external/sqlite`.  Then run
`UPDATE-SOURCE.bash`.  The details depend on the Android release and whether or
not the SQLite release has been published.

### Android before 24Q3

Download the SQLite autoconf amalgamation tarball for the desired version.  This
seems to be the "tarball" link in the SQLite version history.  However, if the
SQLite release has been published, the tarball can also be found directly at 
`https://www.sqlite.org/<year>/sqlite-autoconf-<key>.tar.gz`.

1. Run `UPDATE-SOURCE.bash <path-to-tarball>`.
2. Manually correct compilation errors and rebuild the patch file.  See the
   commands inside `UPDATE-SOURCE.bash`.
3. Run `UPDATE-SOURCE.bash <path-to-tarball>`.  This uses the corrected patch
   file and must complete without warnings or errors.

The process overwrites the original files.

### Android 24Q3 or later

In Android 24Q3 and later, `UPDATE-SOURCE.bash` requires the SQLite version (not
the version key) as its argument.  Other options are controlled by
switches. 

1. Update the source

* If the SQLite release was published in the current year, the script will
  construct the well-known URL for you, using the current year.

  `UPDATE-SOURCE.bash <version>`

* If the SQLite release was published in a prior year, the script will
  construct the well-known URL for you:

  `UPDATE-SOURCE.bash -y <year> <version>`

* If the SQLite release has not been formally published, you need the direct URL:

  `UPDATE-SOURCE.bash -u <url> <version>`

2. Run `REBUILD-ANDROID-PATCH.bash <version>` to rebuild the patch file.
3. Re-run `UPDATE-SOURCE.bash` with the same parameters as before.  This must
   complete without warnings or errors.

Once the scripts have completed, there will be a directory containing the new
source files.  The directory is named after the sqlite release (using the
compacted version) and exists in parallel with other sqlite release directories:
`external/sqlite/dist/sqlite-autoconf-<key>`.

## Flagging

As of Android 24Q3, the release of sqlite is controlled by a trunk-stable
build flag.  The flag is `RELEASE_PACKAGE_LIBSQLITE3`.  The value of that flag
is the sqlite key (e.g., "3420000").  Any target that
respects trunk-stable flags will use the source in
`external/sqlite/dist/sqlite-autoconf-<flag-value>`.

Not all targets respect the trunk-stable flags.  (The SDK, documentation, and
kernel builds are examples). Such targets use the directory
`external/sqlite/dist/sqlite-default`, which is a symlink to a real source
directory.  Maintenance of this symlink is described below.

If upgrading on main, then use gantry to advance the flag
through the necessary stages.

If upgrading an older Android branch, then the new SQLite release must move to `next`
immediately.  The procedure is not well documented, but the current process is:
1. Identify the internal release code.  Examples are "ap3a" and "bp2a".  
2. Find the flag directory, which should be
   `build/release/flag_values/<code>`.
3. In the release directory, create or edit
   `RELEASE_PACKAGE_LIBSQLITE3.textproto` to reflect the new release.  Look
   around for examples.

Once a release has reached the `next` stage, change the symbolic link
`sqlite-default` to point to the new release.  This ensures that any target that
does not honor build flags will use the newly promoted release.  On newer
Android releases, the script `PUBLISH-VERSION.bash` will do this for you.  On
older releases it must be done manually.

Note: there is the possibility that the flag will have to be rolled back.  If that
happens, be sure to update the symlink.

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
