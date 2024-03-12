@Library('xmos_jenkins_shared_library@v0.28.0') _

getApproval()

pipeline {
    agent none
    options {
        disableConcurrentBuilds()
        skipDefaultCheckout()
        timestamps()
        // on develop discard builds after a certain number else keep forever
        buildDiscarder(xmosDiscardBuildSettings())
    }

    parameters {
        string(
            name: 'TOOLS_VERSION',
            defaultValue: '15.2.1',
            description: 'The XTC tools version'
        )
        booleanParam(name: 'NIGHTLY_TEST_ONLY',
            defaultValue: false,
            description: 'Tests that only run during nightly builds.')
    }
    environment {
        REPO = 'sln_voice'
        VIEW = getViewName(REPO)
        PYTHON_VERSION = "3.8.11"
        VENV_DIRNAME = ".venv"
        BUILD_DIRNAME = "dist"
        XMOSDOC_VERSION = 'v4.0'
        VRD_TEST_RIG_TARGET = "XCORE-AI-EXPLORER"
        PIPELINE_TEST_VECTORS = "pipeline_test_vectors"
        ASR_TEST_VECTORS = "asr_test_vectors"
    }
    stages {
        stage('Build and Docs') {
            parallel {
                stage('Build and Test') {
                    when {
                        expression { !env.GH_LABEL_DOC_ONLY.toBoolean() }
                    }
                    agent {
                        label 'xcore.ai && vrd'
                    }
                    stages {
                        stage('Checkout') {
                            steps {
                                checkout scm
                                sh 'git submodule update --init --recursive --depth 1 --jobs \$(nproc)'
                            }
                        }
                        stage('Build tests') {
                            steps {
                                script {
                                    uid = sh(returnStdout: true, script: 'id -u').trim()
                                    gid = sh(returnStdout: true, script: 'id -g').trim()
                                }
                                // pull docker images
                                sh "docker pull ghcr.io/xmos/xcore_builder:latest"
                                sh "docker pull ghcr.io/xmos/xcore_voice_tester:develop"
                                // host apps
                                sh "docker run --rm -u $uid:$gid -w /sln_voice -v $WORKSPACE:/sln_voice ghcr.io/xmos/xcore_builder:latest bash -l tools/ci/build_host_apps.sh"
                                // test firmware and filesystems
                                sh "docker run --rm -u $uid:$gid -w /sln_voice -v $WORKSPACE:/sln_voice ghcr.io/xmos/xcore_builder:latest bash -l tools/ci/build_tests.sh"
                                // List built files for log
                                sh "ls -la dist_host/"
                                sh "ls -la dist/"
                            }
                        }

                        stage('Run ASR test') {
                            //when {
                            //    expression { params.NIGHTLY_TEST_ONLY == true }
                            //}
                            steps {
                                withTools(params.TOOLS_VERSION) {
                                    withVenv {
                                        script {
                                            withXTAG(["$VRD_TEST_RIG_TARGET"]) { adapterIDs ->
                                                sh "test/asr/check_asr.sh Sensory $ASR_TEST_VECTORS test/asr/ffd_quick_sensory.txt test/asr/sensory_output " + adapterIDs[0]
                                                sh "test/asr/check_asr.sh Cyberon $ASR_TEST_VECTORS test/asr/ffd_quick_cyberon.txt test/asr/cyberon_output " + adapterIDs[0]
                                            }
                                            sh "pytest test/asr/test_asr.py --log test/asr/sensory_output/results.csv"
                                            sh "pytest test/asr/test_asr.py --log test/asr/cyberon_output/results.csv"

                                        }
                                    }
                                }
                            }
                        }
                    }
                    post {
                        cleanup {
                            // xcoreCleanSandbox removes all output and artifacts of the Jenkins pipeline
                            //   Comment out this post section to leave the workspace which can be useful for running items on the Jenkins agent.
                            //   However, beware that this pipeline will not run if the workspace is not manually cleaned.
                            xcoreCleanSandbox()
                        }
                    }
                }
                stage('Build docs') {
                    agent { label 'docker' }
                    stages {
                        stage ('Build docs with docker') {
                            steps {
                                checkout scm
                                sh 'git submodule update --init --recursive --depth 1 --jobs \$(nproc)'
                                sh "docker pull ghcr.io/xmos/xmosdoc:$XMOSDOC_VERSION"
                                sh """docker run -u "\$(id -u):\$(id -g)" \
                                        --rm \
                                        -v ${WORKSPACE}:/build \
                                        ghcr.io/xmos/xmosdoc:$XMOSDOC_VERSION -v"""
                                // Zip all the generated files
                                zip dir: "doc/_build/", zipFile: "xcore_voice_docs_original.zip"
                                // Rename latex folder as pdf
                                sh "mv doc/_build/latex doc/_build/pdf"
                                // Update links to latex folder in html files
                                sh "find doc/_build/html -type f -exec sed -i -e 's/latex\\/sln_voice_programming_guide_/pdf\\/sln_voice_programming_guide_/g' {} \\;"
                                sh "find doc/_build/html -type f -exec sed -i -e 's/latex\\/sln_voice_quick_start_guide_/pdf\\/sln_voice_quick_start_guide_/g' {} \\;"
                                // Remove linkcheck folder
                                sh "rm -rf doc/_build/linkcheck"
                                // Zip all the generated files
                                zip dir: "doc/_build/", zipFile: "xcore_voice_docs_release.zip"
                                // Archive doc files
                                archiveArtifacts artifacts: "xcore_voice_docs*.zip"
                            }
                        }
                    }
                    post {
                        cleanup {
                            xcoreCleanSandbox()
                        }
                    }
                }
            }
        }
    }
}
