#!/bin/bash

set -ex

export DEBIAN_FRONTEND=noninteractive

user=jenkins
group=jenkins
uid=1000
gid=1000

export AGENT_HOME=/home/$user


# --------------------
# Create jenkins User:
# --------------------
groupadd -g ${gid} ${group} \
    && useradd -d "${AGENT_HOME}" -u "${uid}" -g "${gid}" -m -s /bin/bash "${user}" \
    && mkdir -p "${AGENT_HOME}/.ssh/" \
    && chown -R "${uid}":"${gid}" "${AGENT_HOME}" "${AGENT_HOME}/.ssh/"


# ---------------------
# Install Dependencies:
# ---------------------
apt-get update
apt-get upgrade -y --with-new-pkgs
apt-get install -y software-properties-common apt-utils
apt-get install -y ca-certificates 
#sed -i '/^#\s*deb-src /s/^#\s*//' "/etc/apt/sources.list"
apt-get install -y build-essential
apt-get install -y pkg-config
apt-get install -y gperf bison
apt-get install -y zlib1g zlib1g-dev
apt-get install -y libgl1-mesa-dev
add-apt-repository -y ppa:maxmind/ppa
apt-get update
apt-get install -y libmaxminddb0 libmaxminddb-dev mmdb-bin geoipupdate
# Easiest to slurp in Qt5 build dependencies, since we are building Qt5 as a pre-requisite:
# apt-get build-dep -qq --yes libqt5core5a libqt5gui5 libqt5widgets5 libqt5network5 libqt5webchannel5 libqt5sql5 libqt5webengine5
apt-get install -y libqt5core5a libqt5gui5 libqt5widgets5 libqt5network5 libqt5webchannel5 libqt5sql5 libqt5webengine5
# A few helpers:
apt-get install -y perl python
apt-get install -y binfmt-support
apt-get install -y wget
apt-get install -y curl git git-lfs xz-utils
apt-get install -y joe
apt-get install -y sudo
apt-get install -y openssh-server
# Need a newer cmake than the system package, so just grab the latest:
wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null
echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ $(lsb_release -cs) main" | tee /etc/apt/sources.list.d/kitware.list >/dev/null
apt-get update
apt-get install -y cmake


# ---------------------
# Setup SSD Config:
# ---------------------
sed -i /etc/ssh/sshd_config \
        -e 's/#PermitRootLogin.*/PermitRootLogin no/' \
        -e 's/#RSAAuthentication.*/RSAAuthentication yes/'  \
        -e 's/#PasswordAuthentication.*/PasswordAuthentication no/' \
        -e 's/#SyslogFacility.*/SyslogFacility AUTH/' \
        -e 's/#LogLevel.*/LogLevel INFO/' && \
    mkdir /var/run/sshd

cp /tmp/setup-sshd /usr/local/bin/setup-sshd


# ---------------------
# Get prebuilt Qt:
# ---------------------
# From: http://download.qt-project.org/official_releases/qt/5.15/5.15.2/
#   Or: https://download.qt.io/official_releases/qt/5.15/5.15.2/
cd $AGENT_HOME
wget --no-verbose -nc http://vnc.purebiblesearch.com/ebe899bf-1dff-4b04-8e1b-8e63895a963d/Qt_5.15.2_18.04_bionic_gcc_64.tar.xz
echo '8287879f0bd6233a050befb3b3f893cf12a6d0cb  Qt_5.15.2_18.04_bionic_gcc_64.tar.xz' | sha1sum -c
tar -Jxf Qt_5.15.2_18.04_bionic_gcc_64.tar.xz
chown -R $user:$user Qt/
rm Qt_5.15.2_18.04_bionic_gcc_64.tar.xz
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
# Cleanup Image
# ---------------------
apt-get clean -y
apt-get autoclean -y
apt-get autoremove -y

rm -r /var/lib/apt/lists/*


