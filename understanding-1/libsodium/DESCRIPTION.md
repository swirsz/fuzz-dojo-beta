Run `/challenge/run` to copy fuzz drivers into `/home/hacker/fuzz-dojo/{project}`

Edit each of these files and add comments to the code to demonstrate your understanding of their operation. A flag will be provided when sufficient comments are added to each driver, and your comments will be evaluated later.

Use (Report link) to browse the source code and fuzz introspector report https://introspector.oss-fuzz.com/project-profile?project=libsodium
       
Fuzz Introspector
https://storage.googleapis.com/oss-fuzz-introspector/libsodium/inspector-report/20250411/fuzz_report.html

LibSodium is an easy-to-Run `/challenge/run` to copy fuzz drivers into `/home/hacker/fuzz-dojo/{project}`

Edit each of these files and add comments to the code to demonstrate your understanding of their operation. A flag will be provided when sufficient comments are added to each driver, and your comments will be evaluated later.

Use software library for encryption, decryption, signatures, password hashing, and more. 

This library has roughly 20% code coverage with only 2 fuzz drivers, exercising the encryption, decryption, and maybe hashing functions and very little else.  Just implementing ED25519 key signatures as a fuzz driver should greatly increase the code coverage of the project