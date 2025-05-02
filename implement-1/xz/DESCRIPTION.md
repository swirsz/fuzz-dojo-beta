This module is identical to the projects module.  Take what you've learned from the 'Understanding Fuzz Drivers' module and implement your plan from the 'Planning New Fuzz Drivers' module to make a simple fuzz driver that achieves at least a 5% boost in total project coverage.
-

---

Use (Report link) to browse the source code and fuzz introspector report https://introspector.oss-fuzz.com/project-profile?project=xz

Fuzz Introspector
https://storage.googleapis.com/oss-fuzz-introspector/xz/inspector-report/20250411/fuzz_report.html

XZ is a file compression library aiming to maximize compression ratios. It supports the .xz and .lzma  formats. 4 different fuzz drivers reach 75% of the project code.

Many of the legacy lzma functions are not fuzzed, and streaming functions similarly have unreached sections.
