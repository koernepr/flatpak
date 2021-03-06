#!/usr/bin/bash
# Install build dependencies, run unit tests and installed tests.

set -xeuo pipefail

dn=$(dirname $0)
. ${dn}/libbuild.sh

pkg_install_builddeps flatpak
pkg_install sudo which attr fuse \
    libubsan libasan libtsan \
    elfutils ostree \
    /usr/bin/{update-mime-database,update-desktop-database,gtk-update-icon-cache}
pkg_install_if_os fedora gjs parallel clang

# Temporarily build ostree from git master for https://github.com/flatpak/flatpak/pull/848
(git clone --depth=1 https://github.com/ostreedev/ostree/
 cd ostree
 pkg_install_builddeps ostree
 unset CFLAGS # the sanitizers require calling apps be linked too
 build
 make
 make install
 ostree --version
)


build --enable-gtk-doc ${CONFIGOPTS:-}
