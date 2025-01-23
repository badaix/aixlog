# AixLog changelog

## Version 1.5.1

### Features

- Write Log tag to syslog as `[TAG]`

### Bugfixes

- Update readme to remove confusing outdated "AixLog::Type" from the docs (PR #16)
- Fix some clang tidy static analysis issues

_Johannes Pohl <snapcast@badaix.de>  Sun, 26 Jan 2025 00:13:37 +0200_

## Version 1.5.0

### Features

- Conditional is virtual, so that is_true can be overloaded
- Conditional can be constructed with a lambda returning true or false
- Support Windows unicode and non-unicode

### Bugfixes

- Fix building on Windows (Issue #11, #13)

_Johannes Pohl <snapcast@badaix.de>  Wed, 24 Feb 2021 00:13:37 +0200_

## Version 1.4.0

### Features

- Add filters for log sinks: filter by tag and/or severity
- Remove macro SLOG and log type SPECIAL in favor to the new filters

_Johannes Pohl <snapcast@badaix.de>  Wed, 17 Jun 2020 00:13:37 +0200_

## Version 1.3.0

### Bugfixes

- Log lines will not overlapped when logging from different threads

_Johannes Pohl <snapcast@badaix.de>  Thu, 21 Apr 2020 00:13:37 +0200_

## Version 1.2.4

### Bugfixes

- Use threadsafe version of localtime

_Johannes Pohl <snapcast@badaix.de>  Thu, 02 Jan 2020 00:13:37 +0200_

## Version 1.2.3

### Bugfixes

- Fix pedantic warnings

_Johannes Pohl <snapcast@badaix.de>  Thu, 02 Jan 2020 00:13:37 +0200_

## Version 1.2.2

### Bugfixes

- Fix pedantic warning ISO C++11 requires at least one argument for the "..." in a variadic macro

_Johannes Pohl <snapcast@badaix.de>  Mon, 07 Oct 2018 00:13:37 +0200_

## Version 1.2.1

### Features

### Bugfixes

- Remove unnecessary virtual function specifier from explicit operator bool() const to workaround a bug in MS Visual Studio

### General

_Johannes Pohl <snapcast@badaix.de>  Sat, 28 Apr 2018 00:13:37 +0200_

## Version 1.2.0

### Bugfixes

- fix __func__ macro on Android

### General

- non-native system log sinks are not defined
- make building of aixlog_example optional in cmake

_Johannes Pohl <snapcast@badaix.de>  Sat, 07 Apr 2018 00:13:37 +0200_

## Version 1.1.0

### Bugfixes

- fix warnings "Function defined but not used"
- fix include for MacOS

_Johannes Pohl <snapcast@badaix.de>  Fri, 09 Feb 2018 00:13:37 +0200_

## Version 1.0.4

### Bugfixes

- Flush buffer on exit (Issue #4)

_Johannes Pohl <snapcast@badaix.de>  Fri, 02 Feb 2018 00:13:37 +0200_

## Version 1.0.3

### Bugfixes

- Fix multi threading issue

_Johannes Pohl <snapcast@badaix.de>  Tue, 30 Jan 2018 00:13:37 +0200_

## Version 1.0.2

### Bugfixes

-Fix conflict with DEBUG macro on Windows (PR #3)
- Fix compile error "Virtual functions but no virtual destructors”

_Johannes Pohl <snapcast@badaix.de>  Thu, 11 Jan 2018 00:13:37 +0200_

## Version 1.0.1

### Bugfixes

- Fix build on MacOS < 10.12 (PR #1)
- Fix build on Windows (PR #2)
- "Format" string with trailing space was ignored

### General

- Improved performance

_Johannes Pohl <snapcast@badaix.de>  Tue, 09 Jan 2018 00:13:37 +0200_

## Version 1.0.0

### General

- Initial release

_Johannes Pohl <snapcast@badaix.de>  Sun, 12 Nov 2017 00:13:37 +0200_

