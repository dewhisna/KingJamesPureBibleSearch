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
apt-get install -y gperf
apt-get install -y binfmt-support bsdmainutils squashfs-tools file
apt-get install -y joe
apt-get install -y sudo
# libgpgme-dev is a dependency of appimage if we want to build/use it:
apt-get install -y libgpgme-dev
#apt-get install -y openssh-server
apt-get install -y ninja-build
apt-get install -y libmodbus-dev
apt-get install -y qtbase5-dev libqt5serialport5-dev libqt5serialbus5-dev qtmultimedia5-dev libqt5websockets5-dev libqt5webchannel5-dev libqt5sql5-sqlite libqt5websockets5-dev qtlocation5-dev qttools5-dev
apt-get install -y libmaxminddb0 libmaxminddb-dev mmdb-bin


# Build CMake from source:
git clone https://github.com/Kitware/CMake.git
cd CMake
git checkout v3.27.9
./bootstrap && make -j 4 && make install
cd ..
rm -rf CMake


# ---------------------
# Setup SSD Config:
# ---------------------
#sed -i /etc/ssh/sshd_config \
#        -e 's/#PermitRootLogin.*/PermitRootLogin no/' \
#        -e 's/#RSAAuthentication.*/RSAAuthentication yes/'  \
#        -e 's/#PasswordAuthentication.*/PasswordAuthentication no/' \
#        -e 's/#SyslogFacility.*/SyslogFacility AUTH/' \
#        -e 's/#LogLevel.*/LogLevel INFO/' && \
#    mkdir /var/run/sshd
#
#cp /tmp/setup-sshd /usr/local/bin/setup-sshd


# ---------------------
# Cleanup Image
# ---------------------
apt-get clean -y
apt-get autoclean -y
apt-get autoremove -y

rm -r /var/lib/apt/lists/*


