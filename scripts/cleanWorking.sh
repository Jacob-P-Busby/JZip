#!/bin/bash

if ! ls ../working/*.jzip > /dev/null 2> /dev/null; # if there are no .jzip files in the working directory
then
    rm ../working/*.jzip # Remove them
    echo "Removed .jzip files from working directory"
    exit 0;
fi

echo "No .jzip files in working directory"
exit 0;
