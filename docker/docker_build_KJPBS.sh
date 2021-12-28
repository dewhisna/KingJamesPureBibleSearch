#!/bin/bash

set -ex

export DEBIAN_FRONTEND=noninteractive

export USER=kjpbs

export BUILD_HOME=/home/$USER
export INSTALL_DIR=/usr/local/bin/

export QT=$BUILD_HOME/Qt
export QT_DIR=$QT/5.15.2

export PROJECT_DIR=$BUILD_HOME/Documents/programs/Bible
export BUILD_TARGET=Qt_5_15_2_gcc_64


# ------------------
# Create KJPBS User and VNC startup users:
# ------------------
useradd -m -U -G adm,dialout,cdrom,sudo,dip,plugdev -s /bin/bash $USER
echo $USER:U6aMy0wojraho | chpasswd -e
#passwd --expire $USER


# ---------------------
# Install Dependencies:
# ---------------------
apt-get update
apt-get install -y software-properties-common apt-utils
#sed -i '/^#\s*deb-src /s/^#\s*//' "/etc/apt/sources.list"
add-apt-repository -y ppa:maxmind/ppa
apt-get update
apt-get install -y build-essential
apt-get install -y pkg-config
apt-get install -y cmake
apt-get install -y gperf bison
apt-get install -y libmaxminddb0 libmaxminddb-dev mmdb-bin geoipupdate
apt-get install -y libgl1-mesa-dev
# Easiest to slurp in Qt5 build dependencies, since we are building Qt5 as a pre-requisite:
# apt-get build-dep -qq --yes libqt5core5a libqt5gui5 libqt5widgets5 libqt5network5 libqt5webchannel5 libqt5sql5 libqt5webengine5
apt-get install -y libqt5core5a libqt5gui5 libqt5widgets5 libqt5network5 libqt5webchannel5 libqt5sql5 libqt5webengine5
# A few helpers:
apt-get install -y perl python
apt-get install -y binfmt-support
apt-get install -y wget
apt-get install -y curl git xz-utils
apt-get install -y nginx
apt-get install -y joe
apt-get install -y sudo

# This is for remote desktop to run KJPBS
apt-get install -y xrdp xfce4
echo xfce4-session >$BUILD_HOME/.xsession
sed -i 's/^\(LogLevel\s*=\s*\).*$/\1WARNING/' /etc/xrdp/xrdp.ini
sed -i 's/^\(SyslogLevel\s*=\s*\).*$/\1WARNING/' /etc/xrdp/xrdp.ini
sed -i 's/^\(EnableSyslog\s*=\s*\).*$/\1false/' /etc/xrdp/xrdp.ini

# Setup Container Startup
(
cat <<'EOF'
#!/bin/bash

su -l kjpbs
EOF
) > /bin/startup
chmod 755 /bin/startup

cd $BUILD_HOME
git clone https://github.com/gdraheim/docker-systemctl-replacement.git
cd docker-systemctl-replacement
git checkout e4ebd56fee93e4867b2adab0df0043ba0f4d5019    # v1.5.4505
mv /bin/systemctl /bin/systemctl.orig
cp files/docker/systemctl.py /bin/systemctl
chmod 755 /bin/systemctl
cd $BUILD_HOME
rm -rf docker-systemctl-replacement


# ---------------------
# Get Git Repository:
# -------------------
cd $BUILD_HOME
mkdir -p $PROJECT_DIR
cd $PROJECT_DIR
git clone --depth 50 https://github.com/dewhisna/KingJamesPureBibleSearch.git
cd KingJamesPureBibleSearch
sed -i 's/git@github.com:/https:\/\/github.com\//' .gitmodules
git submodule update --init --recursive


# ---------------------
# Get prebuilt Qt:
# ---------------------
# From: http://download.qt-project.org/official_releases/qt/5.15/5.15.2/
#   Or: https://download.qt.io/official_releases/qt/5.15/5.15.2/
cd $BUILD_HOME
wget --no-verbose -nc http://vnc.purebiblesearch.com/ebe899bf-1dff-4b04-8e1b-8e63895a963d/Qt_5.15.2_18.04_bionic_gcc_64.tar.xz
echo '8287879f0bd6233a050befb3b3f893cf12a6d0cb  Qt_5.15.2_18.04_bionic_gcc_64.tar.xz' | sha1sum -c
tar -Jxf Qt_5.15.2_18.04_bionic_gcc_64.tar.xz
chown -R $USER:$USER Qt/
# ---------------------
# Link Qt5 Source -> Qt4 Paths for zlib:
#  This has already been applied to the prebuilt binary
#  above.  If building a custom Qt, apply the follow to
#  copy the 3rdparty/zlib* source folders (from the Qt
#  source code) to the Qt binary folder.
# ---------------------
# cd $$[QT_INSTALL_PREFIX]
# mkdir -p src/3rdparty
# cd src/3rdparty/
# cp -r $QT_SRC/qtbase/src/3rdparty/zlib* .
# ---------------------


