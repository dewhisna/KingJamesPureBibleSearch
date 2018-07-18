#!/bin/bash

set -ex

export DEBIAN_FRONTEND=noninteractive

export BUILD_HOME=/root
export INSTALL_DIR=/usr/local/bin/

export QT=$BUILD_HOME/Qt
export QT_SRC=$QT/5.6.3-src
export QT_BUILD=$QT/5.6.3-build
export QT_DIR=$QT/5.6.3
export KJPBS_BUILD=KJVPhraseSearch

export PROJECT_DIR=$BUILD_HOME/Documents/programs/Bible


# ---------------------
# Install Dependencies:
# ---------------------
apt-get update
apt-get install -y software-properties-common
sed -i '/^#\s*deb-src /s/^#\s*//' "/etc/apt/sources.list"
add-apt-repository -y ppa:maxmind/ppa
apt-get update
apt-get install -y build-essential
apt-get install -y libmaxminddb0 libmaxminddb-dev mmdb-bin geoipupdate
# Downgrade Wayland support so libqt5* build-deps will resolve (since stock 14.04 LTS and 16.04 LTS isn't Wayland):
#apt-get install -y libopenvg1-mesa    # 14.04
apt-get install -y libopenvg1-mesa-lts-utopic    # 16.04
# Easiest to slurp in Qt5 build dependencies, since we are building Qt5 as a pre-requisite:
apt-get build-dep -y libqt5core5a libqt5network5 libqt5widgets5
apt-get install -y gperf bison
# The following should be redundant, as they should be part of the build-dep above, but are essential:
apt-get install -y libxcb1 libxcb1-dev libx11-xcb1 libx11-xcb-dev libxcb-keysyms1 libxcb-keysyms1-dev libxcb-image0 libxcb-image0-dev libxcb-shm0 libxcb-shm0-dev libxcb-icccm4 libxcb-icccm4-dev libxcb-sync-dev libxcb-xfixes0-dev libxrender-dev libxcb-shape0-dev
apt-get install -y libfreetype6-dev fontconfig
apt-get install -y libjpeg-dev
apt-get install -y zlib1g zlib1g-dev
# A few helpers:
apt-get install -y perl python
apt-get install -y binfmt-support
apt-get install -y wget
apt-get install -y curl git xz-utils
apt-get install -y nginx


# ---------------------
# Get Git Repository:
# -------------------
cd $BUILD_HOME
mkdir -p $PROJECT_DIR
cd $PROJECT_DIR
git clone https://github.com/dewhisna/KingJamesPureBibleSearch.git
cd KingJamesPureBibleSearch
sed -i 's/git@github.com:/https:\/\/github.com\//' .gitmodules
git submodule update --init --recursive


# ---------------------
# Get Qt:
# ---------------------
mkdir -p $QT
cd $QT
wget --quiet https://download.qt.io/official_releases/qt/5.6/5.6.3/single/qt-everywhere-opensource-src-5.6.3.tar.xz
tar -xf qt-everywhere-opensource-src-5.6.3.tar.xz
mv qt-everywhere-opensource-src-5.6.3 $QT_SRC


# ---------------------
# Build Qt for building KJPBS console mode:
# ---------------------
mkdir -p $QT_BUILD
cd $QT_BUILD
# Note: Qt 5.5.1 (unlike 5.5.0 and before) is setup for Wayland and so we can't use -libinput
# $QT_SRC/configure -v -release -qt-zlib -qt-libpng -qt-libjpeg -qt-sql-sqlite -qt-freetype -qt-pcre -qt-xcb -qt-xkbcommon-x11 -libinput -fontconfig -directfb -linuxfb -nomake examples -nomake tests -opensource -confirm-license -prefix "$QT_DIR"
$QT_SRC/configure -v -release -qt-zlib -qt-libpng -qt-libjpeg -qt-sql-sqlite -qt-freetype -qt-pcre -qt-xcb -qt-xkbcommon-x11 -fontconfig -directfb -linuxfb -nomake examples -nomake tests -opensource -confirm-license -prefix "$QT_DIR"
make -j 4
make install


