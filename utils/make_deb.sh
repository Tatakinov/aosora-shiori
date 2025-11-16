#!/bin/bash -x
WORKDIR=work

BIN_DIR="usr/games"
LIB_DIR="usr/lib/games/ninix-kagari/aosora"
DOC_DIR="usr/share/doc/aosora"

VERSION="0.4.2"

make -j

mkdir ${WORKDIR}

pushd ${WORKDIR}

mkdir -p ${BIN_DIR}
mkdir -p ${LIB_DIR}
mkdir -p ${DOC_DIR}

cp ../ninix/aosora-* ${BIN_DIR}/

cp ../ninix/libaosora.so ${LIB_DIR}/libaosora.so

cp -r ../debian DEBIAN

cp ../license.txt ${DOC_DIR}/copyright

find usr -type f -exec md5sum {} \+ > DEBIAN/md5sums
INSTALLED_SIZE=$(du -sk usr | cut -f 1)
sed -i -e "s/@installed_size/${INSTALLED_SIZE}/g" -e "s/@version/${VERSION}/g" DEBIAN/control
popd
fakeroot dpkg-deb --build ${WORKDIR} .

rm -r work
