#!/bin/bash
set -e

# help text
help()
{
   echo "XCORE-VOICE module PDF builder script"
   echo
   echo "Syntax: build_pdfs.sh"
   echo
   echo "options:"
   echo "h     Print this Help."
}

function contains {
  local list="$1"
  local item="$2"
  if [[ ${list[*]} =~ ${item} ]] ; then
    # yes, list include item
    result=0
  else
    result=1
  fi
  return $result
}

# flag arguments
while getopts h option
do
    case "${option}" in
        h) help
           exit;;
    esac
done

XCORE_VOICE_ROOT=`git rev-parse --show-toplevel`

source ${XCORE_VOICE_ROOT}/tools/ci/helper_functions.sh

# setup distribution folder
DIST_DIR=${XCORE_VOICE_ROOT}/dist_pdfs
mkdir -p ${DIST_DIR}

# setup configurations
# row format is: "module_path  generated_filename   final_filename"
modules=(
    "modules/io       programming_guide.pdf       peripheral_io_programming_guide.pdf"
    "modules/rtos     programming_guide.pdf       rtos_programming_guide.pdf"
    "modules/rtos     build_system_guide.pdf      build_system_guide.pdf"
)


built_paths=()

# perform builds
for ((i = 0; i < ${#modules[@]}; i += 1)); do
    read -ra FIELDS <<< ${modules[i]}
    rel_path="${FIELDS[0]}"
    gen_name="${FIELDS[1]}"
    fin_name="${FIELDS[2]}"
    full_path="${XCORE_VOICE_ROOT}/${rel_path}"

    if [[ ! ${built_paths[*]} =~ ${rel_path} ]] ; then
        echo '******************************************************'
        echo '* Building PDFs for' ${rel_path}
        echo '******************************************************'

        # build docs
        (cd ${full_path}; docker run --rm -t -u "$(id -u):$(id -g)" -v $(pwd):/build -e PDF=1 -e REPO:/build -e DOXYGEN_INCLUDE=/build/doc/Doxyfile.inc -e EXCLUDE_PATTERNS=/build/doc/exclude_patterns.inc -e DOXYGEN_INPUT=ignore ghcr.io/xmos/doc_builder:latest)

        # append to built paths so we do not build it again
        built_paths+=( ${rel_path} )
    fi

    # copy to dist folder
    (cd ${full_path}/doc/_build/pdf; cp ${gen_name} ${DIST_DIR}/${fin_name})
done
