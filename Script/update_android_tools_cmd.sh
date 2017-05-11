#!/bin/zsh
# This script updates Android SDK Command Line Tools to latest version
# Make sure command line tools is placed in ANDROID_SDK_ROOT directory.
#
# Download command line tools from official Android site which download url 
# looks like https://dl.google.com/android/repository/sdk-tools-linux-3859397.zip
# You will get a directory named 'tools' from the zip file, put it under a 
# empty directory with an easy-to-understand name like 'android-sdk'.
#
# You must use sdkmanager to install an arbitrary package at least once
# or you will always be stucked at license agreement page in this script.

ANDROID_SDK_ROOT=$1

if [ ! -d $ANDROID_SDK_ROOT ]; then
	echo "Given Android SDK root $ANDROID_SDK_ROOT does not exist"
	exit 127
fi

# update sdkmanager
echo 'Updating SDK Manager'
$ANDROID_SDK_ROOT/tools/bin/sdkmanager --update

if [[ $? != 0 ]]; then
	echo "Update SDK Manager failed"
	exit $?
fi

# get latest version of build-tools
# package path will be like 'build-tools;25.0.0'
echo 'Downloading packages list'
latest_build_tools_list_row=`$ANDROID_SDK_ROOT/tools/bin/sdkmanager --list | grep 'build-tools;' | sort -n | tail -n1`

if [[ $? != 0 ]]; then
	echo "Downloading packages list failed"
	exit $?
fi

# check for empty string
if [[ $latest_build_tools_list_row = *[!\ ]* ]]; then
	echo "build-tools 404 in list"
	exit 3
fi

echo 'Find build-tools in package list, latest in rows: `'$latest_build_tools_list_row'`'

latest_build_tools_package=`echo $latest_build_tools_list_row | awk -F "|" '{print $1}' | tr -d ' '`
latest_build_tools_ver=`echo $latest_build_tools_list_row | awk -F "|" '{print $2}' | tr -d ' '`

echo "Installing build-tools version $latest_build_tools_ver ($latest_build_tools_package)"

# install latest build-tools
$ANDROID_SDK_ROOT/tools/bin/sdkmanager $latest_build_tools_package

rm "$ANDROID_SDK_ROOT/../android-sdk-build-tools_latest"
ln -s "$ANDROID_SDK_ROOT/build-tools/$latest_build_tools_ver/" "$ANDROID_SDK_ROOT/../android-sdk-build-tools_latest"
