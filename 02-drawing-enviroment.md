## 此環境建置於客戶端上，請依自行需求更改參數，僅供參考用

*   [繪圖環境建置](#on-going)
    *   [下載並連結函式庫](#lib)
    *   [安裝播放器](#player)
    *   [除錯](#troubleshooting)


<h2 id="on-going">繪圖環境建置</h2>

為了在樹莓派客戶端上以最小的資源播放影片

我們需要：
- 樹莓派的官方函式庫
- 安裝足以在Terminal上執行的播放器，而非在X11上
- 協助一般使用者加上影音群組

一開始先協助一般使用者加上副群組(以自己為例)：
```
# usermod -a -G audio,video coreypi
```

接著就讓我們開始設定吧！


<h2 id="lib">下載並連結函式庫</h2>

### 下載函式庫

在這裡特別說明一下 

由於筆者使用的是rpi 3 B+ (kernel:4.19.57+)

現在官方的函式庫已經為rpi 4更新至4.19.69 所以我們必須使用[舊版](https://github.com/corey-fu/Project_KIOSK/blob/master/vc)

把資料夾下載到rootfs/opt，並新增與函式庫的連結

### 新增設定檔

```
# vim /etc/ld.so.conf.d/00-vmcs.conf
```

### 更新連結

```
# ldconfig
```

<h2 id="player">安裝播放器</h2>

在此case中我們需要的是可在Terminal上播放的播放器，所以我們選擇omxplayer


### 安裝所需套件

```
# apt install libasound2 libavcodec58 libavformat58 libavutil56 libfreetype6 libswresample3 libpcre3 fonts-freefont-ttf fbset 
# wget https://archive.raspberrypi.org/debian/pool/main/o/omxplayer/omxplayer_20190723+gitf543a0d-1_armhf.deb
# dpkg -i omxplayer_20190723+gitf543a0d-1_armhf.deb
```

### 試運行

```
$ omxplayer VIDEO.mp4
```

<h2 id="troubleshooting">除錯</h2>

**Q. 出現"error while loading shared libraries"的錯誤訊息**

A. 請檢查函式庫是否被正確連結到

```
# ldconfig -v
```

**Q. 一般使用者無法播放影片**

A. 多半是沒有被加上副群組(audio,video)

```
# usermod -a -G audio,video coreypi
```


