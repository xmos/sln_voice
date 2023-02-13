@Library('xmos_jenkins_shared_library@v0.20.0') _

// Wait here until specified artifacts appear
def artifactUrls = getGithubArtifactUrls([
    "host_apps",
    "sln_voice_main_example_apps",
    "sln_voice_test_apps"
], 60)

getApproval()

pipeline {
    agent {
        label 'vrd-us'
    }
    options {
        disableConcurrentBuilds()
        skipDefaultCheckout()
        timestamps()
        // on develop discard builds after a certain number else keep forever
        buildDiscarder(logRotator(
            numToKeepStr:         env.BRANCH_NAME ==~ /develop/ ? '25' : '',
            artifactNumToKeepStr: env.BRANCH_NAME ==~ /develop/ ? '25' : ''
        ))
    }    
    parameters {
        string(
            name: 'TOOLS_VERSION',
            defaultValue: '15.1.4',
            description: 'The XTC tools version'
        )
    }    
    environment {
        PYTHON_VERSION = "3.8.11"
        VENV_DIRNAME = ".venv"
        DOWNLOAD_DIRNAME = "dist"
        VRD_TEST_RIG_TARGET = "xcore_voice_test_rig"
    }    
    stages {
        stage('Checkout') {
            steps {
                checkout scm
                sh "git clone git@github.com:xmos/sln_voice.git"
            }
        }        
        stage('Download artifacts') {
            steps {
                dir("$DOWNLOAD_DIRNAME") {
                    downloadExtractZips(artifactUrls)
                    // List extracted files for log
                    sh "ls -la"
                }
            }
        }
        stage('Create virtual environment') {
            steps {
                // Create venv
                sh "pyenv install -s $PYTHON_VERSION"
                sh "~/.pyenv/versions/$PYTHON_VERSION/bin/python -m venv $VENV_DIRNAME"
                // Install dependencies
                withVenv() {
                    sh "pip install git+https://github0.xmos.com/xmos-int/xtagctl.git"
                    sh "pip install -r sln_voice/test/requirements.txt"
                }
            }
        }
        stage('Cleanup xtagctl') {
            steps {
                // Cleanup any xtagctl cruft from previous failed runs
                withTools(params.TOOLS_VERSION) {
                    withVenv {
                        sh "xtagctl reset_all $VRD_TEST_RIG_TARGET"
                    }
                }
                sh "rm -f ~/.xtag/status.lock ~/.xtag/acquired"
            }
        }
        stage('Run Sample_Rate_Conversion test') {
            steps {
                withTools(params.TOOLS_VERSION) {
                    withVenv {
                        script {
                            if (fileExists("$DOWNLOAD_DIRNAME/example_ffva_sample_rate_conv_test.xe")) {
                                withXTAG(["$VRD_TEST_RIG_TARGET"]) { adapterIDs ->
                                    sh "test/sample_rate_conversion/check_sample_rate_conversion.sh $DOWNLOAD_DIRNAME/example_ffva_sample_rate_conv_test.xe test/sample_rate_conversion/test_output " + adapterIDs[0]
                                }
                            } else {
                                echo 'SKIPPED: ${TEST_SCRIPT_SRCT}'
                            }
                        }
                        sh "pytest test/sample_rate_conversion/test_sample_rate_conversion.py --wav_file test/sample_rate_conversion/test_output/sample_rate_conversion_output.wav --wav_duration 10"
                    }
                }
            }
        }
        stage('Run GPIO test') {
            steps {
                withTools(params.TOOLS_VERSION) {
                    withVenv {
                        script {
                            if (fileExists("$DOWNLOAD_DIRNAME/example_test_ffd_gpio_test.xe")) {
                                sh "test/ffd_gpio/run_tests.sh"
                            } else {
                                echo 'SKIPPED: ${TEST_SCRIPT_GPIO}' 
                            }
                        }
                        sh "python tools/ci/python/parse_test_output.py testing/test.rpt -outfile="<output_dir>/output_file" --print_test_results --verbose"
                    }
                }
            }
        }
    }
    post {
        cleanup {
            // cleanWs removes all output and artifacts of the Jenkins pipeline
            //   Comment out this post section to leave the workspace which can be useful for running items on the Jenkins agent. 
            //   However, beware that this pipeline will not run if the workspace is not manually cleaned.
            cleanWs()
        }
    }    
}