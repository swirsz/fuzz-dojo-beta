Run `/challenge/run` to copy fuzz drivers into `/home/hacker/fuzz-dojo/{project}`

Edit each of these files and add comments to the code to demonstrate your understanding of their operation. A flag will be provided when sufficient comments are added to each driver, and your comments will be evaluated later.
-

---

Use (Report link) to browse the source code and fuzz introspector report https://introspector.oss-fuzz.com/project-profile?project=snappy

Fuzz Introspector
https://storage.googleapis.com/oss-fuzz-introspector/snappy/inspector-report/20250411/fuzz_report.html

Snappy is a compression tool optimized for high speeds and reasonable compression rates.  2 existing fuzz drivers for compression and decompression currently reach about 67% of the code.

Many IOVec fuctions (a collection of memory regions) are not covered.
