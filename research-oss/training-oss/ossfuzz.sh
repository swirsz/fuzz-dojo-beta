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
    elif [[ ! -d "/downloads/$source" ]]; then
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
    
