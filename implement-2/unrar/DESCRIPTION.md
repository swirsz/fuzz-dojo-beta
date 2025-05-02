This module is identical to the projects module.  Take what you've learned from the 'Understanding Fuzz Drivers' module and implement your plan from the 'Planning New Fuzz Drivers' module to make a simple fuzz driver that achieves at least a 5% boost in total project coverage.
-

---

Use (Report link) to browse the source code and fuzz introspector report https://introspector.oss-fuzz.com/project-profile?project=unrar

Fuzz Introspector
https://storage.googleapis.com/oss-fuzz-introspector/unrar/inspector-report/20250411/fuzz_report.html

Unrar is a free library for extracting rar files.  A single fuzz driver reaches 58% of the code.  A very recent change has broken coverage reports for this project, but older reports are still available.  Browsing the full calltree shows that many commandline functions of this tool are not tested by the fuzz driver.

