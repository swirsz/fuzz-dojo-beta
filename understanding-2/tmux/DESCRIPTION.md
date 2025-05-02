Run `/challenge/run` to copy fuzz drivers into `/home/hacker/fuzz-dojo/{project}`

Edit each of these files and add comments to the code to demonstrate your understanding of their operation. A flag will be provided when sufficient comments are added to each driver, and your comments will be evaluated later.

---

Use (Report link) to browse the source code and fuzz introspector report https://introspector.oss-fuzz.com/project-profile?project=tmux

Fuzz Introspector
https://storage.googleapis.com/oss-fuzz-introspector/tmux/inspector-report/20250411/fuzz_report.html

Tmux is a terminal multiplexer written in C.  A single fuzz driver reaches only 14% of the code by fuzzing the input_parse_buffer.  The fuzz blockers highlighted by Fuzz Introspector show large sections of code that could be targetted by new fuzz drivers.  For example: window layout, cmdq_new_state (creates a new state object that represents the context in which commands are executed within tmux), args_parse (string arguments sent to tmux's command system)
