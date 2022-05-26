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

# A small utility to log metadata about the CI run.
function log_metadata {
    if $GITHUB_ACTIONS
    then
        metadata_filename="$@"
        template='{"actor":"%s","event":"%s","ref":"%s","sha":"%s"}'
        json_string=$(printf "$template" "$GITHUB_ACTOR" "$GITHUB_EVENT_NAME" "$GITHUB_REF_NAME" "$GITHUB_SHA")
        echo "Build metadata: $json_string"
        echo "$json_string" >> $metadata_filename
    fi    
}