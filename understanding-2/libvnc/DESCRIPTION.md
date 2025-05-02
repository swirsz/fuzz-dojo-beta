Run `/challenge/run` to copy fuzz drivers into `/home/hacker/fuzz-dojo/{project}`

Edit each of these files and add comments to the code to demonstrate your understanding of their operation. A flag will be provided when sufficient comments are added to each driver, and your comments will be evaluated later.

---

Use (Report link) to browse the source code and fuzz introspector report https://introspector.oss-fuzz.com/project-profile?project=libvnc

Fuzz Introspector
https://storage.googleapis.com/oss-fuzz-introspector/libvnc/inspector-report/20250411/fuzz_report.html

LibVNCServer/LibVNCClient are cross-platform C libraries that allow you to easily implement VNC server or client functionality in your program. It has a single fuzz driver that fuzzes the server code and reaches 19% of the project's code.  

Many components of this library are completely untouched, such as websocket code.  Roughly more than two thirds of the functions in main.c exhibit high complexity and are also not touched.
