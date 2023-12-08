def commonBuildStages(QTDIR, BUILD_ENV, BUILD_ARCH, BUILD_TARGET) {
    stages = {
        stage('Setup') {
            checkout scm
            sh '''
                export QTDIR='''+QTDIR+'''
                export BUILD_ENV='''+BUILD_ENV+'''
                export BUILD_ARCH='''+BUILD_ARCH+'''
                export BUILD_TARGET='''+BUILD_TARGET+'''
                cd "${WORKSPACE}"
                echo "`date +%y%m%d%H%M%S`-g`git describe --long --always --dirty=-dirty`-${BUILD_ENV}" > "${WORKSPACE_TMP}/VERSION-${BUILD_TARGET}.txt"
                cd "${WORKSPACE_TMP}"
                export VERSION=$(cat VERSION-${BUILD_TARGET}.txt | tr -d '[:space:]')
                echo "Version Descriptor: ${VERSION}"
            '''
        }
        stage('GUI_Build') {
            sh '''
                export QTDIR='''+QTDIR+'''
                export BUILD_ENV='''+BUILD_ENV+'''
                export BUILD_ARCH='''+BUILD_ARCH+'''
                export BUILD_TARGET='''+BUILD_TARGET+'''
                mkdir -p "${WORKSPACE_TMP}/build-KJPBS-${BUILD_TARGET}/Release"
                cd "${WORKSPACE_TMP}/build-KJPBS-${BUILD_TARGET}/Release"
                cmake -S "${WORKSPACE}/programs/" -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=${QTDIR} -DOPTION_TEXT_TO_SPEECH=OFF
                cmake --build . --target all --parallel 4
            '''
        }
        stage('WebChannel_Build') {
            sh '''
                export QTDIR='''+QTDIR+'''
                export BUILD_ENV='''+BUILD_ENV+'''
                export BUILD_ARCH='''+BUILD_ARCH+'''
                export BUILD_TARGET='''+BUILD_TARGET+'''
                mkdir -p "${WORKSPACE_TMP}/build-KJPBS_webchannel-${BUILD_TARGET}/Release"
                cd "${WORKSPACE_TMP}/build-KJPBS_webchannel-${BUILD_TARGET}/Release"
                cmake -S "${WORKSPACE}/programs/" -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=${QTDIR} -DOPTION_TEXT_TO_SPEECH=OFF -DCONSOLE=ON -DWEBCHANNEL=ON
                cmake --build . --target KingJamesPureBibleSearch --parallel 4
            '''
        }
        stage('Deploy') {
            sh '''
                export QTDIR='''+QTDIR+'''
                export BUILD_ENV='''+BUILD_ENV+'''
                export BUILD_ARCH='''+BUILD_ARCH+'''
                export BUILD_TARGET='''+BUILD_TARGET+'''
                mkdir -p "${WORKSPACE_TMP}/${BUILD_TARGET}/KingJamesPureBibleSearch/"
                mkdir -p "${WORKSPACE_TMP}/${BUILD_TARGET}/KingJamesPureBibleSearch/db"
                mkdir -p "${WORKSPACE_TMP}/${BUILD_TARGET}/KingJamesPureBibleSearch/doc"
                mkdir -p "${WORKSPACE_TMP}/${BUILD_TARGET}/KingJamesPureBibleSearch/fonts"
                mkdir -p "${WORKSPACE_TMP}/${BUILD_TARGET}/KingJamesPureBibleSearch/geoip"
                mkdir -p "${WORKSPACE_TMP}/${BUILD_TARGET}/KingJamesPureBibleSearch/examples"
                mkdir -p "${WORKSPACE_TMP}/${BUILD_TARGET}/KingJamesPureBibleSearch/license"
                mkdir -p "${WORKSPACE_TMP}/${BUILD_TARGET}/KingJamesPureBibleSearch/translations"
                #
                cd "${WORKSPACE_TMP}/build-KJPBS-${BUILD_TARGET}/Release"
                cp KJVCanOpener/KingJamesPureBibleSearch "${WORKSPACE_TMP}/${BUILD_TARGET}/KingJamesPureBibleSearch/"
                cp KJVDataDump/KJVDataDump "${WORKSPACE_TMP}/${BUILD_TARGET}/KingJamesPureBibleSearch/"
                cp KJVDataParse/KJVDataParse "${WORKSPACE_TMP}/${BUILD_TARGET}/KingJamesPureBibleSearch/"
                cp KJVDictWord/KJVDictWord "${WORKSPACE_TMP}/${BUILD_TARGET}/KingJamesPureBibleSearch/"
                cp KJVDiff/KJVDiff "${WORKSPACE_TMP}/${BUILD_TARGET}/KingJamesPureBibleSearch/"
                cp KJVLookup/KJVLookup "${WORKSPACE_TMP}/${BUILD_TARGET}/KingJamesPureBibleSearch/"
                cp KJVPhraseSearch/KJVPhraseSearch "${WORKSPACE_TMP}/${BUILD_TARGET}/KingJamesPureBibleSearch/"
                cp KJVSearch/KJVSearch "${WORKSPACE_TMP}/${BUILD_TARGET}/KingJamesPureBibleSearch/"
                cp KJVSumThing/KJVSumThing "${WORKSPACE_TMP}/${BUILD_TARGET}/KingJamesPureBibleSearch/"
                #
                cd "${WORKSPACE}/programs/KJVCanOpener/db/"
                cp bbl-*.ccdb "${WORKSPACE_TMP}/${BUILD_TARGET}/KingJamesPureBibleSearch/db/"
                cp dct-*.s3db "${WORKSPACE_TMP}/${BUILD_TARGET}/KingJamesPureBibleSearch/db/"
                #
                cp "${WORKSPACE}/programs/KJVCanOpener/doc/KingJamesPureBibleSearch.pdf" "${WORKSPACE_TMP}/${BUILD_TARGET}/KingJamesPureBibleSearch/doc/"
                cp "${WORKSPACE}/programs/KJVCanOpener/articles/kjv_stats.xls" "${WORKSPACE_TMP}/${BUILD_TARGET}/KingJamesPureBibleSearch/doc/"
                cp "${WORKSPACE}/programs/KJVCanOpener/kjvdatagen/kjv_summary.xls" "${WORKSPACE_TMP}/${BUILD_TARGET}/KingJamesPureBibleSearch/doc/"
                #
                cd "${WORKSPACE}/programs/KJVCanOpener/fonts/"
                cp *.ttf "${WORKSPACE_TMP}/${BUILD_TARGET}/KingJamesPureBibleSearch/fonts/"
                cp *.TTF "${WORKSPACE_TMP}/${BUILD_TARGET}/KingJamesPureBibleSearch/fonts/"
                #
                cp "${WORKSPACE}/programs/KJVCanOpener/geoip/GeoIP.conf" "${WORKSPACE_TMP}/${BUILD_TARGET}/KingJamesPureBibleSearch/geoip/"
                #
                cd "${WORKSPACE}/programs/KJVCanOpener/examples/"
                cp example*.kjs "${WORKSPACE_TMP}/${BUILD_TARGET}/KingJamesPureBibleSearch/examples/"
                #
                cp "${WORKSPACE}/programs/KJVCanOpener/gpl-3.0.txt" "${WORKSPACE_TMP}/${BUILD_TARGET}/KingJamesPureBibleSearch/license/"
                #
                cd "${WORKSPACE_TMP}/build-KJPBS-${BUILD_TARGET}/Release/KJVCanOpener/translations/"
                cp *.qm "${WORKSPACE_TMP}/${BUILD_TARGET}/KingJamesPureBibleSearch/translations/"
                #
                cd "${WORKSPACE_TMP}/build-KJPBS-${BUILD_TARGET}/Release/KJVDataParse/translations/"
                cp *.qm "${WORKSPACE_TMP}/${BUILD_TARGET}/KingJamesPureBibleSearch/translations/"
                #
                cd "${WORKSPACE_TMP}/build-KJPBS_webchannel-${BUILD_TARGET}/Release"
                cp KJVCanOpener/KingJamesPureBibleSearch "${WORKSPACE_TMP}/${BUILD_TARGET}/KingJamesPureBibleSearch/KingJamesPureBibleSearch_webchannel"
                cp -r KJVCanOpener/html "${WORKSPACE_TMP}/${BUILD_TARGET}/KingJamesPureBibleSearch/"
            '''
        }
        stage('Archive_Basic') {
            sh '''
                export QTDIR='''+QTDIR+'''
                export BUILD_ENV='''+BUILD_ENV+'''
                export BUILD_ARCH='''+BUILD_ARCH+'''
                export BUILD_TARGET='''+BUILD_TARGET+'''
                cd "${WORKSPACE_TMP}"
                export VERSION=$(cat VERSION-${BUILD_TARGET}.txt | tr -d '[:space:]')
                cd "${WORKSPACE_TMP}/${BUILD_TARGET}"
                tar -Jcf "KingJamesPureBibleSearch-${VERSION}-${BUILD_ARCH}-jenkins-${BUILD_NUMBER}.tar.xz" KingJamesPureBibleSearch/
                cp *.tar.xz "${WORKSPACE}/"
                rm *.tar.xz
            '''
            archiveArtifacts artifacts: "KingJamesPureBibleSearch-*-${BUILD_ARCH}-jenkins-${BUILD_NUMBER}.tar.xz"
        }
    }

    return stages
}

