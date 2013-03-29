#!/bin/sh

# Generic installer for LSB developer kits.
# Copyright 2006, 2007 The Linux Foundation.
# Written by Jeff Licquia <licquia@freestandards.org>

# Modified to check for LSB to be installed by checking for the
# presence of lsb_release so this script can be used to deploy
# LSB applications (in addition to the developer kit it was originally
# designed for).
# Modifications Copyright 2013, Donna Whisnant, a.k.a. Dewtronics

# This script automates some of the steps in getting a LSB environment
# installed.  It's intended to be shipped within a tarball as the
# install procedure for the pieces within the tarball.  Basically,
# this script reads a configuration file containing the list of
# packages to install, looks for those packages in the current
# directory, determines how to install them, and installs them.
# 
# The tricky part is figuring out how to install LSB packages.
# Basically, this script uses the following algorithm:
# 
# If the system has /etc/debian_version and the dpkg and alien
# binaries in the PATH, it's Debian (or a derivative), and we should
# use alien.  Otherwise, if the system has rpm in the PATH, then it's
# a RPM-based distribution, and we should just use rpm.  Otherwise,
# freak out, because we don't know how to install LSB packages.

# First, source our configuration.

if [ \! -e ./inst-config ]; then
    echo "error: could not find configuration" >&2
    exit 1
fi

. ./inst-config

# Sanity-check the configuration.

for pkg in ${PACKAGES}; do
    if [ \! -e ${pkg} ]; then
	echo "error: package ${pkg} missing" >&2
	exit 1
    fi
done


# Make sure we have LSB installed:
which lsb_release 2>&1 > /dev/null
if [ $? -ne 0 ]; then
    cat >&2 <<EOF

This software requires a Linux Distribution that is compliant with
Linux Standard Base (LSB).  The "lsb_release" information appears
to be missing.  You should install the "lsb" package, and try
running this script again.  (Note that on most Fedora and Red Hat
systems, the package is called "redhat-lsb")

You can proceed with the installation, but this software will likely
fail to run until you've properly installed "lsb".

EOF
    echo -n "Continue to install anyway? (y/n): "
    read answer
    answer=`echo ${answer} | cut -c1`
    if [ "${answer}" '!=' "y" -a "${answer}" '!=' "Y" ]; then
        exit 3
    fi
    echo
fi


# Now figure out what kind of system we're running.

if [ -f /etc/debian_version ]; then
    # Debian
    cat <<EOF

This system appears to be a variant of Debian GNU/Linux, such as
Debian itself, Ubuntu, MEPIS, Xandros, Linspire, etc.

EOF
    RPM_INSTALL_CMD="alien -ick"
    which alien 2>&1 > /dev/null
    if [ $? -ne 0 ]; then
	cat >&2 <<EOF
However, the alien package (which is necessary to install LSB packages
on Debian-based systems) is not currently installed.  This is likely a
more serious problem, in that alien is required for your system to be
LSB-compliant.  You should install the "lsb" package, and try running
this script again.
EOF
	exit 3
    fi
else
    which rpm 2>&1 > /dev/null
    if [ $? -eq 0 ]; then
	# RPM-based distro
	cat <<EOF

This system appears to be a RPM-based distribution, such as those from
Red Hat, SuSE/Novell, Mandriva, Asianux, etc.

EOF
	RPM_INSTALL_CMD="rpm --quiet --nodeps --replacepkgs -i"
    else
	# Something else
	cat >&2 <<EOF

I cannot determine how to install LSB packages on your system.  You
may have luck unpacking the packages in this directory manually, using
tools such as rpm2cpio, but there is no guarantee that the packages
will work for their intended purpose.  If you think your system should
be able to install LSB packages, please contact the Free Standards
Group.
EOF
	exit 3
    fi
fi

# Now that we've printed what we think, check to make sure we're
# right.

echo -n "Is this correct? (y/n): "
read answer
answer=`echo ${answer} | cut -c1`
if [ "${answer}" '!=' "y" -a "${answer}" '!=' "Y" ]; then
    exit 4
fi
echo

# Figure out how to get root if we need to.

if [ `id -u` -ne 0 ]; then
    cat <<EOF
In order to install these packages, you need administrator
privileges.  You are currently running this script as an unprivileged
user.

EOF

    SU_CMD="su root -c"
    which sudo 2>&1 > /dev/null
    if [ $? -eq 0 ]; then
	echo -n "You have sudo available.  Should I use it? (y/n): "
	read answer
	answer=`echo ${answer} | cut -c1`
	if [ "${answer}" = "y" -o "${answer}" = "Y" ]; then
	    SU_CMD="sudo /bin/sh -c"
	fi
	echo
    fi

    cat <<EOF
Using the command "${SU_CMD}" to gain root access.  Please type the
appropriate password if prompted.

EOF
else
    SU_CMD="/bin/sh -c"
fi

# Install the LSB packages.

echo "Installing packages..."
rm -f error-log.txt >/dev/null 2>&1
touch error-log.txt >/dev/null 2>&1
${SU_CMD} "${RPM_INSTALL_CMD} ${PACKAGES} 2>error-log.txt"

INSTALL_STATUS=$?
if [ -s error-log.txt ]; then
    cat <<EOF
There may have been problems with the package installation.  The
following errors were reported by the system: 

EOF
    cat error-log.txt
elif [ $INSTALL_STATUS -ne 0 ]; then
    cat <<EOF

The command "${SU_CMD} ..." failed.

EOF
else
    if [ -x ./post-install.sh ]; then
	./post-install.sh
    fi
fi

