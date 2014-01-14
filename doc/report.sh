#!/bin/sh

(echo "date: "
date
echo "uname: "
uname -a
echo "ps: "
ps -auxw | grep tinyproxy | grep -v grep
echo "ver: "
if [ -x /usr/local/bin/tinyproxy ]; then
   /usr/local/bin/tinyproxy -v
else
   echo no ver available.
fi;) 2>&1 | mail -s 'tinyproxy install' sdyoung@well.com
