#!/bin/bash

#saved in screenlog.0 -L -Logfile docker_psql_bench.log
#screen -S docker_psql -dm -L sh -c 'time ./benchmark_psql.sh'
#rm screenlog.0

container_name=compass-pg

#input_folder=~/SELECTION_QUERIES
#output_folder=~/PSQL_SELECTION_QUERIES_RESULTS

input_folder=~/QUERIES_COMPASS_PostgreSQL
output_folder=~/QUERIES_RESULTS

#input_folder=~/SUBPLANS_QUERIES
#output_folder=~/PSQL_SUBPLAN_RESULTS_EST
#output_folder=~/PSQL_SUBPLAN_RESULTS_TRUE

psql_key=""
#psql_key="EXPLAIN ANALYZE"
psql_key="EXPLAIN"

docker_server_run="docker exec -i ${container_name} psql -h localhost -U postgres -d imdb"

echo "starting the query runs: " -- `date +"%Y-%m-%d %T"`
for query_file in $(ls ${input_folder})
do
  echo ${query_file} -- `date +"%Y-%m-%d %T"`
  query_result=${output_folder}/${query_file}.result

  client_query=${output_folder}/${query_file}
  rm ${client_query}
  touch ${client_query}
  echo "${psql_key} " > ${client_query}
  cat "${input_folder}/${query_file}" >> ${client_query}
  chmod +x ${client_query}

  (${docker_server_run} < ${client_query} >> ${query_result}) & P0=$!
  wait $P0

done;
echo "finishing the query runs: " -- `date +"%Y-%m-%d %T"`
