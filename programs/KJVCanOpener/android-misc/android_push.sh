#!/bin/bash

if test $# -lt 1; then
    echo $"Usage:    $0 <device>"
    echo $""
    exit 1
fi

if test "$ANDROID_SDK_ROOT" = ""; then
    echo $"Must set ANDROID_SDK_ROOT first!"
    echo $""
    exit 1
fi

echo "Pushing King James Pure Bible Search Support Files to Android Device: $1..."
pushd `dirname $0` > /dev/null
SCRIPTPATH=`pwd -P`
popd > /dev/null

$ANDROID_SDK_ROOT/platform-tools/adb -s $1 push $SCRIPTPATH/../db/kjvtext.s3db /data/local/tmp/com.dewtronics.KingJamesPureBibleSearch/db/kjvtext.s3db

$ANDROID_SDK_ROOT/platform-tools/adb -s $1 push $SCRIPTPATH/../db/kjvuser.s3db /data/local/tmp/com.dewtronics.KingJamesPureBibleSearch/db/kjvuser.s3db

$ANDROID_SDK_ROOT/platform-tools/adb -s $1 push $SCRIPTPATH/../db/dct-web1828.s3db /data/local/tmp/com.dewtronics.KingJamesPureBibleSearch/db/dct-web1828.s3db

$ANDROID_SDK_ROOT/platform-tools/adb -s $1 push $SCRIPTPATH/../fonts/SCRIPTBL.TTF /data/local/tmp/com.dewtronics.KingJamesPureBibleSearch/fonts/SCRIPTBL.TTF

$ANDROID_SDK_ROOT/platform-tools/adb -s $1 push $SCRIPTPATH/../fonts/DejaVuSans-BoldOblique.ttf /data/local/tmp/com.dewtronics.KingJamesPureBibleSearch/fonts/DejaVuSans-BoldOblique.ttf

$ANDROID_SDK_ROOT/platform-tools/adb -s $1 push $SCRIPTPATH/../fonts/DejaVuSans-Bold.ttf /data/local/tmp/com.dewtronics.KingJamesPureBibleSearch/fonts/DejaVuSans-Bold.ttf

$ANDROID_SDK_ROOT/platform-tools/adb -s $1 push $SCRIPTPATH/../fonts/DejaVuSansCondensed-BoldOblique.ttf /data/local/tmp/com.dewtronics.KingJamesPureBibleSearch/fonts/DejaVuSansCondensed-BoldOblique.ttf

$ANDROID_SDK_ROOT/platform-tools/adb -s $1 push $SCRIPTPATH/../fonts/DejaVuSansCondensed-Bold.ttf /data/local/tmp/com.dewtronics.KingJamesPureBibleSearch/fonts/DejaVuSansCondensed-Bold.ttf

$ANDROID_SDK_ROOT/platform-tools/adb -s $1 push $SCRIPTPATH/../fonts/DejaVuSansCondensed-Oblique.ttf /data/local/tmp/com.dewtronics.KingJamesPureBibleSearch/fonts/DejaVuSansCondensed-Oblique.ttf

$ANDROID_SDK_ROOT/platform-tools/adb -s $1 push $SCRIPTPATH/../fonts/DejaVuSansCondensed.ttf /data/local/tmp/com.dewtronics.KingJamesPureBibleSearch/fonts/DejaVuSansCondensed.ttf

$ANDROID_SDK_ROOT/platform-tools/adb -s $1 push $SCRIPTPATH/../fonts/DejaVuSans-ExtraLight.ttf /data/local/tmp/com.dewtronics.KingJamesPureBibleSearch/fonts/DejaVuSans-ExtraLight.ttf

$ANDROID_SDK_ROOT/platform-tools/adb -s $1 push $SCRIPTPATH/../fonts/DejaVuSansMono-BoldOblique.ttf /data/local/tmp/com.dewtronics.KingJamesPureBibleSearch/fonts/DejaVuSansMono-BoldOblique.ttf

