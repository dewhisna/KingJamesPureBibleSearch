pipeline {
    environment {
        QTDIR='~/Qt/5.15.2'
        BUILD_TARGET='Qt_5_15_2_gcc_64'
    }
    agent {
        docker {
            label 'docker'
            image 'localhost:5000/dewtronics/ubuntu_18.04_bionic_qt_5.15.2_jenkins_workspace'
        }
    }
    stages {
        stage('Setup') {
            steps {
                echo "Building ${env.JOB_NAME}..."
                sh '''
                    cd "${WORKSPACE}"
                    rm -rf *.tar.xz
                '''
            }
        }
        stage('GUI_Build') {
            steps {
                sh '''
                    mkdir -p "${WORKSPACE_TMP}/build-KJPBS-${BUILD_TARGET}/Release"
                    cd "${WORKSPACE_TMP}/build-KJPBS-${BUILD_TARGET}/Release"
                    cmake -S "${WORKSPACE}/programs/" -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=${QTDIR} -DOPTION_TEXT_TO_SPEECH=OFF
                    cmake --build . --target all --parallel 4
                '''
            }
        }
        stage('WebChannel_Build') {
            steps {
                sh '''
                    mkdir -p "${WORKSPACE_TMP}/build-KJPBS_webchannel-${BUILD_TARGET}/Release"
                    cd "${WORKSPACE_TMP}/build-KJPBS_webchannel-${BUILD_TARGET}/Release"
                    cmake -S "${WORKSPACE}/programs/" -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=${QTDIR} -DOPTION_TEXT_TO_SPEECH=OFF -DCONSOLE=ON -DWEBCHANNEL=ON
                    cmake --build . --target all --parallel 4
                '''
            }
        }
        stage('Deploy') {
            steps {
                sh '''
                    mkdir -p "${WORKSPACE_TMP}/KingJamesPureBibleSearch/KJVCanOpener/app"
                    mkdir -p "${WORKSPACE_TMP}/KingJamesPureBibleSearch/KJVCanOpener/db"
                    mkdir -p "${WORKSPACE_TMP}/KingJamesPureBibleSearch/KJVCanOpener/doc"
                    mkdir -p "${WORKSPACE_TMP}/KingJamesPureBibleSearch/KJVCanOpener/fonts"
                    mkdir -p "${WORKSPACE_TMP}/KingJamesPureBibleSearch/KJVCanOpener/geoip"
                    mkdir -p "${WORKSPACE_TMP}/KingJamesPureBibleSearch/KJVCanOpener/examples"
                    mkdir -p "${WORKSPACE_TMP}/KingJamesPureBibleSearch/KJVCanOpener/license"
                    mkdir -p "${WORKSPACE_TMP}/KingJamesPureBibleSearch/KJVDataParse"
                    #
                    cd "${WORKSPACE_TMP}/build-KJPBS-${BUILD_TARGET}/Release"
                    cp KJVCanOpener/KingJamesPureBibleSearch "${WORKSPACE_TMP}/KingJamesPureBibleSearch/KJVCanOpener/app/"
                    cp KJVDataDump/KJVDataDump "${WORKSPACE_TMP}/KingJamesPureBibleSearch/KJVCanOpener/app/"
                    cp KJVDataParse/KJVDataParse "${WORKSPACE_TMP}/KingJamesPureBibleSearch/KJVCanOpener/app/"
                    cp KJVDictWord/KJVDictWord "${WORKSPACE_TMP}/KingJamesPureBibleSearch/KJVCanOpener/app/"
                    cp KJVDiff/KJVDiff "${WORKSPACE_TMP}/KingJamesPureBibleSearch/KJVCanOpener/app/"
                    cp KJVLookup/KJVLookup "${WORKSPACE_TMP}/KingJamesPureBibleSearch/KJVCanOpener/app/"
                    cp KJVPhraseSearch/KJVPhraseSearch "${WORKSPACE_TMP}/KingJamesPureBibleSearch/KJVCanOpener/app/"
                    cp KJVSearch/KJVSearch "${WORKSPACE_TMP}/KingJamesPureBibleSearch/KJVCanOpener/app/"
                    cp KJVSumThing/KJVSumThing "${WORKSPACE_TMP}/KingJamesPureBibleSearch/KJVCanOpener/app/"
                    #
                    cd "${WORKSPACE}/programs/KJVCanOpener/db/"
                    cp bbl-*.ccdb "${WORKSPACE_TMP}/KingJamesPureBibleSearch/KJVCanOpener/db/"
                    cp dct-*.s3db "${WORKSPACE_TMP}/KingJamesPureBibleSearch/KJVCanOpener/db/"
                    #
                    cp "${WORKSPACE}/programs/KJVCanOpener/doc/KingJamesPureBibleSearch.pdf" "${WORKSPACE_TMP}/KingJamesPureBibleSearch/KJVCanOpener/doc/"
                    cp "${WORKSPACE}/programs/KJVCanOpener/articles/kjv_stats.xls" "${WORKSPACE_TMP}/KingJamesPureBibleSearch/KJVCanOpener/doc/"
                    cp "${WORKSPACE}/programs/KJVCanOpener/kjvdatagen/kjv_summary.xls" "${WORKSPACE_TMP}/KingJamesPureBibleSearch/KJVCanOpener/doc/"
                    #
                    cd "${WORKSPACE}/programs/KJVCanOpener/fonts/"
                    cp *.ttf "${WORKSPACE_TMP}/KingJamesPureBibleSearch/KJVCanOpener/fonts/"
                    cp *.TTF "${WORKSPACE_TMP}/KingJamesPureBibleSearch/KJVCanOpener/fonts/"
                    #
                    cp "${WORKSPACE}/programs/KJVCanOpener/geoip/GeoIP.conf" "${WORKSPACE_TMP}/KingJamesPureBibleSearch/KJVCanOpener/geoip/"
                    #
                    cd "${WORKSPACE}/programs/KJVCanOpener/examples/"
                    cp example*.kjs "${WORKSPACE_TMP}/KingJamesPureBibleSearch/KJVCanOpener/examples/"
                    #
                    cp "${WORKSPACE}/programs/KJVCanOpener/gpl-3.0.txt" "${WORKSPACE_TMP}/KingJamesPureBibleSearch/KJVCanOpener/license/"
                    #
                    cd "${WORKSPACE_TMP}/KingJamesPureBibleSearch/KJVCanOpener"
                    cp -r "${WORKSPACE_TMP}/build-KJPBS-${BUILD_TARGET}/Release/KJVCanOpener/translations" .
                    #
                    cd "${WORKSPACE_TMP}/KingJamesPureBibleSearch/KJVDataParse"
                    cp -r "${WORKSPACE_TMP}/build-KJPBS-${BUILD_TARGET}/Release/KJVDataParse/translations" .
                    #
                    cd "${WORKSPACE_TMP}/build-KJPBS_webchannel-${BUILD_TARGET}/Release"
                    cp KJVCanOpener/KingJamesPureBibleSearch "${WORKSPACE_TMP}/KingJamesPureBibleSearch/KJVCanOpener/app/KingJamesPureBibleSearch_webchannel"
                    cp -r KJVCanOpener/html "${WORKSPACE_TMP}/KingJamesPureBibleSearch/KJVCanOpener/app/"
                '''
            }
        }
        stage('Archive') {
            steps {
                sh '''
                    cd "${WORKSPACE}"
                    export GIT_DESC="g`git describe --long --always --dirty=-dirty`"
                    cd "${WORKSPACE_TMP}"
                    tar -Jcf "KingJamesPureBibleSearch-${BUILD_TARGET}-`date +"%y%m%d%H%M%S"`-${GIT_DESC}-jenkins-${BUILD_NUMBER}.tar.xz" KingJamesPureBibleSearch/
                    cp *.tar.xz "${WORKSPACE}/"
                    rm *.tar.xz
                    rm -rf "${WORKSPACE_TMP}/build-KJPBS-${BUILD_TARGET}"
                    rm -rf "${WORKSPACE_TMP}/build-KJPBS_webchannel-${BUILD_TARGET}"
                    rm -rf "${WORKSPACE_TMP}/KingJamesPureBibleSearch"
                '''
                archiveArtifacts artifacts: '*.tar.xz'
            }
        }
    }
    post {
        always {
            cleanWs(cleanWhenNotBuilt: false,
                    deleteDirs: true,
                    disableDeferredWipeout: true,
                    notFailBuild: true)
        }
    }
}

