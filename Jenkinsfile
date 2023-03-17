@Library('xmos_jenkins_shared_library@v0.20.0') _

// Wait here until specified artifacts appear
def artifactUrls = getGithubArtifactUrls([
    "host_apps",
    "xcore_voice_example_apps",
    "xcore_voice_test_apps"
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
        BUILD_DIRNAME = "dist"
        VRD_TEST_RIG_TARGET = "xcore_voice_test_rig"
    }    
    stages {
        stage('Checkout') {
            steps {
                checkout scm
                sh "git clone git@github.com:xmos/sln_voice.git"
            }
        }        
        stage('Build artifacts') {
            steps {
                withTools(params.TOOLS_VERSION) {
                    dir("$BUILD_DIRNAME") {
                        // host apps
                        sh "docker pull ghcr.io/xmos/xcore_builder:latest"
                        sh "docker run --rm -w /xcore_sdk -v $WORKSPACE:/xcore_sdk ghcr.io/xmos/xcore_builder:latest bash -l tools/ci/build_host_apps.sh"
                        
                        // example apps

                        // test apps

                        // List extracted files for log
                        sh "ls -la"
                    }
                }
            }
        }


        // stage('Create virtual environment') {
        //     steps {
        //         // Create venv
        //         sh "pyenv install -s $PYTHON_VERSION"
        //         sh "~/.pyenv/versions/$PYTHON_VERSION/bin/python -m venv $VENV_DIRNAME"
        //         // Install dependencies
        //         withVenv() {
        //             sh "pip install git+https://github0.xmos.com/xmos-int/xtagctl.git"
        //             sh "pip install -r sln_voice/test/requirements.txt"
        //         }
        //     }
        // }
        // stage('Cleanup xtagctl') {
        //     steps {
        //         // Cleanup any xtagctl cruft from previous failed runs
        //         withTools(params.TOOLS_VERSION) {
        //             withVenv {
        //                 sh "xtagctl reset_all $VRD_TEST_RIG_TARGET"
        //             }
        //         }
        //         sh "rm -f ~/.xtag/status.lock ~/.xtag/acquired"
        //     }
        // }
        // stage('Run Sample_Rate_Conversion test') {
        //     steps {
        //         withTools(params.TOOLS_VERSION) {
        //             withVenv {
        //                 script {
        //                     if (fileExists("$BUILD_DIRNAME/example_ffva_sample_rate_conv_test.xe")) {
        //                         withXTAG(["$VRD_TEST_RIG_TARGET"]) { adapterIDs ->
        //                             sh "test/sample_rate_conversion/check_sample_rate_conversion.sh $BUILD_DIRNAME/example_ffva_sample_rate_conv_test.xe test/sample_rate_conversion/test_output " + adapterIDs[0]
        //                         }
        //                         sh "pytest test/sample_rate_conversion/test_sample_rate_conversion.py --wav_file test/sample_rate_conversion/test_output/sample_rate_conversion_output.wav --wav_duration 10"
        //                     } else {
        //                         echo 'SKIPPED: Sample_Rate_Conversion test'
        //                     }
        //                 }
        //             }
        //         }
        //     }
        // }
        // stage('Run GPIO test') {
        //     steps {
        //         withTools(params.TOOLS_VERSION) {
        //             withVenv {
        //                 script {
        //                     if (fileExists("$BUILD_DIRNAME/example_test_ffd_gpio_test.xe")) {
        //                         sh "test/ffd_gpio/run_tests.sh"
        //                         sh 'python tools/ci/python/parse_test_output.py testing/test.rpt -outfile="testing/test_results" --print_test_results --verbose'
        //                     } else {
        //                         echo 'SKIPPED: GPIO test'
        //                     }
        //                 }
        //             }
        //         }
        //     }
        // }
        // stage('Run FFD Low Power Audio Buffer test') {
        //     steps {
        //         withTools(params.TOOLS_VERSION) {
        //             withVenv {
        //                 script {
        //                     if (fileExists("$BUILD_DIRNAME/example_test_ffd_low_power_audio_buffer_test.xe")) {
        //                         sh "test/ffd_low_power_audio_buffer/run_tests.sh"
        //                         sh "pytest test/ffd_low_power_audio_buffer/test_verify_low_power_audio_buffer.py"
        //                     } else {
        //                         echo 'SKIPPED: FFD Low Power Audio Buffer test'
        //                     }
        //                 }
        //             }
        //         }
        //     }
        // }
        // stage('Run Device_Firmware_Update test') {
        //     steps {
        //         withTools(params.TOOLS_VERSION) {
        //             withVenv {
        //                 script {
        //                     if (fileExists("$BUILD_DIRNAME/example_ffva_ua_adec_test.xe")) {
        //                         sh "docker pull ghcr.io/xmos/xcore_voice_tester:develop"
        //                         withXTAG(["$VRD_TEST_RIG_TARGET"]) { adapterIDs ->
        //                             sh "docker run --rm --privileged -v /dev/bus/usb:/dev/bus/usb -w /sln_voice -v $WORKSPACE:/sln_voice ghcr.io/xmos/xcore_voice_tester:develop bash -l test/device_firmware_update/check_dfu.sh $BUILD_DIRNAME/example_ffva_ua_adec_test.xe test/device_firmware_update/test_output " + adapterIDs[0]
        //                         }
        //                         sh "pytest test/device_firmware_update/test_dfu.py --readback_image test/device_firmware_update/test_output/readback_upgrade.bin --upgrade_image test/device_firmware_update/test_output/example_ffva_ua_adec_test_upgrade.bin"
        //                     } else {
        //                         echo 'SKIPPED: Device_Firmware_Update test'
        //                     }
        //                 }
        //             }
        //         }
        //     }
        // }


        // TODO the commands and pipeline tests require the testing suite sample files
        // stage('Run Commands test') {
        //     steps {
        //         withTools(params.TOOLS_VERSION) {
        //             withVenv {
        //                 script {
        //                     if (fileExists("$BUILD_DIRNAME/example_ffd_usb_audio_test.xe")) {
        //                         sh "test/commands/check_commands.sh $BUILD_DIRNAME/example_ffd_usb_audio_test.xe $BUILD_DIRNAME/samples test/commands/ffd.txt test/commands/test_output " + adapterIDs[0]
        //                         sh "pytest test/commands/test_commands.py --log test/commands/test_output/results.csv"
        //                     } else {
        //                         echo 'SKIPPED: Commands test' 
        //                     }
        //                 }
        //             }
        //         }
        //     }
        // }
        // TODO both pipeline tests require the amazon wakeword engine repo. They can maybe be combined into one test stage.
        // stage('Run Pipeline FFD test') {
        //     steps {
        //         withTools(params.TOOLS_VERSION) {
        //             withVenv {
        //                 script {
        //                     if (fileExists("$BUILD_DIRNAME/example_ffd_usb_audio_test.xe")) {
        //                         sh "test/pipeline/check_pipeline.sh $BUILD_DIRNAME/example_ffd_usb_audio_test.xe <path-to-input-dir> <path-to-input-list> <path-to-output-dir> <path-to-amazon-wwe> " + adapterIDs[0]
        //                         sh "pytest test/pipeline/test_pipeline.py --log test/pipeline/test_output/results.csv"
        //                     } else {
        //                         echo 'SKIPPED: Pipeline FFD test' 
        //                     }
        //                 }
        //                 
        //             }
        //         }
        //     }
        // }
        // stage('Run Pipeline FFVA test') {
        //     steps {
        //         withTools(params.TOOLS_VERSION) {
        //             withVenv {
        //                 script {
        //                     if (fileExists("$BUILD_DIRNAME/example_ffva_ua_adec_test.xe")) {
        //                         sh "test/pipeline/check_pipeline.sh $BUILD_DIRNAME/example_ffva_ua_adec_test.xe <path-to-input-dir> <path-to-input-list> <path-to-output-dir> <path-to-amazon-wwe> " + adapterIDs[0]
        //                         sh "pytest test/pipeline/test_pipeline.py --log test/pipeline/test_output/results.csv"
        //                     } else {
        //                         echo 'SKIPPED: Pipeline FFVA test' 
        //                     }
        //                 }
        //             }
        //         }
        //     }
        // }
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
