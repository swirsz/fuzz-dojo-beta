This module is identical to the projects module.  Take what you've learned from the 'Understanding Fuzz Drivers' module and implement your plan from the 'Planning New Fuzz Drivers' module to make a simple fuzz driver that achieves at least a 5% boost in total project coverage.
-

---

Use (Report link) to browse the source code and fuzz introspector report https://introspector.oss-fuzz.com/project-profile?project=zlib

Fuzz Introspector
https://storage.googleapis.com/oss-fuzz-introspector/zlib/inspector-report/20250411/fuzz_report.html

Zlib is a general purpose data compression library. It has 10 different fuzz drivers that reach 80% of the project's code, and 87% of the project's cyclomatic complexity.

The deflate slow and deflate fast functions have some untouched sections of code.
