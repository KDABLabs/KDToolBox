#!/bin/sh
# SPDX-FileCopyrightText: 2020-2022 Klaravdalens Datakonsult AB (KDAB), author David Faure <david.faure@kdab.com>
# SPDX-License-Identifier: MIT

# Generate a CMakeLists.txt based on the .cpp files in the current directory
# Similar to qmake -project, in essence.


usage()
{
  echo "Usage: $0 [ -q | --qt 5 | 6 | none ]"
  echo "The -q or --qt option allows to choose between Qt 5, Qt 6, or no Qt dependency"
  exit 2
}

PARSED_ARGUMENTS=$(getopt -a -n cmake-project -o q:,h --long qt:,help -- "$@")
INVALID_ARGUMENTS=$?
if [ "$INVALID_ARGUMENTS" != "0" ]; then
  usage
fi

eval set -- "$PARSED_ARGUMENTS"

QTVERSION=5
while :
do
  case "$1" in
    -q | --qt)   QTVERSION="$2"      ; shift 2  ;;
    -h | --help) usage; ;;
    --) shift; break;;
    *) echo "Unexpected option: $1"; usage; ;; # can't happen
  esac
done

if [ -f CMakeLists.txt ]; then
    echo "CMakeLists.txt already exists, aborting for safety reasons"
    exit 1
fi
FILES="`find -name '*.cpp' -o -name '*.qrc' -o -name '*.ui' | sed -e 's,\.\/,  ,'`"
if [ -z "$FILES" ]; then
    echo "No cpp files found in the current directory"
    exit 1
fi
PROJECTNAME=`basename "$PWD" | sed -e 's/ /_/g'`
TARGET="$PROJECTNAME"

if [ "$QTVERSION" != "none" ]; then
    QT=Qt$QTVERSION
    QTLIBS="Core Gui"
    QTTARGETS="$QT::Core $QT::Gui"
    if [ -n "`grep QApplication *.cpp`" ]; then
        QTLIBS="$QTLIBS Widgets"
        QTTARGETS="$QTTARGETS $QT::Widgets"
        AUTOUIC="set(CMAKE_AUTOUIC TRUE)"
    fi
fi

cat > CMakeLists.txt <<EOF
cmake_minimum_required(VERSION 3.9)
project($PROJECTNAME)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
EOF

if [ "$QTVERSION" != "none" ]; then
cat >> CMakeLists.txt <<EOF
find_package($QT CONFIG REQUIRED COMPONENTS $QTLIBS)
set(CMAKE_AUTOMOC TRUE)
set(CMAKE_AUTORCC TRUE)
$AUTOUIC
EOF
fi

cat >> CMakeLists.txt <<EOF
add_executable($PROJECTNAME
$FILES
)
EOF

if [ "$QTVERSION" != "none" ]; then
cat >> CMakeLists.txt <<EOF
target_link_libraries($PROJECTNAME $QTTARGETS)
EOF
fi
