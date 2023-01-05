@Library('xmos_jenkins_shared_library@v0.20.0') _

Wait here until specified artifacts appear
def artifactUrls = getGithubArtifactUrls([
    "host_apps",
    "sln_voice_example_apps",
    "sln_voice_test_apps"
])

getApproval()

pipeline {
    agent {
        label 'vrd'
    }
    options {
        disableConcurrentBuilds()
        skipDefaultCheckout()
        timestamps()
        // on develop discard builds after a certain number else keep forever
        buildDiscarder(logRotator(
            numToKeepStr:         env.BRANCH_NAME ==~ /origin/release/v0.21.0-beta.0/ ? '25' : '',
            artifactNumToKeepStr: env.BRANCH_NAME ==~ /origin/release/v0.21.0-beta.0/ ? '25' : ''
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
                    // sh "pip install git+https://github0.xmos.com/xmos-int/xtagctl.git"
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
        stage('Run tests') {
            steps {
                withTools(params.TOOLS_VERSION) {
                    withVenv {
                        script {
                            if (fileExists("$DOWNLOAD_DIRNAME/example_stlp_sample_rate_conv_test.xe")) {
                                withXTAG(["$VRD_TEST_RIG_TARGET"]) { adapterIDs ->
                                    sh "test/sample_rate_conversion/check_sample_rate_conversion.sh " + adapterIDs[0]
                                }
                            } else {
                                echo 'SKIPPED: example_stlp_sample_rate_conv_test'
                            }
                        } 
                    }
                }
            }
        }
    }
    post {
        cleanup {
            cleanWs()
        }
    }    
}