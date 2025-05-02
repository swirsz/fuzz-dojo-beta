This module is identical to the projects module.  Take what you've learned from the 'Understanding Fuzz Drivers' module and implement your plan from the 'Planning New Fuzz Drivers' module to make a simple fuzz driver that achieves at least a 5% boost in total project coverage.
-

---

Use (Report link) to browse the source code and fuzz introspector report https://introspector.oss-fuzz.com/project-profile?project=libpng

Fuzz Introspector
https://storage.googleapis.com/oss-fuzz-introspector/libpng/inspector-report/20250411/fuzz_report.html

LIBPNG is the official library for the PNG, Portable Network Graphics, image format.  For a detailed description on using libpng, read `libpng-manual.txt`.  For examples of libpng in a program, see `example.c` and `pngtest.c`.

A single fuzz driver calls png_read_info and performs several transforms, but only reaches 50% of the code.  Lots of example code in `example.c` and `pngtest.c` could be turned into new fuzz drivers.
