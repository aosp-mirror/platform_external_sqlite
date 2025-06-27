## Upgrading SQLite

This document lists the steps needed to upgrade the sources in this repository
with the latest revision from upstream.

## Source Directory

The upgrade takes place on `androidx-main`. The primary directory is below.

```text
external/sqlite
```

The upgrade steps are:

*   Select a version for the upgrade.
*   Find the amalgamation zip file. For release 3.46.0, the URL is
    [sqlite-amalgamation-3460000.zip](https://www.sqlite.org/2024/sqlite-amalgamation-3460000.zip).
*   Extract the content of the direcoty into `external/sqlite/src`,
    it should contain the following files:
    * `shell.c`
    * `sqlite3.c`
    * `sqlite3.h`
    * `sqlite3ext.h`
* Update the version and year in `sqlite-bundled`'s build file:
```text
frameworks/support/sqlite/sqlite-bundled/build.gradle
```
```
    it.sqliteVersion.set("3.46.0")
    it.sqliteReleaseYear.set(2024)
```
* Update `METADATA` and `README.version` located in `external/sqlite`.


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

## Version Test

There is a test that verifies the SQLite version. This must be updated as
well and it must be updated within `androidx-main` at the same time as the
source.

```text
frameworks/support/sqlite/integration-tests/driver-conformance-test/src/commonTest/kotlin/androidx/sqlite/driver/test/BaseBundledConformanceTest.kt
```

Update the following constant:

```kotlin
    const val EXPECTED_SQLITE_VERSION = "3.46.0"


## Recommended Tests

It is recommended to execute the following test script before committing an
update:

```test
frameworks/support/sqlite/scripts/runConformanceTest.sh
```