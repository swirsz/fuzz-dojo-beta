#!/usr/bin/env python3
import json
import sys

print("###\n### Welcome to training!\n###\n")
print("In this challenge, you have a working fuzz driver that improves code coverage")
print("by calling functions higher in the call tree. Examine the source code of the")
print("current fuzz driver and the source code of the project to find 2 different")
print("functions that can be swapped for similar functions.\n")

print("Hints: Viewing the source code of the fuzz driver, you will see 4 calls to")
print("bzlib functions. You can either browse the source code directly or use")
print("the fuzz introspector report, per-fuzzer coverage to graphically see how")
print("many times sections of the code are executed or not executed. Another helpful")
print("view is the 'full calltree' option available under Fuzzer Details that will")
print("represent called or missed functions in a useful tree format.\n")

summary_file = "/out/fuzzer_stats/newfuzzer.json"
returncode = 0
try:
    with open(summary_file, "r") as fj:
        content = json.load(fj)
        for dd in content['data']:
            filenames = dd.get('files')
            for files in filenames:
                if files.get('filename') == "/src/bzip2/bzlib.c":
                    func = files['summary']['functions']['covered']
                    print("{0} functions reached. ".format(str(func)))
                    returncode = int(func)
except:
    print("Missing "+summary_file)
    print("\nPlease run /challenge/loc first\n")

exit(returncode)
