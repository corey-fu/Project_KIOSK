# 基於嵌入式系統之互動式廣告機


## 前言

此為我的碩論筆記，請隨意參考

*   [行前準備](#on-going)
    *   [根檔案系統](#rootfs)
    *   [伺服器之基本設定](#server)
    *   [客戶端之基本設定](#client)

<h2 id="on-going">行前準備</h2>

- 請先燒錄官方映像檔至SD卡並開機一次
- 第1分割區不變
- 第2分割區格式化成ext4檔案格式
- 接著設定細項請參考[心智圖](https://drive.google.com/open?id=1nERW7qR-LH6WHuvIVXvl9z2GfrjVFB5R) 

<h2 id="on-going">製作根檔案系統</h2>

1. 創建一個資料夾，名為rootfs
```
mkdir rootfs
```
2. 為了製作一個基本的rootfs，執行以下的[執行檔](https://github.com/corey-fu/Project_KIOSK/blob/master/deboot.sh)
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
6. 一個基本的根檔案系統完成了，接著我們可以進行客製化囉！
