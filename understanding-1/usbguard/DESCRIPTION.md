Run `/challenge/run` to copy fuzz drivers into `/home/hacker/fuzz-dojo/{project}`

Edit each of these files and add comments to the code to demonstrate your understanding of their operation. A flag will be provided when sufficient comments are added to each driver, and your comments will be evaluated later.
-

---

Use (Report link) to browse the source code and fuzz introspector report https://introspector.oss-fuzz.com/project-profile?project=usbguard

Fuzz Introspector
https://storage.googleapis.com/oss-fuzz-introspector/usbguard/inspector-report/20250411/fuzz_report.html

USBGuard is a software framework for implementing USB device authorization policies. 3 fuzz drivers reach 41% of the project code.  97% static code coverage indicates that large sections of the code could possibly be called by these existing fuzz drivers but currently is not.

For example, the appliesToWithConditions, a function in RulePrivate.cpp that checks if a rule applies to a given device with additional conditions is mostly untested.
