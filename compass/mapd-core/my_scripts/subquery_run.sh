#!/bin/bash

#screen (-ls, exit)
#control+a d
#screen -r pid.tty.host
#screen -S subquery_run -dm -L sh -c 'time ./subquery_run.sh' -Logfile subquery_run.log (saved in screenlog.0)

server_run="../build/bin/mapd_server --num-gpus 1 --start-gpu 0 -p 7071 --http-port 7070 --data /home/username/.mapd/data/"
client_run="../build/bin/mapdql imdb -u mapd --port 7071 -p HyperInteractive"

input_folder=../../../SUB_QUERIES/SKETCH_SUBS_1_0_base

output_folder=../../../SUB_QUERIES/SKETCH_SUBS_1_0_base_RESULTS

iterations=2
mode=\\fpd

echo "starting the queries"
for sql_folder in $(ls ${input_folder})
do
  echo "starting the query ${sql_folder}"
  rm -r ${output_folder}/${sql_folder}
  mkdir ${output_folder}/${sql_folder}

  for sql in $(ls ${input_folder}/${sql_folder})
  do
    server_file=${output_folder}/${sql_folder}/${sql}.server
    touch ${server_file}

    # starting the server
    ${server_run} >> ${server_file} & P1=$!
    sleep 10s & K1=$!
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
      sleep 3s & K2=$!
      wait $K2
      wait $P2

      kill -9 $P2 & K3=$!
      wait $K3
      echo "+++++++++++++++++++++++++++++++++++++++++++" >> ${client_result}

      ((iter++))
    done;

    kill -9 $P1 & K4=$!
    wait $K4

    # killing server and java processes first
    java_process=$(ps -u username | grep java | awk '{print $1}')
    kill -9 ${java_process} & K5=$!
    wait $K5

    server_process=$(ps -u username | grep '$USER_mapd_server' | awk '{print $1}')
    kill -9 ${server_process} & K6=$!
    wait $K6

    sleep 3s & K2=$!
    wait $K2

  done;
done;
