# A small utility to log metadata about the CI run.

#!/bin/bash
set -e

metadata_filename="$1"
template='{"actor":"%s","event":"%s","ref":"%s","sha":"%s"}'
json_string=$(printf "$template" "$GITHUB_ACTOR" "$GITHUB_EVENT_NAME" "$GITHUB_REF_NAME" "$GITHUB_SHA")
echo "Build metadata: $json_string"
echo "$json_string" >> $metadata_filename
