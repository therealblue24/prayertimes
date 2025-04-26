# libprayertimes `1.0.2`
Made symbols hidden

# libprayertimes `1.0.1`
Made a function to return version of library

# libprayertimes `1.0`
Made a library that can be used to calculate prayer times

# prayertimes `2.0.4`
Now shows version of library

# `2.0.3`
Added new makefile flags (`CODEREVIEW` and `SANITIZERS`):

- `make CODEREVIEW=yes`: Enable basically every warning in the book that we want
- `make SANITIZERS=yes`: Enable sanitizers

Also fixed some obscure bugs

# `2.0.2`
Bugfix for use-after-free
Merge PR `portability++`

# `2.0.1`
Possible bugfix for weird file names on Linux

# `2.0`
## The big one

List of changes:

- Fixed DST bug(s)
- Migration from old to new config system, will be removed in 3.0
- Fixed many bugs
- Cleaned up some stuff
- Readable, yml-based config
- Fixed some memory leaks & related memory crashes
- Many, many more internal stuff

I'm going to leave code cleanup to 2.1.

# `2.0-beta4`
Fixed DST bug

Shenatigans

# `2.0-beta3`
Comments in config where horrible mistake

Migration from old config system to New (TM) config system

> NOTE: The migration from the old to new config system will be removed in the eventual update 3.0

# `2.0-beta2`
Fixes by far the most stupid bug I have ever made

Remove some leftover debug stuff

Comments in config!

# `2.0-beta1`
Breaks backwards compatibility

Readable config support.

Heavily untested.

Heavily undocumented.

> Use with caution.

# `1.5`
Fixed Julian day calculation

Customizable (through command line):
- Maghrib angle
- Isha angle
- Isha minutes
- Elevation

Added calculation methods:
- MWL
- ISNA
- Egypt
- Makkah (pass command line flag `--ramadan` if it is Ramadan to use properly)
- Karachi
- Tehran
- Jafari

# `1.4`
Fixed Asr time calculation

Prayer time adjustment

# `1.2`
Versioning system introduced; refactoring

# pre-1.2
Look at the git history
