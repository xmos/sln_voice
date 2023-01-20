#!/usr/bin/env bash

# Collection of helper functions that can be used in the different continuous
# integration scripts.

# A small utility to run a command and only print logs if the command fails.
# On success, all logs are hidden.
function log_errors {
    if log=$("$@" 2>&1); then
        echo "'$@' completed successfully!"
    else
        echo "$log"
        return 1
    fi    
}

function export_tools_version {
    xcc_version_output_string=`cat "$XMOS_TOOL_PATH"/doc/version.txt`
    # Find the semantic version substring
    prefix=${xcc_version_output_string%%" "*}
    space_position=${#prefix}
    xcc_semver_substring=`echo $xcc_version_output_string | cut -c1-$space_position`
    # Split semver substring into fields
    IFS='.' read -ra FIELDS <<< "$xcc_semver_substring"
    export XTC_VERSION_MAJOR=${FIELDS[0]}
    export XTC_VERSION_MINOR=${FIELDS[1]}
    export XTC_VERSION_PATCH=${FIELDS[2]}
}