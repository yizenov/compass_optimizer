#!/bin/bash

#screen (-ls, exit)
#control+a d
#screen -r pid.tty.host
#screen -S estimate_check -dm -L sh -c 'time ./estimate_check.sh'
# (-Logfile predicate_check.log, saved in screenlog.0)

server_run="../build/bin/mapd_server --num-gpus 1 --start-gpu 0 -p 7071 --http-port 7070 --data /home/username/.mapd/data/"
client_run="../build/bin/mapdql imdb -u mapd --port 7071 -p HyperInteractive"

input_folder=../../../SUB_QUERIES_EST/MAPD_SUBS_EST_1_0

output_folder=../../../SUB_QUERIES_EST/MAPD_SUBS_EST_1_0_RESULTS

iterations=1
mode=\\fpd

echo "starting the queries"
for sql_folder in $(ls ${input_folder})
do
  echo ""
  echo "starting the query ${sql_folder}"
  rm -r ${output_folder}/${sql_folder}
  mkdir ${output_folder}/${sql_folder}

  server_file=${output_folder}/${sql_folder}/${sql_folder}.server
  touch ${server_file}

  for sql in $(ls ${input_folder}/${sql_folder})
  do
    # starting the server
    ${server_run} >> ${server_file} & P1=$!
    sleep 10s & K1=$!
    wait -n
    wait $K1

    echo "file-name: ${sql}"
    client_query=${output_folder}/${sql_folder}/${sql}.mapd_sql
    client_result=${output_folder}/${sql_folder}/${sql}.result
	  touch ${client_query}
	  touch ${client_result}

    echo "\gpu" > ${client_query}
    echo "${mode}" >> ${client_query}
	  cat "${input_folder}/${sql_folder}/${sql}" >> "${client_query}"
	  echo "\q" >> ${client_query}
	  chmod +x ${client_query}

    echo "" >> ${server_file}
    echo "${sql}" >> ${server_file}

    iter=1
    while [ $iter -le ${iterations} ]
    do
      echo "" >> ${server_file}
      echo "iteration #${iter}" >> ${server_file}

      # starting the client
      (cat ${client_query} | ${client_run} >> ${client_result}) & P2=$!
      wait -n
      wait $P2;
      echo "+++++++++++++++++++++++++++++++++++++++++++" >> ${client_result}

      ((iter++))
    done;

    kill -9 $P1 & K2=$!
    wait -n
    wait $K2

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
done;