# ---------------------
# Build QtStylePlugins:
# ---------------------
cd $PROJECT_DIR/KingJamesPureBibleSearch/programs
mkdir -p build-qtstyleplugins-$BUILD_TARGET
cd build-qtstyleplugins-$BUILD_TARGET
$QT_DIR/bin/qmake CONFIG+=static CONFIG+=release CONFIG+=force-debug-info ../qtstyleplugins/qtstyleplugins.pro
make -j 4
make install


# ---------------------
# Build wwWidgets:
# ---------------------
cd $QT
mkdir -p build-wwwidgets4-$BUILD_TARGET/Release
cd build-wwwidgets4-$BUILD_TARGET/Release
$QT_DIR/bin/qmake CONFIG+=release $PROJECT_DIR/KingJamesPureBibleSearch/support/wwWidgets/wwwidgets4.pro
make -j 4
make install
cp $PROJECT_DIR/KingJamesPureBibleSearch/support/wwWidgets/translations/wwwidgets_*.qm $QT_DIR/translations/


# ---------------------
# Create target hierarchy folders:
# ---------------------
mkdir -p $BUILD_HOME/KJVCanOpener/app
mkdir -p $BUILD_HOME/KJVDataParse


# ---------------------
# Configure and Build KJPBS GUI:
# ---------------------
mkdir -p $PROJECT_DIR/KingJamesPureBibleSearch/programs/build-KJVCanOpener-$BUILD_TARGET/Release
cd $PROJECT_DIR/KingJamesPureBibleSearch/programs/build-KJVCanOpener-$BUILD_TARGET/Release
$QT_DIR/bin/qmake CONFIG+=release CONFIG+=nospeech ../../KJVCanOpener/KJVCanOpener.pro
make -j 4
cp KingJamesPureBibleSearch $BUILD_HOME/KJVCanOpener/app/
ln -s $BUILD_HOME/KJVCanOpener/app/KingJamesPureBibleSearch $INSTALL_DIR/KingJamesPureBibleSearch


# ---------------------
# Configure and Build KJPBS WebChannel:
# ---------------------
mkdir -p $PROJECT_DIR/KingJamesPureBibleSearch/programs/build-KJVCanOpener_webchannel-$BUILD_TARGET/Release
cd $PROJECT_DIR/KingJamesPureBibleSearch/programs/build-KJVCanOpener_webchannel-$BUILD_TARGET/Release
$QT_DIR/bin/qmake CONFIG+=release CONFIG+=console CONFIG+=nospeech CONFIG+=webchannel ../../KJVCanOpener/KJVCanOpener.pro
make -j 4
cp KingJamesPureBibleSearch $BUILD_HOME/KJVCanOpener/app/KingJamesPureBibleSearch_webchannel
ln -s $BUILD_HOME/KJVCanOpener/app/KingJamesPureBibleSearch_webchannel $INSTALL_DIR/KingJamesPureBibleSearch_webchannel

(
cat <<'EOF'
#!/bin/bash

KingJamesPureBibleSearch_webchannel -webchannel 9340,0.0.0.0
EOF
) > $INSTALL_DIR/webchannel
chmod 755 $INSTALL_DIR/webchannel

