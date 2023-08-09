#!/usr/bin/env python3
import json
import sys

print("###\n### Welcome to training!\n###\n")
print("In this challenge, you have a working fuzz driver that improve code coverage")
print("by calling functions higher in the call tree. Examine the source code of the")
print("current fuzz driver and the source code of the project to find 2 different")
print("functions that can be swapped for similar functions.\n")

summary_file = "/out/fuzzer_stats/bzip2_decompress_target.json"
returncode = 0
try:
    with open(summary_file, "r") as fj:
        content = json.load(fj)
        for dd in content['data']:
            filenames = dd.get('files')
            for files in filenames:
                if files.get('filename') == "/src/bzip2_decompress_target.c":
                    func = files['summary']['branches']['covered']
                    print("{0} branches reached. ".format(str(func)))
                    returncode = int(func)
except:
    print("Missing "+summary_file)
    print("\nPlease run /challenge/loc first\n")

exit(returncode)
