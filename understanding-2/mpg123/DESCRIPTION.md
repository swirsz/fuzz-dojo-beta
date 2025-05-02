Run `/challenge/run` to copy fuzz drivers into `/home/hacker/fuzz-dojo/{project}`

Edit each of these files and add comments to the code to demonstrate your understanding of their operation. A flag will be provided when sufficient comments are added to each driver, and your comments will be evaluated later.

---

Use (Report link) to browse the source code and fuzz introspector report https://introspector.oss-fuzz.com/project-profile?project=mpg123

Fuzz Introspector
https://storage.googleapis.com/oss-fuzz-introspector/mpg123/inspector-report/20250411/fuzz_report.html

Mpg123 is a Linux MPEG 1.0/2.0/2.5 audio player.  Two fuzz drivers reaching 52% of the code.  Most of the decoding library libmpg123.c is not fuzzed, indicating that it might benefit from a larger corpus.
