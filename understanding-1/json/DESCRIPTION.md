Run `/challenge/run` to copy fuzz drivers into `/home/hacker/fuzz-dojo/{project}`

Edit each of these files and add comments to the code to demonstrate your understanding of their operation. A flag will be provided when sufficient comments are added to each driver, and your comments will be evaluated later.

Use (Report link) to browse the source code and fuzz introspector report https://introspector.oss-fuzz.com/project-profile?project=json

Fuzz Introspector
https://storage.googleapis.com/oss-fuzz-introspector/json/inspector-report/20250411/fuzz_report.html

JSON is a single file json.hpp implementation of a json parser.

With 6 drivers achieving 74% coverage, it seems to have small sections of code that could be exercised by additional fuzz drivers, or additional corpus data. For example there are untouched functions like scan_comment(), start_object() 