def commonCleanStages(QTDIR, BUILD_ENV, BUILD_ARCH, BUILD_TARGET) {
    stages = {
        stage('CleanTmp') {
            sh '''
                export QTDIR='''+QTDIR+'''
                export BUILD_ENV='''+BUILD_ENV+'''
                export BUILD_ARCH='''+BUILD_ARCH+'''
                export BUILD_TARGET='''+BUILD_TARGET+'''
                cd "${WORKSPACE_TMP}"
                export VERSION=$(cat VERSION-${BUILD_TARGET}.txt | tr -d '[:space:]')
                cd "${WORKSPACE}"
                rm -f "KingJamesPureBibleSearch-${VERSION}-${BUILD_ARCH}-jenkins-${BUILD_NUMBER}.tar.xz"
                rm -f "KingJamesPureBibleSearch-${VERSION}-${BUILD_ARCH}.AppImage"
                rm -f "KingJamesPureBibleSearch-${VERSION}-${BUILD_ARCH}-AppImage.tar.xz"
                cd "${WORKSPACE_TMP}"
                rm -f VERSION-${BUILD_TARGET}.txt
                rm -rf "build-KJPBS-${BUILD_TARGET}"
                rm -rf "build-KJPBS_webchannel-${BUILD_TARGET}"
                rm -rf "${BUILD_TARGET}/KingJamesPureBibleSearch"
                rmdir "${BUILD_TARGET}"
            '''
        }
    }

    return stages
}

