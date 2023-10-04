# voip_gw

1. Сперва собираем и устанавливаем зависимости: ptlib и opal

1.1. Сборка и установка ptlib:
1.1.1 Устанавливаем зависимости ptlib: 
#apt install pkg-config autoconf libtool autotools-dev libasound2-dev libpcap-dev liblua5.2-dev libncurses-dev libexpat1-dev libjpeg-dev libssl-dev

1.1.2. Клонируем pt (https://sourceforge.net/p/opalvoip/ptlib/ci/v2_20/tree/):
git clone https://git.code.sf.net/p/opalvoip/ptlib opalvoip-ptlib

1.1.3. Переключаемся на ветку релиза 2.20:
$git checkout 8493bb

1.1.4. Внимательно читаем инструкцию по сборке pt: http://wiki.opalvoip.org/index.php?n=Main.BuildingPTLibUnix

1.1.5. Либо собираем по инструкции сами (если получится), либо собираем как я собирал:
$cd ptlib
$aclocal
$autoconf
$./configure --prefix=/usr --with-plugin-installdir=ptlib_plugins --disable-v4l --disable-v4l2 --enable-cpp11 --disable-oss --disable-pthread_kill
$make
1.1.6 Если все собралось, устанавливаем:
#make install

1.2. Сборка и установка opal:
1.2.1 Устанавливаем зависимости opal: 
#apt install pkg-config autoconf libtool autotools-dev libavutil-dev libswresample-dev libavcodec-dev libx264-dev libspeexdsp-dev libopus-dev libvpx-dev libexpat1-dev libgsm1-dev libpcap-dev liblua5.2-dev libncurses-dev libjpeg-dev libssl-dev

1.2.2. Клонируем pt (https://sourceforge.net/p/opalvoip/opal/ci/v3_20/tree/):
git clone https://git.code.sf.net/p/opalvoip/opal opalvoip-opal

1.2.3. Переключаемся на ветку релиза 2.20:
$git checkout 04810b

1.2.4. Внимательно читаем инструкцию по сборке opal: http://wiki.opalvoip.org/index.php?n=Main.BuildingOpalUnix

1.2.5. Либо собираем по инструкции сами (если получится), либо собираем как я собирал:
$cd opal
$aclocal
$autoconf
./configure --with-plugin-installdir=opal_plugins --enable-video --enable-sip --enable-h323 --enable-h224 --enable-h281 --enable-h239 --enable-sipim --enable-plugins --disable-silk --disable-csharp --disable-h2356 --disable-h2358 --enable-x264-licensed --enable-cpp11 --disable-mixer --disable-ivr --disable-openh264 --disable-rtpfec --disable-t120 --disable-aec --disable-gsm --disable-amr --disable-iLBC --disable-isac --disable-lpc10 --disable-ima_adpcm --disable-srtp
$make
1.2.6 Если все собралось, устанавливаем:
#make install

1.3 Сборка и установка voipgw:
1.3.1. Устанавливаем зависимости voipgw:
#apt install cmake

1.3.2. Клонируем voipgw:
$git clone https://github.com/vkurbatov/voip_gw.git

1.3.3. Собираем библиотеку:
$cd voip_gw
$export CC=gcc
$export CXX=gcc
mkdir -p build && cd build && cmake .. && make

1.3.4. Если все собралось, устанавливаем (из директории build):
#make install

1.3.5. Пользуемся

    

