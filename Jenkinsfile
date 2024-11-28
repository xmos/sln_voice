@Library('xmos_jenkins_shared_library@v0.34.0') _

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
            defaultValue: '15.3.0',
            description: 'The XTC tools version'
        )
        string(
            name: 'XMOSDOC_VERSION',
            defaultValue: 'v6.2.0',
            description: 'The xmosdoc version'
        )
        booleanParam(name: 'NIGHTLY_TEST_ONLY',
            defaultValue: false,
            description: 'Tests that only run during nightly builds.'
        )
        booleanParam(name: 'FORCE_FULL_RUN',
            defaultValue: true,
            description: 'Force to run all tests, including nigthly.'
        )
    } // parameters
    environment {
        REPO = 'sln_voice'
        VIEW = getViewName(REPO)
        VENV_DIRNAME = ".venv"
        BUILD_DIRNAME = "dist"
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
                        label 'sw-hw-xcai-vrd0' //TODO put back
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
                                createVenv(reqFile: "requirements.txt")
                                withTools(params.TOOLS_VERSION) {
                                    withVenv {
                                        // host apps
                                        sh "tools/ci/build_host_apps.sh"
                                        // test firmware and filesystems
                                        sh "tools/ci/build_tests.sh"
                                        // List built files for log
                                        sh "ls -la dist_host/"
                                        sh "ls -la dist/"
                                    } // venv
                                } // tools
                            }
                        }
                        stage('ASRC Unit tests') {
                            steps {
                                withTools(params.TOOLS_VERSION) {
                                    // tools/ci/build_tests.sh does not build for x86
                                    sh "mkdir -p build_x86"
                                    sh "cmake -B build_x86 -DXCORE_VOICE_TESTS=ON"
                                    sh "cmake --build build_x86 --target test_asrc_div -j8"
                                    // x86 build
                                    sh "./build_x86/test_asrc_div"
                                    // xcore build
                                    sh "xsim dist/test_asrc_div.xe"
                                }
                            }
                        }


                        stage('ASRC Simulator') {
                            steps {
                                withTools(params.TOOLS_VERSION) {
                                    dir("test/asrc_sim") {
                                        createVenv('requirements.txt')
                                        withVenv {
                                            sh "pip install -r ./requirements.txt"
                                            sh './run.sh'
                                        }
                                    }
                                }
                            }
                        }
                        stage('Install test requirements') {
                            when {
                                expression { params.NIGHTLY_TEST_ONLY == true || params.FORCE_FULL_RUN == true}
                            }
                            steps {
                                // Install dependencies
                                withVenv() {
                                    sh "pip install git+https://github0.xmos.com/xmos-int/xtagctl.git"
                                    sh "pip install -r test/requirements.txt"
                                }
                            }
                        }
                        stage('Cleanup xtagctl') {
                            when {
                                expression { params.NIGHTLY_TEST_ONLY == true || params.FORCE_FULL_RUN == true}
                            }
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
                        stage('Run Sample Rate Conversion test') {
                            when {
                                expression { params.NIGHTLY_TEST_ONLY == true || params.FORCE_FULL_RUN == true}
                            }
                            steps {
                                withTools(params.TOOLS_VERSION) {
                                    withVenv {
                                        script {
                                            withXTAG(["$VRD_TEST_RIG_TARGET"]) { adapterIDs ->
                                                sh "test/sample_rate_conversion/check_sample_rate_conversion.sh " + adapterIDs[0]
                                            }
                                            sh "pytest test/sample_rate_conversion/test_sample_rate_conversion.py --wav_file test/sample_rate_conversion/test_output/sample_rate_conversion_output.wav --wav_duration 10"
                                        }
                                    }
                                }
                            }
                        }
                        stage('Run GPIO test') {
                            when {
                                expression { params.NIGHTLY_TEST_ONLY == true || params.FORCE_FULL_RUN == true}
                            }
                            steps {
                                withTools(params.TOOLS_VERSION) {
                                    withVenv {
                                        script {
                                            sh "test/ffd_gpio/run_tests.sh"
                                            sh 'python tools/ci/python/parse_test_output.py testing/test.rpt -outfile="testing/test_results" --print_test_results --verbose'
                                        }
                                    }
                                }
                            }
                        }
                        stage('Run FFD Low Power Audio Buffer test') {
                            when {
                                expression { params.NIGHTLY_TEST_ONLY == true || params.FORCE_FULL_RUN == true}
                            }
                            steps {
                                withTools(params.TOOLS_VERSION) {
                                    withVenv {
                                        script {
                                            sh "test/ffd_low_power_audio_buffer/run_tests.sh"
                                            sh "pytest test/ffd_low_power_audio_buffer/test_verify_low_power_audio_buffer.py"
                                        }
                                    }
                                }
                            }
                        }
                        stage('Run Device Firmware Update test') {
                            when {
                                expression { params.NIGHTLY_TEST_ONLY == true || params.FORCE_FULL_RUN == true}
                            }
                            steps {
                                withTools(params.TOOLS_VERSION) {
                                    withVenv {
                                        script {
                                            uid = sh(returnStdout: true, script: 'id -u').trim()
                                            gid = sh(returnStdout: true, script: 'id -g').trim()
                                            withXTAG(["$VRD_TEST_RIG_TARGET"]) { adapterIDs ->
                                                sh "test/device_firmware_update/check_dfu.sh " + adapterIDs[0]
                                            }
                                            sh "pytest test/device_firmware_update/test_dfu.py --readback_image test/device_firmware_update/test_output/readback_upgrade.bin --upgrade_image test/device_firmware_update/test_output/test_ffva_dfu_upgrade.bin"
                                        }
                                    }
                                }
                            }
                        }
                        stage('Checkout Amazon WWE') {
                            when {
                                expression { params.NIGHTLY_TEST_ONLY == true || params.FORCE_FULL_RUN == true}
                            }
                            steps {
                                sh 'git clone git@github.com:xmos/amazon_wwe.git'
                            }
                        }
                        stage('Setup test vectors') {
                            when {
                                expression { params.NIGHTLY_TEST_ONLY == true || params.FORCE_FULL_RUN == true}
                            }
                            steps {
                                sh "cp -r /projects/hydra_audio/xcore-voice_xvf3510_no_processing_xmos_test_suite_subset $PIPELINE_TEST_VECTORS"
                                sh "ls -la $PIPELINE_TEST_VECTORS"
                                sh "cp -r /projects/hydra_audio/xcore-voice_no_processing_ffd_test_suite $ASR_TEST_VECTORS"
                                sh "ls -la $ASR_TEST_VECTORS"
                            }
                        }
                        stage('Run FFVA Pipeline test') {
                            when {
                                expression { params.NIGHTLY_TEST_ONLY == true || params.FORCE_FULL_RUN == true}
                            }
                            steps {
                                withTools(params.TOOLS_VERSION) {
                                    withVenv {
                                        script {
                                            withXTAG(["$VRD_TEST_RIG_TARGET"]) { adapterIDs ->
                                                sh "xtagctl reset " + adapterIDs[0]
                                                sh "pip install -e modules/xscope_fileio/xscope_fileio/"
                                                sh "cp modules/xscope_fileio/xscope_fileio/host/xscope_host_endpoint dist_host/"
                                                sh "test/pipeline/check_pipeline.sh $BUILD_DIRNAME/test_pipeline_ffva_adec_altarch.xe $PIPELINE_TEST_VECTORS test/pipeline/ffva_quick.txt test/pipeline/ffva_test_output $WORKSPACE/amazon_wwe " + adapterIDs[0]
                                            }
                                            sh "pytest test/pipeline/test_pipeline.py --log test/pipeline/ffva_test_output/results.csv"
                                        }
                                    }
                                }
                            }
                        }
                        stage('Run FFD Pipeline test') {
                            when {
                                expression { params.NIGHTLY_TEST_ONLY == true || params.FORCE_FULL_RUN == true}
                            }
                            steps {
                                withTools(params.TOOLS_VERSION) {
                                    withVenv {
                                        script {
                                            withXTAG(["$VRD_TEST_RIG_TARGET"]) { adapterIDs ->
                                                sh "test/pipeline/check_pipeline.sh $BUILD_DIRNAME/test_pipeline_ffd.xe $PIPELINE_TEST_VECTORS test/pipeline/ffd_quick.txt test/pipeline/ffd_test_output $WORKSPACE/amazon_wwe " + adapterIDs[0]
                                            }
                                            sh "pytest test/pipeline/test_pipeline.py --log test/pipeline/ffd_test_output/results.csv"
                                        }
                                    }
                                }
                            }
                        }
                        stage('Run ASR test') {
                            when {
                                expression { params.NIGHTLY_TEST_ONLY == true || params.FORCE_FULL_RUN == true}
                            }
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
                stage('Build Documentation') {
                    agent {
                        label 'documentation&&docker'
                    }
                    steps {
                        checkout scm
                        sh 'git submodule update --init --recursive --depth 1 --jobs \$(nproc)'
                        warnError("Docs") {
                            buildDocs()
                        } // warnError("Docs")
                    } // steps
                    post {
                        cleanup {
                            xcoreCleanSandbox()
                        }
                    }
                } // stage('Build Documentation')

            }
        }
    }
}
