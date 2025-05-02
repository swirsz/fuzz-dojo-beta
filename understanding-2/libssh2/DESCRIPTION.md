Run `/challenge/run` to copy fuzz drivers into `/home/hacker/fuzz-dojo/{project}`

Edit each of these files and add comments to the code to demonstrate your understanding of their operation. A flag will be provided when sufficient comments are added to each driver, and your comments will be evaluated later.
-

---

Use (Report link) to browse the source code and fuzz introspector report https://introspector.oss-fuzz.com/project-profile?project=libssh2

Fuzz Introspector
https://storage.googleapis.com/oss-fuzz-introspector/libssh2/inspector-report/20250411/fuzz_report.html

Libssh2 is a library implementing the SSH2 protocol.  It has only 1 fuzz driver that acts as a client and reaches only 26% of the code.  This library has many similarities to openssh and may be able to implement similar fuzz drivers.  A tests directory also provides many unit tests that possibly could be implemented as fuzz drivers.
