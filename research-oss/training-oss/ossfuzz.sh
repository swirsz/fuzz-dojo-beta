#!/bin/bash

# Default values
DEFAULT_ARCHITECTURE="x86_64"
DEFAULT_ENGINE="libfuzzer"
DEFAULT_SANITIZER="address"
DEFAULT_LANGUAGE="c++"

# Logging functions
log_info() {
    echo "[INFO] $*" >&2
}


git() {
    local project_name="$1"
    local fuzzer_name="$2"
    local engine="${3:-$DEFAULT_ENGINE}"
    local sanitizer="${4:-$DEFAULT_SANITIZER}"
    local architecture="${5:-$DEFAULT_ARCHITECTURE}"
    local corpus_dir="$6"
    shift 6
    local fuzzer_args=("$@")
    
    log_info "Running fuzzer: $fuzzer_name for project: $project_name"
}
    