$ANDROID_SDK_ROOT/platform-tools/adb -s $1 push $SCRIPTPATH/../fonts/DejaVuSansMono-Bold.ttf /data/local/tmp/com.dewtronics.KingJamesPureBibleSearch/fonts/DejaVuSansMono-Bold.ttf

$ANDROID_SDK_ROOT/platform-tools/adb -s $1 push $SCRIPTPATH/../fonts/DejaVuSansMono-Oblique.ttf /data/local/tmp/com.dewtronics.KingJamesPureBibleSearch/fonts/DejaVuSansMono-Oblique.ttf

$ANDROID_SDK_ROOT/platform-tools/adb -s $1 push $SCRIPTPATH/../fonts/DejaVuSansMono.ttf /data/local/tmp/com.dewtronics.KingJamesPureBibleSearch/fonts/DejaVuSansMono.ttf

$ANDROID_SDK_ROOT/platform-tools/adb -s $1 push $SCRIPTPATH/../fonts/DejaVuSans-Oblique.ttf /data/local/tmp/com.dewtronics.KingJamesPureBibleSearch/fonts/DejaVuSans-Oblique.ttf

$ANDROID_SDK_ROOT/platform-tools/adb -s $1 push $SCRIPTPATH/../fonts/DejaVuSans.ttf /data/local/tmp/com.dewtronics.KingJamesPureBibleSearch/fonts/DejaVuSans.ttf

$ANDROID_SDK_ROOT/platform-tools/adb -s $1 push $SCRIPTPATH/../fonts/DejaVuSerif-BoldItalic.ttf /data/local/tmp/com.dewtronics.KingJamesPureBibleSearch/fonts/DejaVuSerif-BoldItalic.ttf

$ANDROID_SDK_ROOT/platform-tools/adb -s $1 push $SCRIPTPATH/../fonts/DejaVuSerif-Bold.ttf /data/local/tmp/com.dewtronics.KingJamesPureBibleSearch/fonts/DejaVuSerif-Bold.ttf

$ANDROID_SDK_ROOT/platform-tools/adb -s $1 push $SCRIPTPATH/../fonts/DejaVuSerifCondensed-BoldItalic.ttf /data/local/tmp/com.dewtronics.KingJamesPureBibleSearch/fonts/DejaVuSerifCondensed-BoldItalic.ttf

$ANDROID_SDK_ROOT/platform-tools/adb -s $1 push $SCRIPTPATH/../fonts/DejaVuSerifCondensed-Bold.ttf /data/local/tmp/com.dewtronics.KingJamesPureBibleSearch/fonts/DejaVuSerifCondensed-Bold.ttf

$ANDROID_SDK_ROOT/platform-tools/adb -s $1 push $SCRIPTPATH/../fonts/DejaVuSerifCondensed-Italic.ttf /data/local/tmp/com.dewtronics.KingJamesPureBibleSearch/fonts/DejaVuSerifCondensed-Italic.ttf

$ANDROID_SDK_ROOT/platform-tools/adb -s $1 push $SCRIPTPATH/../fonts/DejaVuSerifCondensed.ttf /data/local/tmp/com.dewtronics.KingJamesPureBibleSearch/fonts/DejaVuSerifCondensed.ttf

$ANDROID_SDK_ROOT/platform-tools/adb -s $1 push $SCRIPTPATH/../fonts/DejaVuSerif-Italic.ttf /data/local/tmp/com.dewtronics.KingJamesPureBibleSearch/fonts/DejaVuSerif-Italic.ttf

$ANDROID_SDK_ROOT/platform-tools/adb -s $1 push $SCRIPTPATH/../fonts/DejaVuSerif.ttf /data/local/tmp/com.dewtronics.KingJamesPureBibleSearch/fonts/DejaVuSerif.ttf

$ANDROID_SDK_ROOT/platform-tools/adb -s $1 push $SCRIPTPATH/../doc/KingJamesPureBibleSearch.pdf /data/local/tmp/com.dewtronics.KingJamesPureBibleSearch/doc/KingJamesPureBibleSearch.pdf


