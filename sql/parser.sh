#!/bin/sh
egrep '[[:digit:]]{1,3}\.[[:digit:]]{1,3}\.[[:digit:]]{1,3}\.[[:digit:]]{1,3}' < input.txt | tr -d "'[]" | sed -e 's/), /)\n/g' | tr -d "()" | tr -d " " > output.txt
