Run `/challenge/run` to copy fuzz drivers into `/home/hacker/fuzz-dojo/{project}`

Edit each of these files and add comments to the code to demonstrate your understanding of their operation. A flag will be provided when sufficient comments are added to each driver, and your comments will be evaluated later.

---

Use (Report link) to browse the source code and fuzz introspector report https://introspector.oss-fuzz.com/project-profile?project=xz

Fuzz Introspector
https://storage.googleapis.com/oss-fuzz-introspector/xz/inspector-report/20250411/fuzz_report.html

XZ is a file compression library aiming to maximize compression ratios. It supports the .xz and .lzma  formats. 4 different fuzz drivers reach 75% of the project code.

Many of the legacy lzma functions are not fuzzed, and streaming functions similarly have unreached sections.
