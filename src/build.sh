#!/bin/bash
cmd="make NOUPX=1 NOVERSION=1"
windows=false
while [[ $# -gt 0 ]]; do
    case $1 in
        rebuild)
        cmd="${cmd} -B"
        shift
        ;;
        windows)
        windows=true
        shift
        ;;
    esac
done
if $windows; then
    cmd="${cmd} MINGW=1 SDL=1 PREFIX=i686-w64-mingw32"
else
    cmd="${cmd} LINUX64=1"
fi

echo "$cmd"
eval "$cmd"
