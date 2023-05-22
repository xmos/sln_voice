@Library('xmos_jenkins_shared_library@v0.20.0') _

getApproval()

pipeline {
    agent none
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
            defaultValue: '15.2.1',
            description: 'The XTC tools version'
        )
        booleanParam(name: 'NIGHTLY_TEST_ONLY',
            defaultValue: false,
            description: 'Tests that only run during nightly builds.')
    }    
    environment {
        PYTHON_VERSION = "3.8.11"
        VENV_DIRNAME = ".venv"
        BUILD_DIRNAME = "dist"
        VRD_TEST_RIG_TARGET = "xcore_voice_test_rig"
        SAMPLE_SUITE = "test_vectors"
    }    
    stages {
        stage('Setup') {
            agent {
                label 'vrd-us'
            }
            stages{
                stage('Checkout') {
                    steps {
                        checkout scm
                        sh 'git submodule update --init --recursive --depth 1 --jobs \$(nproc)'
                    }
                }
                stage('Build applications and firmware') {
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
                stage('Create virtual environment') {
                    steps {
                        // Create venv
                        sh "pyenv install -s $PYTHON_VERSION"
                        sh "~/.pyenv/versions/$PYTHON_VERSION/bin/python -m venv $VENV_DIRNAME"
                        // Install dependencies
                        withVenv() {
                            sh "pip install git+https://github0.xmos.com/xmos-int/xtagctl.git"
                            sh "pip install -r test/requirements.txt"
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
            }
        }
        stage('Standard tests') {
            agent {
                label 'vrd-us'
            }
            when {
                expression { params.NIGHTLY_TEST_ONLY == false }
            }
            stages{
                stage('Run Sample_Rate_Conversion test') {
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
                stage('Run Device_Firmware_Update test') {
                    steps {
                        withTools(params.TOOLS_VERSION) {
                            withVenv {
                                script {
                                    uid = sh(returnStdout: true, script: 'id -u').trim()
                                    gid = sh(returnStdout: true, script: 'id -g').trim()
                                    withXTAG(["$VRD_TEST_RIG_TARGET"]) { adapterIDs ->
                                        sh "docker run --rm -u $uid:$gid --privileged -v /dev/bus/usb:/dev/bus/usb -w /sln_voice -v $WORKSPACE:/sln_voice ghcr.io/xmos/xcore_voice_tester:develop bash -l test/device_firmware_update/check_dfu.sh " + adapterIDs[0]
                                    }
                                    sh "pytest test/device_firmware_update/test_dfu.py --readback_image test/device_firmware_update/test_output/readback_upgrade.bin --upgrade_image test/device_firmware_update/test_output/test_ffva_dfu_upgrade.bin"
                                }
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
        stage('Nightly tests') {
            agent {
                label 'vrd-us'
            }
            when {
                expression { params.NIGHTLY_TEST_ONLY == true }
            }
            stages{
                stage('Checkout') {
                    steps {
                        sh 'git clone git@github.com:xmos/amazon_wwe.git'
                    }
                }
                stage('Set up sample suite') {
                    steps {
                        sh "cp -r /projects_us/hydra_audio/xcore-voice_xvf3510_no_processing_xmos_test_suite_subset $SAMPLE_SUITE"
                        // List samples for log
                        sh "ls -la $SAMPLE_SUITE"
                    }
                }
                stage('Run FFVA Pipeline test') {
                    steps {
                        withTools(params.TOOLS_VERSION) {
                            withVenv {
                                script {
                                    withXTAG(["$VRD_TEST_RIG_TARGET"]) { adapterIDs ->
                                        sh "test/pipeline/check_pipeline.sh $BUILD_DIRNAME/test_pipeline_ffva_adec_altarch.xe $SAMPLE_SUITE test/pipeline/ffva_quick.txt test/pipeline/ffva_test_output $WORKSPACE/amazon_wwe " + adapterIDs[0]
                                    }
                                    sh "pytest test/pipeline/test_pipeline.py --log test/pipeline/ffva_test_output/results.csv"
                                }
                            }
                        }
                    }
                }
                stage('Run FFD Pipeline test') {
                    steps {
                        withTools(params.TOOLS_VERSION) {
                            withVenv {
                                script {
                                    withXTAG(["$VRD_TEST_RIG_TARGET"]) { adapterIDs ->
                                        sh "test/pipeline/check_pipeline.sh $BUILD_DIRNAME/test_pipeline_ffd.xe $SAMPLE_SUITE test/pipeline/ffd_quick.txt test/pipeline/ffd_test_output $WORKSPACE/amazon_wwe " + adapterIDs[0]
                                    }
                                    sh "pytest test/pipeline/test_pipeline.py --log test/pipeline/ffd_test_output/results.csv"
                                }
                            }
                        }
                    }
                }
                stage('Run ASR test') {
                    steps {
                        withTools(params.TOOLS_VERSION) {
                            withVenv {
                                script {
                                    withXTAG(["$VRD_TEST_RIG_TARGET"]) { adapterIDs ->
                                        sh "test/asr/check_asr.sh Sensory $SAMPLE_SUITE test/asr/ffd_quick.txt test/asr/sensory_output " + adapterIDs[0]
                                    }
                                    sh "pytest test/asr/test_asr.py --log test/asr/sensory_output/results.csv"
                                }
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
    }
}
