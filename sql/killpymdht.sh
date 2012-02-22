#!/bin/sh
sleep 120
PROCESSID=$(ps axj | grep run_pymdht_nod | grep python | awk '{ print $2}')
echo $PROCESSID
kill -2 $PROCESSID
