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
        log_info "Build Image: $project_name"
        /challenge/build
    elif [[ "$command" == "build_fuzzers" ]]; then
        log_info "Build Fuzzers: $project_name"
        /challenge/build
    elif [[ "$command" == "check_build" ]]; then
        log_info "Check Build: $project_name"
        /challenge/build
    elif [[ "$command" == "run_fuzzer" ]]; then
        log_info "Run Fuzzer: $project_name  options $@"
        run_fuzzer $fuzzer_name $@
    elif [[ "$command" == "coverage" ]]; then
        log_info "Coverage: $project_name"
        rm -rf /corpus/$fuzzer_name
        mv /tmp/$fuzzer_name_corpus /corpus/$fuzzer_name
        
        coverage
        
        cp -r /out/report_target/$fuzzer_name /out/report/$fuzzer_name
    elif [[ "$command" == "introspector" ]]; then
        log_info "Run Fuzzer: $project_name"
        /challenge/build introspector
                
        DRIVERS=(/src/inspector/*.covreport)

        rm -rf /out/introspector-report
        mkdir -p /out/introspector-report
        mv /out/inspector /out/introspector-report/inspector
        mv /out/report /out/introspector-report/inspector/covreport

        # fix delays caused by internet references to unpkg.com & googletagmanager.com
        patch -tuN /out/introspector-report/inspector/fuzz_report.html -i /usr/local/sbin/fuzz_report.patch -r -

        for f in /out/introspector-report/inspector/calltree_view*.html ; do patch -tuN "$f" -i /usr/local/sbin/calltree_view.patch -r -; done
        for f in /out/report_target/*; do mv $f /out/introspector-report/inspector/covreport/$(basename $f); done

        zip -y /home/hacker/.log/$(TZ=US/Arizona date +"%F--%H-%M-%S")-fi /out/introspector-report/inspector/fuzz_report.html
        echo
        echo Workspace:
        echo  Coverage: https://pwn.college/workspace/code/proxy/8008/covreport
        echo  Fuzz-Introspector: https://pwn.college/workspace/code/proxy/8008/fuzz_report.html
        echo
        echo Virtual Desktop under Applications/Web Browser:
        echo  Coverage: http://127.0.0.1:8008/covreport
        echo  Fuzz-Introspector: http://127.0.0.1:8008/fuzz_report.html
        echo
        echo If you encounter errors, run /challenge/fuzz-introspector under shell or Workspace first, then view report on Desktop/Web Browser
        echo
        echo \'fi-report\' to restart web server only
        echo 
        python3 -m http.server 8008 --directory /out/introspector-report/inspector/
    else
        log_error Syntax:  helper {command} {project name} {options}
    fi
}
    
