#!/bin/bash

for i in {1..10000}
do
  echo ''
  echo ''
  echo 'Current time is :'$(date "+%Y-%m-%d %H:%M:%S")
  make report
  cat ./workspace/report/time_report_face_detection.csv
  qstat -u ylxiao
  sleep 10
done
