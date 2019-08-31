#!/bin/bash

URL=ftp://120.117.72.71/debian
#URL=http://opensource.nchc.org.tw/debian

VERSION=buster
DIR=rootfs-origin

debootstrap \
	--arch=armhf \
	--keyring=/usr/share/keyrings/debian-archive-keyring.gpg \
	--verbose \
	--foreign \
	$VERSION \
	$DIR \
	$URL
