# voip_gw

# Сперва собираем и устанавливаем зависимости: ptlib и opal

## Сборка и установка ptlib:

Устанавливаем зависимости ptlib:
```
#apt install pkg-config autoconf libtool autotools-dev libasound2-dev libpcap-dev liblua5.2-dev libncurses-dev libexpat1-dev libjpeg-dev libssl-dev
```
Клонируем pt (https://sourceforge.net/p/opalvoip/ptlib/ci/v2_20/tree/):
```
git clone https://git.code.sf.net/p/opalvoip/ptlib opalvoip-ptlib
```
Переключаемся на ветку релиза 2.20:
```
$git checkout 8493bb
```
Внимательно читаем инструкцию по сборке pt: http://wiki.opalvoip.org/index.php?n=Main.BuildingPTLibUnix

Либо собираем по инструкции сами (если получится), либо собираем как я собирал:
```
$cd ptlib
$aclocal
$autoconf
$./configure --prefix=/usr --with-plugin-installdir=ptlib_plugins --disable-v4l --disable-v4l2 --enable-cpp11 --disable-oss --disable-pthread_kill
$make
```

Если все собралось, устанавливаем:
```
#make install
```
## Сборка и установка opal:

Устанавливаем зависимости opal: 

#apt install pkg-config autoconf libtool autotools-dev libavutil-dev libswresample-dev libavcodec-dev libx264-dev libspeexdsp-dev libopus-dev libvpx-dev libexpat1-dev libgsm1-dev libpcap-dev liblua5.2-dev libncurses-dev libjpeg-dev libssl-dev

Клонируем pt (https://sourceforge.net/p/opalvoip/opal/ci/v3_20/tree/):

git clone https://git.code.sf.net/p/opalvoip/opal opalvoip-opal

Переключаемся на ветку релиза 2.20:

$git checkout 04810b

Внимательно читаем инструкцию по сборке opal: http://wiki.opalvoip.org/index.php?n=Main.BuildingOpalUnix

Либо собираем по инструкции сами (если получится), либо собираем как я собирал:
```
$cd opal
$aclocal
$autoconf
./configure --with-plugin-installdir=opal_plugins --enable-video --enable-sip --enable-h323 --enable-h224 --enable-h281 --enable-h239 --enable-sipim --enable-plugins --disable-silk --disable-csharp --disable-h2356 --disable-h2358 --enable-x264-licensed --enable-cpp11 --disable-mixer --disable-ivr --disable-openh264 --disable-rtpfec --disable-t120 --disable-aec --disable-gsm --disable-amr --disable-iLBC --disable-isac --disable-lpc10 --disable-ima_adpcm --disable-srtp
$make
```
Если все собралось, устанавливаем:
```
#make install
```

# Сборка и установка voipgw:
Устанавливаем зависимости voipgw:
```
#apt install cmake
```
Клонируем voipgw:
```
$git clone https://github.com/vkurbatov/voip_gw.git
```
Собираем библиотеку:
```
$cd voip_gw
$export CC=gcc
$export CXX=gcc
mkdir -p build && cd build && cmake .. && make
```
Если все собралось, устанавливаем (из директории build):
```
#make install
```
Пользуемся

    