def builders = [:]

builders['ubuntu_18.04_bionic_qt_5.15.2'] = {
    node('docker') {
        docker.image('localhost:5000/dewtronics/ubuntu_18.04_bionic_qt_5.15.2_jenkins_workspace').inside {
            def QTDIR='/home/jenkins/Qt/5.15.2'
            def BUILD_ENV='bionic-Qt_5.15.2'
            def BUILD_ARCH='x86_64'
            def BUILD_TARGET="$BUILD_ENV-$BUILD_ARCH"

            commonBuildStages(QTDIR, BUILD_ENV, BUILD_ARCH, BUILD_TARGET).call()

            stage('AppImage_build') {
                sh '''
                    export QTDIR='''+QTDIR+'''
                    export BUILD_ENV='''+BUILD_ENV+'''
                    export BUILD_ARCH='''+BUILD_ARCH+'''
                    export BUILD_TARGET='''+BUILD_TARGET+'''
                    cd "${WORKSPACE_TMP}"
                    export VERSION=$(cat VERSION-${BUILD_TARGET}.txt | tr -d '[:space:]')   # Note: VERSION is used here by AppImage
                    cd "${WORKSPACE_TMP}/${BUILD_TARGET}/KingJamesPureBibleSearch"
                    # No need to ship WebChannel daemon build with end-user app bundle:
                    rm -rf "html"
                    rm -rf "KingJamesPureBibleSearch_webchannel"
                    # Note: Our custom jenkins_workspace Docker image doesn't have all of
                    #  the dependencies needed for these other SQL drivers and we'll fail
                    #  to build if we include them.  But we don't need them anyway, so
                    #  just remove them:
                    rm -rf /home/jenkins/Qt/5.15.2/plugins/sqldrivers/libqsqlmysql.so
                    rm -rf /home/jenkins/Qt/5.15.2/plugins/sqldrivers/libqsqlodbc.so
                    rm -rf /home/jenkins/Qt/5.15.2/plugins/sqldrivers/libqsqlpsql.so
                    rm -rf /home/jenkins/Qt/5.15.2/plugins/sqldrivers/libqsqltds.so
                    #
                    cp "${WORKSPACE}/programs/KJVCanOpener/KingJamesPureBibleSearch.desktop" ./
                    cp "${WORKSPACE}/programs/KJVCanOpener/res/bible_256.png" KingJamesPureBibleSearch.png
                    linuxdeployqt KingJamesPureBibleSearch.desktop -appimage -no-copy-copyright-files -qmake=${QTDIR}/bin/qmake -executable=KJVDataDump -executable=KJVDataParse -executable=KJVDictWord -executable=KJVDiff -executable=KJVLookup -executable=KJVPhraseSearch -executable=KJVSearch -executable=KJVSumThing
                    cp *.AppImage "${WORKSPACE}/"
                    rm *.AppImage
                '''
            }
            stage('Archive_AppImage') {
                archiveArtifacts artifacts: '*.AppImage'
            }

            commonCleanStages(QTDIR, BUILD_ENV, BUILD_ARCH, BUILD_TARGET).call()
        }
    }
}

