#!/bin/bash

template_data_path="../QueryEngine/Sketch/sketch_templates.txt"
template_seed_path="../QueryEngine/Sketch/sketch_templates_seeds.txt"
rm ${template_data_path}
rm ${template_seed_path}
touch ${template_data_path}
touch ${template_seed_path}

# set PRE_PROCESSING to true
# sketch type = 1/0 -- no need

# sketch_templates.txt
# left_table_id left_column_id right_table_id right_column_id sketch_type buckets rows
  # values

# sketch_templates_seeds.txt
# left_table_id left_column_id right_table_id right_column_id sketch_type buckets rows
  # seed 1
  # seed 2

#screen -S templates_create -dm -L sh -c 'time ./templates_create.sh'

server_run="../build/bin/mapd_server --num-gpus 1 --start-gpu 0 -p 7071 --http-port 7070 --data /home/username/.mapd/data/"
client_run="../build/bin/mapdql imdb -u mapd --port 7071 -p HyperInteractive"

input_folder=../../../TEMPLATE_QUERIES
output_folder=../../../TEMPLATE_QUERIES_RESULTS

iterations=1
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
  rm ${client_query}
  rm ${client_result}
	touch ${client_query}
	touch ${client_result}

  echo "\gpu" > ${client_query}
  echo "${mode}" >> ${client_query}
	cat "${input_folder}/${sql}" >> "${client_query}"
	echo "\q" >> ${client_query}
	chmod +x ${client_query}

  echo "" >> ${server_file}
  echo "${sql}" >> ${server_file}

  iter=1
  while [ $iter -le ${iterations} ]
  do
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
