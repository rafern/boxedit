#!/bin/bash
args="pointyboxes.pb tilemap.png"

if xhost >& /dev/null ; then
    (cd "./bin" && ./sublime_out $args)
    echo Program terminated! Exit code $?.
else
    echo Starting with xinit!
    (cd "./bin" && xinit ./sublime_out $args -- :1 vt8)
    echo Program terminated! Exit code $?.
fi
