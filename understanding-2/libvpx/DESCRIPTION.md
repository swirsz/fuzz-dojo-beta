Run `/challenge/run` to copy fuzz drivers into `/home/hacker/fuzz-dojo/{project}`

Edit each of these files and add comments to the code to demonstrate your understanding of their operation. A flag will be provided when sufficient comments are added to each driver, and your comments will be evaluated later.

---

Use (Report link) to browse the source code and fuzz introspector report https://introspector.oss-fuzz.com/project-profile?project=libvpx

Fuzz Introspector
https://storage.googleapis.com/oss-fuzz-introspector/libvpx/inspector-report/20250411/fuzz_report.html

Libvpx is a free software video codec library that serves as the reference software implementation for the VP8 and VP9 video coding formats, and for AV1.  It has 2 decoding fuzz drivers, one for VP8 and another for VP9 that reaches 60% of the code.  The source code includes hundreds of unit tests in /test and additional code under /examples that could be turned into new fuzz drivers.
