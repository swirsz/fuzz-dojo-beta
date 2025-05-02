Run `/challenge/run` to copy fuzz drivers into `/home/hacker/fuzz-dojo/{project}`

Edit each of these files and add comments to the code to demonstrate your understanding of their operation. A flag will be provided when sufficient comments are added to each driver, and your comments will be evaluated later.
-

---

Use (Report link) to browse the source code and fuzz introspector report https://introspector.oss-fuzz.com/project-profile?project=unrar

Fuzz Introspector
https://storage.googleapis.com/oss-fuzz-introspector/unrar/inspector-report/20250411/fuzz_report.html

Unrar is a free library for extracting rar files.  A single fuzz driver reaches 58% of the code.  A very recent change has broken coverage reports for this project, but older reports are still available.  Browsing the full calltree shows that many commandline functions of this tool are not tested by the fuzz driver.