builders['raspbian_bullseye_armv7l_qt_5.15.2'] = {
    node('docker && qemu') {
        docker.image('localhost:5000/dewtronics/raspbian_bullseye_armv7l_qt_5.15.2_jenkins_workspace').inside {
            def QTDIR='/usr/lib/arm-linux-gnueabihf'
            def BUILD_ENV='bullseye-Qt_5.15.2'
            def BUILD_ARCH='armv7l'
            def BUILD_TARGET="$BUILD_ENV-$BUILD_ARCH"

            commonBuildStages(QTDIR, BUILD_ENV, BUILD_ARCH, BUILD_TARGET).call()

            stage('AppImage_Staging') {
                sh '''
                    export QTDIR='''+QTDIR+'''
                    export BUILD_ENV='''+BUILD_ENV+'''
                    export BUILD_ARCH='''+BUILD_ARCH+'''
                    export BUILD_TARGET='''+BUILD_TARGET+'''
                    cd "${WORKSPACE_TMP}"
                    export VERSION=$(cat VERSION-${BUILD_TARGET}.txt | tr -d '[:space:]')
                    cd "${WORKSPACE_TMP}/${BUILD_TARGET}/KingJamesPureBibleSearch"
                    # No need to ship WebChannel daemon build with end-user app bundle:
                    rm -rf "html"
                    rm -rf "KingJamesPureBibleSearch_webchannel"
                    #
                    cp "${WORKSPACE}/programs/KJVCanOpener/KingJamesPureBibleSearch.desktop" ./
                    cp "${WORKSPACE}/programs/KJVCanOpener/res/bible_256.png" KingJamesPureBibleSearch.png
                    #
                    cd "${WORKSPACE_TMP}/${BUILD_TARGET}"
                    tar -Jcf "KingJamesPureBibleSearch-${VERSION}-${BUILD_ARCH}-AppImage.tar.xz" KingJamesPureBibleSearch/
                    cp *.tar.xz "${WORKSPACE}/"
                    rm *.tar.xz
                '''
            }
            stage('Archive_AppImage') {
                archiveArtifacts artifacts: "KingJamesPureBibleSearch-*-${BUILD_ARCH}-AppImage.tar.xz"
            }

            commonCleanStages(QTDIR, BUILD_ENV, BUILD_ARCH, BUILD_TARGET).call()
        }
    }
}

