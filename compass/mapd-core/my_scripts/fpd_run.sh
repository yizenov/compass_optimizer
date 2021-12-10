#!/bin/bash

#screen (-ls, exit)
#control+a d
#screen -r pid.tty.host
#screen -S fpd_run -dm -L sh -c 'time ./fpd_run.sh' (saved in screenlog.0, -Logfile fpd_run.log)

server_run="../build/bin/mapd_server --num-gpus 1 --start-gpu 0 -p 7071 --http-port 7070 --data /home/username/.mapd/data/"
client_run="../build/bin/mapdql imdb -u mapd --port 7071 -p HyperInteractive"
# mapdql.cpp and MapDServer.cpp are files to find the other config parameters
# https://docs.omnisci.com/installation-and-configuration/config-parameters

#input_folder=../../../JOB_QUERIES
input_folder=../../../JOB_QUERIES_FULL

#output_folder=../../../JOB_RESULTS_NOFPD
#output_folder=../../../JOB_RESULTS_WITHFPD
#output_folder=../../../JOB_RESULTS_WITHFPD_FULL
output_folder=../../../JOB_RESULTS_WITHFPD_CPU

iterations=3
device_type=\\cpu
mode=\\fpd

echo "starting the queries"
for sql in $(ls ${input_folder})
do
  echo ""
  echo "starting the query ${sql}"

  server_file=${output_folder}/${sql}.server
  rm ${server_file}
  touch ${server_file}

  # starting the server
  (${server_run} >> ${server_file}) & P1=$!
  sleep 10s & K1=$!
  wait $K1

  echo "file-name: ${sql}"
  client_query=${output_folder}/${sql}.mapd_sql
  client_result=${output_folder}/${sql}.result
	touch ${client_query}
	touch ${client_result}

  echo "${device_type}" > ${client_query}
  echo "${mode}" >> ${client_query}
	#echo "\status" >> ${client_query}
	#echo "\memory_summary" >> ${client_query}
	echo "\timing" >> ${client_query}
	cat "${input_folder}/${sql}" >> "${client_query}"
	echo "\q" >> ${client_query}
	chmod +x ${client_query}

  echo "" >> ${server_file}
  echo "${sql}" >> ${server_file}

  iter=1
  while [ $iter -le ${iterations} ]
  do
    echo "" >> ${server_file}
    echo "iteration #${iter}" >> ${server_file}
    echo "iteration #${iter}"

    # starting the client
    (cat ${client_query} | ${client_run} >> ${client_result}) & P2=$!
    sleep 3s & K2=$!
    wait $K2
    wait $P2

    kill -9 $P2 & K3=$!
    wait $K3
    echo "+++++++++++++++++++++++++++++++++++++++++++" >> ${client_result}
    echo "" >> ${client_result}

    ((iter++))
  done;

  echo "" >> ${server_file}
  kill -9 $P1 & K4=$!
  wait $K4

  # killing server and java processes first
  java_process=$(ps -u username | grep java | awk '{print $1}')
  kill -9 ${java_process} & K5=$!
  wait -n
  wait $K5

  server_process=$(ps -u username | grep '$USER_mapd_server' | awk '{print $1}')
  kill -9 ${server_process} & K6=$!
  wait -n
  wait $K6

  sleep 3s & K2=$!
  wait $K2

done;
