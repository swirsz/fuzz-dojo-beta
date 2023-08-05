#!/usr/bin/env python3
import json
import sys

print("###\n### Welcome to "+sys.argv[0]+"!\n###\n")
print("In this challenge, you have a working fuzz driver that improve code coverage")
print("by calling functions higher in the call tree. Examine the source code of the")
print("current fuzz driver and the source code of the project to find 2 different")
print("function that can be swapped for similar functions.\n")
print("Note: Fuzzing is non-deterministic. You may need to run /challenge/loc again.\n")

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
