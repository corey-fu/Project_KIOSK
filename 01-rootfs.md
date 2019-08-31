## 製作根檔案系統

1. 創建一個資料夾，名為rootfs
2. 為了製作一個基本的rootfs，執行以下的[執行檔](deboot.sh)
```
#!/bin/bash
URL=ftp://120.117.72.71/debian
#URL=http://opensource.nchc.org.tw/debian
VERSION=buster
DIR=rootfs
debootstrap \
--arch=armhf \
--keyring=/usr/share/keyrings/debian-archive-keyring.gpg \
--verbose \
--foreign \
$VERSION \
$DIR \
$URL
```
3. 將本機指令複製至rootfs
```
cp /usr/bin/qemu-arm-static rootfs/usr/bin
```
4. 進入rootfs系統
```
chroot rootfs
```
5. 下載第2階段的套件
```
/debootstrap/debootstrap --second-stage
```
