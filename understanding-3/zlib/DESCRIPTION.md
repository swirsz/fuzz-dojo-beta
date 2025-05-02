Run `/challenge/run` to copy fuzz drivers into `/home/hacker/fuzz-dojo/{project}`

Edit each of these files and add comments to the code to demonstrate your understanding of their operation. A flag will be provided when sufficient comments are added to each driver, and your comments will be evaluated later.

---

Use (Report link) to browse the source code and fuzz introspector report https://introspector.oss-fuzz.com/project-profile?project=zlib

Fuzz Introspector
https://storage.googleapis.com/oss-fuzz-introspector/zlib/inspector-report/20250411/fuzz_report.html

Zlib is a general purpose data compression library. It has 10 different fuzz drivers that reach 80% of the project's code, and 87% of the project's cyclomatic complexity.

The deflate slow and deflate fast functions have some untouched sections of code.