cp -r $PROJECT_DIR/KingJamesPureBibleSearch/programs/build-KJVCanOpener_webchannel-$BUILD_TARGET/Release/html/* /var/www/html/


# ---------------------
# Configure and Build KJVDataParse:
# ---------------------
mkdir -p $PROJECT_DIR/KingJamesPureBibleSearch/programs/build-KJVDataParse-$BUILD_TARGET/Release
cd $PROJECT_DIR/KingJamesPureBibleSearch/programs/build-KJVDataParse-$BUILD_TARGET/Release
$QT_DIR/bin/qmake CONFIG+=release ../../KJVDataParse/KJVDataParse.pro
make -j 4
cp KJVDataParse $BUILD_HOME/KJVCanOpener/app/
ln -s $BUILD_HOME/KJVCanOpener/app/KJVDataParse $INSTALL_DIR/KJVDataParse


# ---------------------
# Configure and Build KJVDictWord:
# ---------------------
mkdir -p $PROJECT_DIR/KingJamesPureBibleSearch/programs/build-KJVDictWord-$BUILD_TARGET/Release
cd $PROJECT_DIR/KingJamesPureBibleSearch/programs/build-KJVDictWord-$BUILD_TARGET/Release
$QT_DIR/bin/qmake CONFIG+=release ../../KJVDictWord/KJVDictWord.pro
make -j 4
cp KJVDictWord $BUILD_HOME/KJVCanOpener/app/
ln -s $BUILD_HOME/KJVCanOpener/app/KJVDictWord $INSTALL_DIR/KJVDictWord


# ---------------------
# Configure and Build KJVDiff:
# ---------------------
mkdir -p $PROJECT_DIR/KingJamesPureBibleSearch/programs/build-KJVDiff-$BUILD_TARGET/Release
cd $PROJECT_DIR/KingJamesPureBibleSearch/programs/build-KJVDiff-$BUILD_TARGET/Release
$QT_DIR/bin/qmake CONFIG+=release ../../KJVDiff/KJVDiff.pro
make -j 4
cp KJVDiff $BUILD_HOME/KJVCanOpener/app/
ln -s $BUILD_HOME/KJVCanOpener/app/KJVDiff $INSTALL_DIR/KJVDiff


# ---------------------
# Configure and Build KJVLookup:
# ---------------------
mkdir -p $PROJECT_DIR/KingJamesPureBibleSearch/programs/build-KJVLookup-$BUILD_TARGET/Release
cd $PROJECT_DIR/KingJamesPureBibleSearch/programs/build-KJVLookup-$BUILD_TARGET/Release
$QT_DIR/bin/qmake CONFIG+=release ../../KJVLookup/KJVLookup.pro
make -j 4
cp KJVLookup $BUILD_HOME/KJVCanOpener/app/
ln -s $BUILD_HOME/KJVCanOpener/app/KJVLookup $INSTALL_DIR/KJVLookup


# ---------------------
# Configure and Build KJVPhraseSearch:
# ---------------------
mkdir -p $PROJECT_DIR/KingJamesPureBibleSearch/programs/build-KJVPhraseSearch-$BUILD_TARGET/Release
cd $PROJECT_DIR/KingJamesPureBibleSearch/programs/build-KJVPhraseSearch-$BUILD_TARGET/Release
$QT_DIR/bin/qmake CONFIG+=release ../../KJVPhraseSearch/KJVPhraseSearch.pro
make -j 4
cp KJVPhraseSearch $BUILD_HOME/KJVCanOpener/app/
ln -s $BUILD_HOME/KJVCanOpener/app/KJVPhraseSearch $INSTALL_DIR/KJVPhraseSearch


# ---------------------
# Configure and Build KJVSearch:
# ---------------------
mkdir -p $PROJECT_DIR/KingJamesPureBibleSearch/programs/build-KJVSearch-$BUILD_TARGET/Release
cd $PROJECT_DIR/KingJamesPureBibleSearch/programs/build-KJVSearch-$BUILD_TARGET/Release
$QT_DIR/bin/qmake CONFIG+=release ../../KJVSearch/KJVSearch.pro
make -j 4
cp KJVSearch $BUILD_HOME/KJVCanOpener/app/
ln -s $BUILD_HOME/KJVCanOpener/app/KJVSearch $INSTALL_DIR/KJVSearch


# ---------------------
# Configure and Build KJVSumThing:
# ---------------------
mkdir -p $PROJECT_DIR/KingJamesPureBibleSearch/programs/build-KJVSumThing-$BUILD_TARGET/Release
cd $PROJECT_DIR/KingJamesPureBibleSearch/programs/build-KJVSumThing-$BUILD_TARGET/Release
$QT_DIR/bin/qmake CONFIG+=release ../../KJVSumThing/KJVSumThing.pro
make -j 4
cp KJVSumThing $BUILD_HOME/KJVCanOpener/app/
ln -s $BUILD_HOME/KJVCanOpener/app/KJVSumThing $INSTALL_DIR/KJVSumThing


# ---------------------
# Install databases and related pieces
# ---------------------
cd $BUILD_HOME/KJVCanOpener/
cp -r $PROJECT_DIR/KingJamesPureBibleSearch/programs/KJVCanOpener/db .
cp -r $PROJECT_DIR/KingJamesPureBibleSearch/programs/KJVCanOpener/doc .
cp -r $PROJECT_DIR/KingJamesPureBibleSearch/programs/KJVCanOpener/fonts .
cp -r $PROJECT_DIR/KingJamesPureBibleSearch/programs/KJVCanOpener/geoip .
cp -r $PROJECT_DIR/KingJamesPureBibleSearch/programs/KJVCanOpener/translations .
ln -s $QT_DIR/plugins $BUILD_HOME/KJVCanOpener/plugins
ln -s $QT_DIR/plugins/platforms $BUILD_HOME/KJVCanOpener/app/platforms

cd $BUILD_HOME/KJVDataParse/
cp -r $PROJECT_DIR/KingJamesPureBibleSearch/programs/KJVDataParse/translations .


# ---------------------
# Setup noVNC skeleton
# ---------------------
cp -r $PROJECT_DIR/KingJamesPureBibleSearch/programs/vnc/noVNC /var/www/html/
ln -s /var/www/html/noVNC/vnc.html /var/www/html/noVNC/index.html


# ---------------------
# Cleanup Permissions
# ---------------------
chown -R $USER:$USER $BUILD_HOME


# ---------------------
# Cleanup Image
# ---------------------
#rm -rf $PROJECT_DIR/KingJamesPureBibleSearch
apt-get clean -y
apt-get autoclean -y
apt-get autoremove -y

# This would save ~150MB, at the expense of breaking apt-get
# until a fresh apt-get update. Omitted for now
#rm -r /var/lib/apt/lists/*


