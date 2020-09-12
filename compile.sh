#!/bin/bash
#Set additional options for compiling and running.
sources="./pointybox.cpp"
options="-Wall -Wno-switch -O3 -pipe -std=c++11 -lsfml-system -lsfml-window -lsfml-graphics"
#Display g++ version before building
s1="Building using $(g++ --version | grep --color=never "g++")"
s2="Sources: $sources"
s3="Options: $options"

w=${#s1}
if [ ${#s2} -gt $w ]; then
    w=${#s2}
fi
if [ ${#s3} -gt $w ]; then
    w=${#s3}
fi

printf "$s1\n$s2\n$s3\n"
for (( x=0; x < $w; x++)); do
    printf "-"
done
printf "\n"
#Build the project with current directory and additional user options. Project must have a main.cpp file as the main source.
g++ "./main.cpp" $sources $options -o "./bin/sublime_out"
#Get exitcode for later use.
exitcode=$?
if [ $exitcode -ne 0 ];then #If the exitcode is not equal to 0 (EXIT_SUCCESS) then it failed.
    echo Build failed! Exit code $exitcode.
else #If the exitcode is equal to 0 (EXIT_SUCCESS) then it succeded.
    echo Build finished!
    ./run.sh
fi
