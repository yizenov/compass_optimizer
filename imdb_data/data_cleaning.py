import os
import csv
import random

##############################
""" 
1. REPLACING ',' with '|' separator because commas in text values in columns.
    - replaced '|' with ' '

2. MapD outputs the following error:
    "Input exception thrown: String too long for column info was 34827 max is 32767. Row discarded, issue at column : 4 data"
      - This is the error for 14 string values.
      - The values are too long for columns "info: 4th column" and "note: 5th column".
      - Both columns are in person_info.csv file/relation.
    The queries that have selection predicates on these two columns are the following:
      - queries: 7a, 7b, 7c (pi.note = 'Volker Boehm' and pi.note is not null)
    This script cuts the string values which are longer then 29,000 characters.
      - 29,000 is the maximum string length that MapD supports.
      - This change doesn't effect the query results.

3. Primary-Keys used in the join predicates in JOB
    -a. char_name chn
    -b. comp_cast_type cct
    -c. company_name cn
    -d. company type ct
    -e. info_type it
    -f. keyword k
    -g. kind_type kt
    -h. link_type lt
    -i. name n
    -j. role_type rt
    -k. title t
There are 11 PKs used in the workload join predicates.

There are empty values in cast_info.person_id (FK) which joins with name.id (PK).
In MapD, this causes an error, and there are such 18,672,825 out of 36,244,344 tuples.
    - replaced the empty values with randomly selected from name.id domain range.
"""
##############################

pre_path = ""
input_folder_n, output_folder_n = "imdb", "postgres_mapd_imdb"
file_names = sorted(os.listdir(pre_path + input_folder_n))

name_primary_keys = []
input_f_csv_file = pre_path + input_folder_n + "/name.csv"
with open(input_f_csv_file, "r") as result_f:
    for idx, line in enumerate(result_f):
        line = line.strip().split(",")
        name_primary_keys.append(line[0])

for f_idx, ff in enumerate(file_names):
    print(ff)
    if ff != "cast_info.csv": continue
    output_f_csv_file = pre_path + output_folder_n + "/" + ff
    output_f = open(output_f_csv_file, "w")
    output_f_writer = csv.writer(output_f, delimiter='|')

    missing_ci_person_role_id_values = 0
    input_f_csv_file = pre_path + input_folder_n + "/" + ff
    with open(input_f_csv_file, "r") as result_f:
        for idx, line in enumerate(result_f):
            line = line.strip().split(",")
            value_cols, text_cols, new_row = [], [], []

            if ff in ["cast_info.csv"]:
                start, end = 4, 2
                text_col = ",".join(line[start:len(line) - end])
                text_col = text_col.replace("|", " ")
                if line[3] == "":
                    missing_ci_person_role_id_values += 1  # 18,672,825 out of 36,244,344
                    key_idx = random.randint(0, len(name_primary_keys)-1)
                    line[3] = name_primary_keys[key_idx]
                if len(text_col) != 0 and text_col[0] == "\"": text_col = text_col[1:]
                if len(text_col) != 0 and text_col[len(text_col) - 1] == "\"": text_col = text_col[:-1]
                [new_row.append(val) for val in line[:start]]
                new_row.append(text_col)
                [new_row.append(val) for val in line[len(line) - end:]]

            elif ff in [
                "aka_name.csv", "aka_title.csv", "char_name.csv",
                "company_name.csv", "keyword.csv", "movie_companies.csv",
                "movie_info_idx.csv", "name.csv", "title.csv"
            ]:
                start, end = -1, -1
                if ff == "aka_name.csv": start, end = 2, 5
                elif ff == "aka_title.csv": start, end = 2, 9
                # elif ff == "cast_info.csv": start, end = 4, 2
                elif ff == "char_name.csv": start, end = 1, 5
                elif ff == "company_name.csv": start, end = 1, 5
                elif ff == "keyword.csv": start, end = 1, 1
                elif ff == "movie_companies.csv": start, end = 4, 0
                elif ff == "movie_info_idx.csv": start, end = 3, 1
                elif ff == "name.csv": start, end = 1, 7
                elif ff == "title.csv": start, end = 1, 10
                text_col = ",".join(line[start:len(line) - end])
                text_col = text_col.replace("|", " ")
                if len(text_col) != 0:
                    if text_col[0] == "\"": text_col = text_col[1:]
                    if text_col[len(text_col) - 1] == "\"": text_col = text_col[:-1]
                [new_row.append(val) for val in line[:start]]
                new_row.append(text_col)
                [new_row.append(val) for val in line[len(line) - end:]]
            elif ff in [
                "comp_cast_type.csv", "company_type.csv", "complete_cast.csv",
                "info_type.csv", "kind_type.csv", "link_type.csv",
                "movie_keyword.csv", "movie_link.csv", "role_type.csv"
            ]:
                end = -1
                if ff == "comp_cast_type.csv":  end = 2
                elif ff == "company_type.csv":  end = 2
                elif ff == "complete_cast.csv":  end = 4
                elif ff == "info_type.csv":  end = 2
                elif ff == "kind_type.csv":  end = 2
                elif ff == "link_type.csv":  end = 2
                elif ff == "movie_keyword.csv": end = 3
                elif ff == "movie_link.csv": end = 4
                elif ff == "role_type.csv": end = 2
                [new_row.append(val) for val in line[:end]]
            elif ff in [
                "movie_info.csv", "person_info.csv"
            ]:
                start1, start2, end = 3, 4, 1
                text_col1 = ",".join(line[start1:len(line)-end])
                text_col1 = text_col1.replace("|", " ")
                if ff == "person_info.csv" and len(text_col1) > 29000: text_col1 = text_col1[:29000]
                text_col2 = ",".join(line[start2:])
                text_col2 = text_col2.replace("|", " ")
                if ff == "person_info.csv" and len(text_col2) > 29000: text_col2 = text_col2[:29000]
                [new_row.append(val) for val in line[:start1]]
                new_row.append(text_col1)
                new_row.append(text_col2)
            output_f_writer.writerow(new_row)
        print("missing_ci_person_role_id_values: " + str(missing_ci_person_role_id_values))
    output_f.close()
