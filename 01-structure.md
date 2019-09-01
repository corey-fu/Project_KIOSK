## 此為整體基本架構，請依自行需求更改參數，僅供參考用

*   [行前準備](#on-going)
    *   [根檔案系統](#rootfs)
    *   [伺服器](#server)
    *   [客戶端](#client)

<h2 id="on-going">行前準備</h2>

- 請先燒錄官方映像檔至SD卡並開機一次
- 第1分割區不變
- 第2分割區格式化成ext4檔案格式
- 整體概覽可參考[心智圖](https://drive.google.com/open?id=1nERW7qR-LH6WHuvIVXvl9z2GfrjVFB5R) 

<h2 id="rootfs">製作根檔案系統</h2>

創建一個資料夾，名為**rootfs**

```
# mkdir rootfs
```
為了製作一個基本的rootfs，執行以下的[腳本](https://github.com/corey-fu/Project_KIOSK/blob/master/deboot.sh)

```bash
# sh deboot.sh
```

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

將本機指令複製至rootfs

```
# cp /usr/bin/qemu-arm-static rootfs/usr/bin
```

進入rootfs系統

```
# chroot rootfs
```
下載第2階段的套件

```
# /debootstrap/debootstrap --second-stage
```
一個基本的根檔案系統完成了，接著我們可以進行客製化囉！

### 使用者資訊

切換至根檔案系統

```
# chroot rootfs
```
更改 root 帳號/密碼

```
# echo root:YourPassword | chpasswd
```

創建 user 帳號/密碼 (以我自己為例)

```
# useradd -m -d /home/coreypi -s /bin/bash coreypi
# echo coreypi:YourPassword | chpasswd
```

###  系統設定

更改fstab 

```
# vim /etc/fstab
```

```
# <file system> <mount point>   <type>  <options>       <dump>  <pass> 
proc        /proc        proc    defaults        0       0 
/dev/mmcblk0p2        /        	ext4     errors=remount-ro        0       2 
/dev/mmcblk0p1        /boot       vfat     defaults      0       1 
```


替換lib目錄  

> 將原本raspbian的lib更換至rootfs的lib


新增udev-rule檔案(70-persistent-net.rules)

- 目的：rename to eth0 
- 路徑：/lib/udev/rules.d/  
- 請自行更改MAC位址

```
# vim /lib/udev/rules.d/70-persistent-net.rules
```

``` 
SUBSYSTEM=="net", ACTION=="add", DRIVERS=="?*", \ 
  ATTR{address}==" b8-27-eb-d8-3e-18",           \ 
  ATTR{type}=="1", NAME="eth0" 
```

<h2 id="server">伺服器</h2>

#### 在這裡會以無線路由器的方式去做設定

### 網路設定

網路介面設定(via interfaces)
```
# vim /etc/network/interfaces
```
```
# This file describes the network interfaces available on your system
# and how to activate them. For more information, see interfaces(5).

source /etc/network/interfaces.d/*

# The loopback network interface
auto lo
iface lo inet loopback

# The primary network interface

auto eth0 
allow-hotplug eth0
iface eth0 inet dhcp

# DO NOT auto start wlan0 !
# auto wlan0
allow-hotplug wlan0
iface wlan0 inet static
	address 192.168.80.1/24
	netmask 255.255.255.0
```


設定校內DNS解析 

```
# vim /etc/resolv.conf
```
```
domain stust.edu.tw 
search stust.edu.tw 
nameserver 120.117.2.2 
nameserver 120.117.2.1 
nameserver 120.117.74.28
```

設定主機與IP對應的文件

```
# vim /etc/hosts
```
```
127.0.0.1       localhost 
127.0.0.1       Kiosk_Server
192.168.80.1    Kiosk_Server
```

更改鏡像站(via sources.list)

```
# vim /etc/apt/sources.list
```

```
deb http://opensource.nchc.org.tw/debian/ buster main contrib non-free 
deb-src http://opensource.nchc.org.tw/debian/ buster main contrib  non-free
```

Update & Upgrade 
```
# apt update && apt upgrade
```

安裝所需套件及關閉服務
```
	apt install vim ssh dnsmasq hostapd
	systemctl stop dnsmasq
	systemctl stop hostapd
```

設定DHCP Server

```
# vim /etc/dnsmasq.conf
```
```
interface=wlan0 # Use the wlan interface
#listen-address=127.0.0.1 
listen-address=192.168.80.1
bind-interfaces
#bind-dynamic server=8.8.8.8
#server=/google.com/172.217.27.142
#domain-needed
#bogus-priv 
dhcp-range=192.168.80.100,192.168.80.150,12h
#dhcp-option=3,192.168.80.1 
```

設定無線熱點

```
# vim /etc/hostapd/hostapd.conf
```
```
interface=wlan0
#bridge=br0 
driver=nl80211 
ssid=AP-SSID
hw_mode=g 
channel=7 
wmm_enabled=1 
macaddr_acl=0 
auth_algs=1 
ignore_broadcast_ssid=0 
wpa=2
wpa_passphrase=YourPassword
wpa_key_mgmt=WPA-PSK 
wpa_pairwise=TKIP
rsn_pairwise=CCMP
```

設定NAT(網路位址轉換)

```
# iptables -t nat -A POSTROUTING -o eth0 -j MASQUERADE
# iptables -A FORWARD -i eth0 -o wlan0 -m state --state RELATED,ESTABLISHED -j ACCEPT
# iptables -A FORWARD -i wlan0 -o eth0 -j ACCEPT 
```
```
# sh -c "iptables-save > /etc/iptables.ipv4.nat" 
```
在rc.local加入這行以便在開機時生效
```
# vim /etc/rc.local
```
> iptables-restore < /etc/iptables.ipv4.nat
```
#!/bin/sh -e 
# 
# rc.local 
# 
# This script is executed at the end of each multiuser runlevel. 
# Make sure that the script will "exit 0" on success or any other 
# value on error. 
# 
# In order to enable or disable this script just change the execution 
# bits. 
# 
# By default this script does nothing. 

iptables-restore < /etc/iptables.ipv4.nat 

exit 0 
```

設定ipv4轉發

```
# vim /etc/sysctl.conf
```
將這行取消註解
> net.ipv4.ip_forward=1

如果想立刻執行的話
```bash
# sh -c "echo 1 > /proc/sys/net/ipv4/ip_forward" 
```

<h2 id="client">客戶端</h2>

### 系統設定

#### 安裝所需套件

```
# apt install iw wpasupplicant wireless-tools net-tools
```

#### 設定主機名稱

> Kiosk_Client1

### 網路設定

#### 網路介面設定(via interfaces)

```
auto lo 
iface lo inet loopback 

auto eth0 
allow-hotplug eth0
iface eth0 inet dhcp 

auto wlan0
allow-hotplug wlan0
iface wlan0 inet dhcp 
         post-up route add default gw 192.168.80.1 
         post-up iw wlan0 set power_save off
         wpa-ssid P401_PM_LAB(Kiosk) 
         wpa-psk KioskAuthorized

```

#### 設定主機與IP對應的文件

```
# vim /etc/hosts
```

```
127.0.0.1    localhost 
127.0.0.1    Kiosk_Client_1
```

#### 更改鏡像站

```
deb http://opensource.nchc.org.tw/debian/ buster main contrib non-free 
deb-src http://opensource.nchc.org.tw/debian/ buster main contrib non-free  
```

#### Update & Upgrade 

```
# apt update && apt upgrade 
```





