#!/bin/bash
if [ $# -gt 1 ];then
    echo "[Usage]: $0 or $0 clean."
    exit
elif [ $# -eq 0 ];then
    ndk-build NDK_PROJECT_PATH=. APP_BUILD_SCRIPT=Android.mk NDK_APPLICATION_MK=Application.mk
elif [ $# -eq 1 ];then
    if [ "$1" = "clean" ];then
        ndk-build NDK_PROJECT_PATH=. APP_BUILD_SCRIPT=Android.mk NDK_APPLICATION_MK=Application.mk clean
        exit
    else
        echo "[Usage]: $0 or $0 clean."
        exit
    fi
fi