builders['ubuntu_18.04_bionic_qt_4.8.7_vnc'] = {
    node('docker') {
        docker.image('localhost:5000/dewtronics/ubuntu_18.04_bionic_qt_5.15.2_jenkins_workspace').inside {
            def QTDIR='/home/jenkins/Qt/4.8.7-vnc-run'
            def BUILD_ENV='bionic-Qt_4.8.7_vnc'
            def BUILD_ARCH='x86_64'
            def BUILD_TARGET="$BUILD_ENV-$BUILD_ARCH"

            stage('Setup') {
                checkout scm
                sh '''
                    export QTDIR='''+QTDIR+'''
                    export BUILD_ENV='''+BUILD_ENV+'''
                    export BUILD_ARCH='''+BUILD_ARCH+'''
                    export BUILD_TARGET='''+BUILD_TARGET+'''
                    cd "${WORKSPACE}"
                    echo "`date +%y%m%d%H%M%S`-g`git describe --long --always --dirty=-dirty`-${BUILD_ENV}" > "${WORKSPACE_TMP}/VERSION-${BUILD_TARGET}.txt"
                    cd "${WORKSPACE_TMP}"
                    export VERSION=$(cat VERSION-${BUILD_TARGET}.txt | tr -d '[:space:]')
                    echo "Version Descriptor: ${VERSION}"
                '''
            }
            stage('GUI_Build') {
                sh '''
                    export QTDIR='''+QTDIR+'''
                    export BUILD_ENV='''+BUILD_ENV+'''
                    export BUILD_ARCH='''+BUILD_ARCH+'''
                    export BUILD_TARGET='''+BUILD_TARGET+'''
                    mkdir -p "${WORKSPACE_TMP}/build-KJPBS-${BUILD_TARGET}/Release"
                    cd "${WORKSPACE_TMP}/build-KJPBS-${BUILD_TARGET}/Release"
                    ${QTDIR}/bin/qmake -spec ${QTDIR}/mkspecs/qws/vnc-linux-x86_64-g++ CONFIG+=release -o Makefile "${WORKSPACE}/programs/KJVCanOpener/KJVCanOpener.pro"
                    make -j 4
                '''
            }
            stage('Deploy') {
                sh '''
                    export QTDIR='''+QTDIR+'''
                    export BUILD_ENV='''+BUILD_ENV+'''
                    export BUILD_ARCH='''+BUILD_ARCH+'''
                    export BUILD_TARGET='''+BUILD_TARGET+'''
                    mkdir -p "${WORKSPACE_TMP}/${BUILD_TARGET}/KingJamesPureBibleSearch/"
                    mkdir -p "${WORKSPACE_TMP}/${BUILD_TARGET}/KingJamesPureBibleSearch/db"
                    mkdir -p "${WORKSPACE_TMP}/${BUILD_TARGET}/KingJamesPureBibleSearch/doc"
                    mkdir -p "${WORKSPACE_TMP}/${BUILD_TARGET}/KingJamesPureBibleSearch/fonts"
                    mkdir -p "${WORKSPACE_TMP}/${BUILD_TARGET}/KingJamesPureBibleSearch/license"
                    mkdir -p "${WORKSPACE_TMP}/${BUILD_TARGET}/KingJamesPureBibleSearch/translations"
                    #
                    cd "${WORKSPACE_TMP}/build-KJPBS-${BUILD_TARGET}/Release"
                    cp KingJamesPureBibleSearch "${WORKSPACE_TMP}/${BUILD_TARGET}/KingJamesPureBibleSearch/"
                    #
                    cd "${WORKSPACE}/programs/KJVCanOpener/db/"
                    cp bbl-*.ccdb "${WORKSPACE_TMP}/${BUILD_TARGET}/KingJamesPureBibleSearch/db/"
                    cp dct-*.s3db "${WORKSPACE_TMP}/${BUILD_TARGET}/KingJamesPureBibleSearch/db/"
                    #
                    cp "${WORKSPACE}/programs/KJVCanOpener/doc/KingJamesPureBibleSearch.pdf" "${WORKSPACE_TMP}/${BUILD_TARGET}/KingJamesPureBibleSearch/doc/"
                    #
                    cd "${WORKSPACE}/programs/KJVCanOpener/fonts/"
                    cp *.ttf "${WORKSPACE_TMP}/${BUILD_TARGET}/KingJamesPureBibleSearch/fonts/"
                    cp *.TTF "${WORKSPACE_TMP}/${BUILD_TARGET}/KingJamesPureBibleSearch/fonts/"
                    #
                    cp "${WORKSPACE}/programs/KJVCanOpener/gpl-3.0.txt" "${WORKSPACE_TMP}/${BUILD_TARGET}/KingJamesPureBibleSearch/license/"
                    #
                    cd "${WORKSPACE}/programs/KJVCanOpener/translations/"
                    cp *.qm "${WORKSPACE_TMP}/${BUILD_TARGET}/KingJamesPureBibleSearch/translations/"
                '''
            }
            stage('Archive_Basic') {
                sh '''
                    export QTDIR='''+QTDIR+'''
                    export BUILD_ENV='''+BUILD_ENV+'''
                    export BUILD_ARCH='''+BUILD_ARCH+'''
                    export BUILD_TARGET='''+BUILD_TARGET+'''
                    cd "${WORKSPACE_TMP}"
                    export VERSION=$(cat VERSION-${BUILD_TARGET}.txt | tr -d '[:space:]')
                    cd "${WORKSPACE_TMP}/${BUILD_TARGET}"
                    tar -Jcf "KingJamesPureBibleSearch-${VERSION}-${BUILD_ARCH}-jenkins-${BUILD_NUMBER}.tar.xz" KingJamesPureBibleSearch/
                    cp *.tar.xz "${WORKSPACE}/"
                    rm *.tar.xz
                '''
                archiveArtifacts artifacts: "KingJamesPureBibleSearch-*-${BUILD_ARCH}-jenkins-${BUILD_NUMBER}.tar.xz"
            }

            commonCleanStages(QTDIR, BUILD_ENV, BUILD_ARCH, BUILD_TARGET).call()
        }
    }
}

stage('KingJamesPureBibleSearch') {
    parallel builders
}

