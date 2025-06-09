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

normalize_name() {
    local name="$1"
    echo "$name" | sed 's/[^a-zA-Z0-9_-]/-/g' | sed 's/^-\+\|-\+$//g'
}

git() {
    local command="$1"
    local source="$2"
    local destination="$3"

    if [[ "$command" == "clone" ]]; then
        log_info "Running fuzzer: $source for project: $destination"
    fi
    
    
    
    
}
    
helper() {
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
    
