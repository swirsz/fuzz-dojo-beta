Run `/challenge/run` to copy fuzz drivers into `/home/hacker/fuzz-dojo/{project}`

Edit each of these files and add comments to the code to demonstrate your understanding of their operation. A flag will be provided when sufficient comments are added to each driver, and your comments will be evaluated later.
-

---

Use (Report link) to browse the source code and fuzz introspector report https://introspector.oss-fuzz.com/project-profile?project=janus-gateway

Fuzz Introspector
https://storage.googleapis.com/oss-fuzz-introspector/janus-gateway/inspector-report/20250411/fuzz_report.html

Janus is an open source, general purpose, WebRTC server.  Web Real-Time Communication is a free, open-source technology that enables real-time voice, video, and data communication between browsers and devices.

3 fuzz drivers cover 38% of the project code.  The RTP fuzz driver calls a long library list of functions, but there are quite a few that are not called that have significant complexity, such as janus_rtp_skew_compensate_audio, janus_rtp_skew_compensate_video, janus_rtp_simulcasting_prepare, janus_rtp_header_update, and janus_rtp_header_extension_get_from_id.   These and many more could be investigated for how they are Run `/challenge/run` to copy fuzz drivers into `/home/hacker/fuzz-dojo/{project}`

Edit each of these files and add comments to the code to demonstrate your understanding of their operation. A flag will be provided when sufficient comments are added to each driver, and your comments will be evaluated later.
-

---

Used in practice, and probably combined into this fuzz driver or a new one.