# ---------------------
# Link Qt5 Source -> Qt4 Paths used in KJPBS:
# ---------------------
cd $QT_DIR
mkdir -p src/3rdparty
cd src/3rdparty/
cp -r $QT_SRC/qtbase/src/3rdparty/zlib* .
cd $QT_DIR
mkdir -p src/gui/mac/qt_menu.nib
cd src/gui/mac/qt_menu.nib
cp -r $QT_SRC/qtbase/src/plugins/platforms/cocoa/qt_menu.nib/* .


# ---------------------
# Build QtStylePlugins:
# ---------------------
cd $PROJECT_DIR/KingJamesPureBibleSearch/programs
mkdir build-qtstyleplugins_gcc_64
cd build-qtstyleplugins_gcc_64
$QT_DIR/bin/qmake CONFIG+=static CONFIG+=release CONFIG+=force-debug-info ../qtstyleplugins/qtstyleplugins.pro
make -j 4
make install


# ---------------------
# Build wwWidgets:
# ---------------------
cd $QT_DIR/..
mkdir -p build-wwwidgets4_gcc_64/Release
cd build-wwwidgets4_gcc_64/Release
$QT_DIR/bin/qmake CONFIG+=release $PROJECT_DIR/KingJamesPureBibleSearch/support/wwWidgets/wwwidgets4.pro
make -j 4
make install
cp $PROJECT_DIR/KingJamesPureBibleSearch/support/wwWidgets/translations/wwwidgets_*.qm $QT_DIR/translations/


# ---------------------
# Create target hierarchy folder:
# ---------------------
mkdir -p $BUILD_HOME/KJVCanOpener/app


# ---------------------
# Configure and Build KJPBS webchannel:
# ---------------------
cd $PROJECT_DIR/KingJamesPureBibleSearch/programs/
mkdir -p build-KJVCanOpener_gcc_64_webchannel/Release
cd build-KJVCanOpener_gcc_64_webchannel/Release
$QT_DIR/bin/qmake CONFIG+=release CONFIG+=console CONFIG+=nospeech CONFIG+=webchannel ../../KJVCanOpener/KJVCanOpener.pro
make -j 4
cp KingJamesPureBibleSearch $BUILD_HOME/KJVCanOpener/app/
ln -s $BUILD_HOME/KJVCanOpener/app/KingJamesPureBibleSearch $INSTALL_DIR/KingJamesPureBibleSearch

(
cat <<'EOF'
#!/bin/bash

KingJamesPureBibleSearch -webchannel 9340,0.0.0.0
EOF
) > $INSTALL_DIR/webchannel
chmod 755 $INSTALL_DIR/webchannel

cp -r $PROJECT_DIR/KingJamesPureBibleSearch/programs/build-KJVCanOpener_gcc_64_webchannel/Release/html/* /var/www/html/

# ---------------------
# Configure and Build KJVPhraseSearch:
# ---------------------
cd $PROJECT_DIR/KingJamesPureBibleSearch/programs/
mkdir -p build-KJVPhraseSearch/Release
cd build-KJVPhraseSearch/Release
$QT_DIR/bin/qmake CONFIG+=release ../../KJVPhraseSearch/KJVPhraseSearch.pro
make -j 4
cp KJVPhraseSearch $BUILD_HOME/KJVCanOpener/app/
ln -s $BUILD_HOME/KJVCanOpener/app/KJVPhraseSearch $INSTALL_DIR/KJVPhraseSearch


# ---------------------
# Configure and Build KJVSearch:
# ---------------------
cd $PROJECT_DIR/KingJamesPureBibleSearch/programs/
mkdir -p build-KJVSearch/Release
cd build-KJVSearch/Release
$QT_DIR/bin/qmake CONFIG+=release ../../KJVSearch/KJVSearch.pro
make -j 4
cp KJVSearch $BUILD_HOME/KJVCanOpener/app/
ln -s $BUILD_HOME/KJVCanOpener/app/KJVSearch $INSTALL_DIR/KJVSearch


# ---------------------
# Configure and Build KJVLookup:
# ---------------------
cd $PROJECT_DIR/KingJamesPureBibleSearch/programs/
mkdir -p build-KJVLookup/Release
cd build-KJVLookup/Release
$QT_DIR/bin/qmake CONFIG+=release ../../KJVLookup/KJVLookup.pro
make -j 4
cp KJVLookup $BUILD_HOME/KJVCanOpener/app/
ln -s $BUILD_HOME/KJVCanOpener/app/KJVLookup $INSTALL_DIR/KJVLookup


# ---------------------
# Install database and related pieces
# ---------------------
cd $BUILD_HOME/KJVCanOpener/
cp -r $PROJECT_DIR/KingJamesPureBibleSearch/programs/KJVCanOpener/db .
cp -r $PROJECT_DIR/KingJamesPureBibleSearch/programs/KJVCanOpener/doc .
cp -r $PROJECT_DIR/KingJamesPureBibleSearch/programs/KJVCanOpener/fonts .
cp -r $PROJECT_DIR/KingJamesPureBibleSearch/programs/KJVCanOpener/geoip .
cp -r $PROJECT_DIR/KingJamesPureBibleSearch/programs/KJVCanOpener/translations .



