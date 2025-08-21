###
### Welcome to a Fuzz Dojo Challenge
###

LIBSSH2 - medium 12632 loc 9%-20%-26% - 2 min

Use (Report link) to browse the source code and fuzz introspector report https://introspector.oss-fuzz.com/project-profile?project=libssh2

Fuzz Introspector
https://storage.googleapis.com/oss-fuzz-introspector/libssh2/inspector-report/20250411/fuzz_report.html

Libssh2 is a library implementing the SSH2 protocol.  It has only 1 fuzz driver that acts as a client and reaches only 26% of the code.  This library has many similarities to openssh and may be able to implement similar fuzz drivers.  A tests directory also provides many unit tests that possibly could be implemented as fuzz drivers.
