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

log_error() {
    echo "[ERROR] $*" >&2
}


git() {
    local command="$1"
    local source="$2"
    local destination="$3"
    source="${source##*/}"  #strip off path
    
    if [[ -z "$source" || ! -z "$4" ]]; then
        log_info "Syntax: git clone {source} {destination}"
    elif [[ ! -d "/downloads/$source" && "$source" != "oss-fuzz" ]]; then
        log_error "Project $source is not available"
    else
        if [[ ! -n "$destination" ]]; then
            local destination="$source"
        fi
        log_info "git clone project: $source   to destination: $destination"
        mkdir -p $destination
        if [[ "$source" == "oss-fuzz" ]]; then
            cp -r /opt/$source/. $destination
        else
            cp -r /downloads/$source/. $destination
        fi
    fi   
}
    
helper() {
    local command="$1"
    local project_name="$2"
    local fuzzer_name="$3"
    local engine="${4:-$DEFAULT_ENGINE}"
    local sanitizer="${5:-$DEFAULT_SANITIZER}"
    local architecture="${6:-$DEFAULT_ARCHITECTURE}"
    local corpus_dir="$7"
    shift 7
    local fuzzer_args=("$@")
    
    log_info "command: $command"
    log_info "project_name: $project_name"
    log_info "fuzzer_name: $fuzzer_name"
    log_info "engine: $engine"
    log_info "sanitizer: $sanitizer"
    log_info "architecture: $architecture"
    log_info "corpus_dir: $corpus_dir"
    log_info "fuzzer_args: $fuzzer_args"

    if [[ "$command" == "build_image" ]]; then
        log_info
    elif [[ "$command" == "build_fuzzers" ]]; then
        log_info
    elif [[ "$command" == "check_build" ]]; then
        log_info
    elif [[ "$command" == "run_fuzzer" ]]; then
        log_info
    elif [[ "$command" == "coverage" ]]; then
        log_info
    elif [[ "$command" == "introspector" ]]; then
        log_info
    else
        log_error Syntax:  helper {command} {project name} {options}
    fi
}
    
