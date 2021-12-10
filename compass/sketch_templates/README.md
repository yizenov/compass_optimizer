# Pre-built Fast-AGMS sketch details

In case of no selection predicates on base tables, the pre-built sketches are used to further optimize the runtime performance by avoiding table-scan operators.

The pre-built sketches are build over all the table tuples and built for each join attribute in the JOB workload.

There are 106 pre-built Fast-AGMS sketches (53 different join connections) build for the workload. We already uploaded the pre-built sketches for sketch sizes `11 x 1021` on the `imdb` workload. However one may need to re-create the templates in case of changing sketch sizes by running the queries in [`template-queries`](https://github.com/yizenov/compass_optimizer/tree/master/compass/sketch_templates/template_queries).


In order create the pre-build sketches:
- set `PRE_PROCESSING` to `true`
- run 53 provided queries in [`template_queries`](https://github.com/yizenov/compass_optimizer/tree/master/compass/sketch_templates/template_queries)
- set the correct paths to the sketch files shown below.

There are two `.txt` files that are read when MapD server starts. `1021` is the bucket size and `11` is the number base sketches.
- [`sketch_templates_1021_11.txt`](https://github.com/yizenov/compass_optimizer/blob/master/compass/sketch_templates/sketch_templates_1021_11.txt)
- [`sketch_templates_seeds_1021_11.txt`](https://github.com/yizenov/compass_optimizer/blob/master/compass/sketch_templates/sketch_templates_seeds_1021_11.txt)

New seeds provide different sketches thus providing different estimations. However the trend shown in the paper are preserved regardless of the sketch seed values.

The sketch templates are written in the following way:
- each template description contains two consequetive rows.
- first row includes `left_table_id`, `column_id`, `right_table_id`, `column_id`, `bucket_size` and `sketch_nbr`.
- second row represents sketch values. Sketch values are stored as a 1-D array i.e. read sketch by sketch.

The seeds are written in the following way:
- each sketch seed description contains three consequetive rows.
- first row includes `left_table_id`, `column_id`, `right_table_id`, `column_id`, `bucket_size` and `sketch_nbr`.
- second and third rows are seed values. Each seed is a tuple of two numbers.
  - second row seeds correspond to `Xi_CW2B` values.
  - third row seeds correspond to `Xi_EH3` values.

See `uploadSketches` and `saveSketches` methods for details in [`compass/mapd-core/Catalog/Catalog.h`](https://github.com/yizenov/compass_optimizer/blob/master/compass/mapd-core/Catalog/Catalog.h).

# Useful queries to identify/retrieve table ids in MapD:
select count(\*) from aka_name as an1, aka_name as an2 where an1.id = an2.id; <br/>
select count(\*) from aka_title as att1, aka_title as att2 where att1.id = att2.id; <br/>
select count(\*) from cast_info as ci1, cast_info as ci2 where ci1.id = ci2.id; <br/>
select count(\*) from char_name as chn1, char_name as chn2 where chn1.id = chn2.id; <br/>
select count(\*) from company_name as cn1, company_name as cn2 where cn1.id = cn2.id; <br/>
select count(\*) from company_type as ct1, company_type as ct2 where ct1.id = ct2.id; <br/>
select count(\*) from comp_cast_type as cct1, comp_cast_type as cct2 where cct1.id = cct2.id; <br/>
select count(\*) from complete_cast as cc1, complete_cast as cc2 where cc1.id = cc2.id; <br/>

select count(\*) from info_type as it1, info_type as it2 where it1.id = it2.id; <br/>
select count(\*) from keyword as k1, keyword as k2 where k1.id = k2.id; <br/>
select count(\*) from kind_type as kt1, kind_type as kt2 where kt1.id = kt2.id; <br/>
select count(\*) from link_type as lt1, link_type as lt2 where lt1.id = lt2.id; <br/>
select count(\*) from movie_companies as mc1, movie_companies as mc2 where mc1.id = mc2.id; <br/>
select count(\*) from movie_info_idx as mi_idx1, movie_info_idx as mi_idx2 where mi_idx1.id = mi_idx2.id; <br/>
select count(\*) from movie_keyword as mk1, movie_keyword as mk2 where mk1.id = mk2.id; <br/>
select count(\*) from movie_link as ml1, movie_link as ml2 where ml1.id = ml2.id; <br/>

select count(\*) from name as n1, name as n2 where n1.id = n2.id; <br/>
select count(\*) from role_type as rt1, role_type as rt2 where rt1.id = rt2.id; <br/>
select count(\*) from title as t1, title as t2 where t1.id = t2.id; <br/>
select count(\*) from movie_info as mi1, movie_info as mi2 where mi1.id = mi2.id; <br/>
select count(\*) from person_info as pi1, person_info as pi2 where pi1.id = pi2.id; <br/>
