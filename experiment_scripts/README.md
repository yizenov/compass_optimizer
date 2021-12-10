
# Useful queries and commands

select count(\*) from aka_name;<br/>
select count(\*) from aka_title;<br/>
select count(\*) from cast_info;<br/>
select count(\*) from char_name;<br/>
select count(\*) from company_name;<br/>
select count(\*) from company_type;<br/>
select count(\*) from comp_cast_type;<br/>
select count(\*) from complete_cast;<br/>
select count(\*) from info_type;<br/>
select count(\*) from keyword;<br/>
select count(\*) from kind_type;<br/>
select count(\*) from link_type;<br/>
select count(\*) from movie_companies;<br/>
select count(\*) from movie_info_idx;<br/>
select count(\*) from movie_keyword;<br/>
select count(\*) from movie_link;<br/>
select count(\*) from name;<br/>
select count(\*) from role_type;<br/>
select count(\*) from title;<br/>
select count(\*) from movie_info;<br/>
select count(\*) from person_info;<br/>

drop table aka_name;<br/>
drop table aka_title;<br/>
drop table cast_info;<br/>
drop table char_name;<br/>
drop table company_name;<br/>
drop table company_type;<br/>
drop table comp_cast_type;<br/>
drop table complete_cast;<br/>
drop table info_type;<br/>
drop table keyword;<br/>
drop table kind_type;<br/>
drop table link_type;<br/>
drop table movie_companies;<br/>
drop table movie_info_idx;<br/>
drop table movie_keyword;<br/>
drop table movie_link;<br/>
drop table name;<br/>
drop table role_type;<br/>
drop table title;<br/>
drop table movie_info;<br/>
drop table person_info;<br/>

truncate table aka_name;<br/>
truncate table aka_title;<br/>
truncate table cast_info;<br/>
truncate table char_name;<br/>
truncate table company_name;<br/>
truncate table company_type;<br/>
truncate table comp_cast_type;<br/>
truncate table complete_cast;<br/>
truncate table info_type;<br/>
truncate table keyword;<br/>
truncate table kind_type;<br/>
truncate table link_type;<br/>
truncate table movie_companies;<br/>
truncate table movie_info_idx;<br/>
truncate table movie_keyword;<br/>
truncate table movie_link;<br/>
truncate table name;<br/>
truncate table role_type;<br/>
truncate table title;<br/>
truncate table movie_info;<br/>
truncate table person_info;<br/>

wc -l aka_name.csv;<br/>
wc -l aka_title.csv;<br/>
wc -l cast_info.csv;<br/>
wc -l char_name.csv;<br/>
wc -l company_name.csv;<br/>
wc -l company_type.csv;<br/>
wc -l comp_cast_type.csv;<br/>
wc -l complete_cast.csv;<br/>
wc -l info_type.csv;<br/>
wc -l keyword.csv;<br/>
wc -l kind_type.csv;<br/>
wc -l link_type.csv;<br/>
wc -l movie_companies.csv;<br/>
wc -l movie_info_idx.csv;<br/>
wc -l movie_keyword.csv;<br/>
wc -l movie_link.csv;<br/>
wc -l name.csv;<br/>
wc -l role_type.csv;<br/>
wc -l title.csv;<br/>
wc -l movie_info.csv;<br/>
wc -l person_info.csv;<br/>

# Transfer table.csv into a docker container.
docker cp origin_imdb_bar_delim/aka_name.csv pg-docker-est:/var/lib/postgresql/data/imdb_bar_delim;<br/>
docker cp origin_imdb_bar_delim/aka_title.csv pg-docker-est:/var/lib/postgresql/data/imdb_bar_delim;<br/>
docker cp origin_imdb_bar_delim/cast_info.csv pg-docker-est:/var/lib/postgresql/data/imdb_bar_delim;<br/>
docker cp origin_imdb_bar_delim/char_name.csv pg-docker-est:/var/lib/postgresql/data/imdb_bar_delim;<br/>
docker cp origin_imdb_bar_delim/company_name.csv pg-docker-est:/var/lib/postgresql/data/imdb_bar_delim;<br/>
docker cp origin_imdb_bar_delim/company_type.csv pg-docker-est:/var/lib/postgresql/data/imdb_bar_delim;<br/>
docker cp origin_imdb_bar_delim/comp_cast_type.csv pg-docker-est:/var/lib/postgresql/data/imdb_bar_delim;<br/>
docker cp origin_imdb_bar_delim/complete_cast.csv pg-docker-est:/var/lib/postgresql/data/imdb_bar_delim;<br/>
docker cp origin_imdb_bar_delim/info_type.csv pg-docker-est:/var/lib/postgresql/data/imdb_bar_delim;<br/>
docker cp origin_imdb_bar_delim/keyword.csv pg-docker-est:/var/lib/postgresql/data/imdb_bar_delim;<br/>
docker cp origin_imdb_bar_delim/kind_type.csv pg-docker-est:/var/lib/postgresql/data/imdb_bar_delim;<br/>
docker cp origin_imdb_bar_delim/link_type.csv pg-docker-est:/var/lib/postgresql/data/imdb_bar_delim;<br/>
docker cp origin_imdb_bar_delim/movie_companies.csv pg-docker-est:/var/lib/postgresql/data/imdb_bar_delim;<br/>
docker cp origin_imdb_bar_delim/movie_info_idx.csv pg-docker-est:/var/lib/postgresql/data/imdb_bar_delim;<br/>
docker cp origin_imdb_bar_delim/movie_keyword.csv pg-docker-est:/var/lib/postgresql/data/imdb_bar_delim;<br/>
docker cp origin_imdb_bar_delim/movie_link.csv pg-docker-est:/var/lib/postgresql/data/imdb_bar_delim;<br/>
docker cp origin_imdb_bar_delim/name.csv pg-docker-est:/var/lib/postgresql/data/imdb_bar_delim;<br/>
docker cp origin_imdb_bar_delim/role_type.csv pg-docker-est:/var/lib/postgresql/data/imdb_bar_delim;<br/>
docker cp origin_imdb_bar_delim/title.csv pg-docker-est:/var/lib/postgresql/data/imdb_bar_delim;<br/>
docker cp origin_imdb_bar_delim/movie_info.csv pg-docker-est:/var/lib/postgresql/data/imdb_bar_delim;<br/>
docker cp origin_imdb_bar_delim/person_info.csv pg-docker-est:/var/lib/postgresql/data/imdb_bar_delim;<br/>

# Import data from a csv into a table in PostgreSQL.
copy aka_name from '/var/lib/postgresql/data/imdb_bar_delim/aka_name.csv' delimiter '|' null '';<br/>
copy aka_title from '/var/lib/postgresql/data/imdb_bar_delim/aka_title.csv' delimiter '|' null '';<br/>
copy cast_info from '/var/lib/postgresql/data/imdb_bar_delim/cast_info.csv' delimiter '|' null '';<br/>
copy char_name from '/var/lib/postgresql/data/imdb_bar_delim/char_name.csv' delimiter '|' null '';<br/>
copy company_name from '/var/lib/postgresql/data/imdb_bar_delim/company_name.csv' delimiter '|' null '';<br/>
copy company_type from '/var/lib/postgresql/data/imdb_bar_delim/company_type.csv' delimiter '|' null '';<br/>
copy comp_cast_type from '/var/lib/postgresql/data/imdb_bar_delim/comp_cast_type.csv' delimiter '|' null '';<br/>
copy complete_cast from '/var/lib/postgresql/data/imdb_bar_delim/complete_cast.csv' delimiter '|' null '';<br/>
copy info_type from '/var/lib/postgresql/data/imdb_bar_delim/info_type.csv' delimiter '|' null '';<br/>
copy keyword from '/var/lib/postgresql/data/imdb_bar_delim/keyword.csv' delimiter '|' null '';<br/>
copy kind_type from '/var/lib/postgresql/data/imdb_bar_delim/kind_type.csv' delimiter '|' null '';<br/>
copy link_type from '/var/lib/postgresql/data/imdb_bar_delim/link_type.csv' delimiter '|' null '';<br/>
copy movie_companies from '/var/lib/postgresql/data/imdb_bar_delim/movie_companies.csv' delimiter '|' null '';<br/>
copy movie_info_idx from '/var/lib/postgresql/data/imdb_bar_delim/movie_info_idx.csv' delimiter '|' null '';<br/>
copy movie_keyword from '/var/lib/postgresql/data/imdb_bar_delim/movie_keyword.csv' delimiter '|' null '';<br/>
copy movie_link from '/var/lib/postgresql/data/imdb_bar_delim/movie_link.csv' delimiter '|' null '';<br/>
copy name from '/var/lib/postgresql/data/imdb_bar_delim/name.csv' delimiter '|' null '';<br/>
copy role_type from '/var/lib/postgresql/data/imdb_bar_delim/role_type.csv' delimiter '|' null '';<br/>
copy title from '/var/lib/postgresql/data/imdb_bar_delim/title.csv' delimiter '|' null '';<br/>
copy movie_info from '/var/lib/postgresql/data/imdb_bar_delim/movie_info.csv' delimiter '|' null '';<br/>
copy person_info from '/var/lib/postgresql/data/imdb_bar_delim/person_info.csv' delimiter '|' null '';<br/>

# Import data from a csv into a table in MapD.
copy aka_name from '/home/yizenov/origin_imdb_bar_delim/aka_name.csv' with (delimiter='|', header='false');<br/>
copy aka_title from '/home/yizenov/origin_imdb_bar_delim/aka_title.csv' with (delimiter='|', header='false');<br/>
copy cast_info from '/home/yizenov/origin_imdb_bar_delim/cast_info.csv' with (delimiter='|', header='false');<br/>
copy char_name from '/home/yizenov/origin_imdb_bar_delim/char_name.csv' with (delimiter='|', header='false');<br/>
copy company_name from '/home/yizenov/origin_imdb_bar_delim/company_name.csv' with (delimiter='|', header='false');<br/>
copy company_type from '/home/yizenov/origin_imdb_bar_delim/company_type.csv' with (delimiter='|', header='false');<br/>
copy comp_cast_type from '/home/yizenov/origin_imdb_bar_delim/comp_cast_type.csv' with (delimiter='|', header='false');<br/>
copy complete_cast from '/home/yizenov/origin_imdb_bar_delim/complete_cast.csv' with (delimiter='|', header='false');<br/>
copy info_type from '/home/yizenov/origin_imdb_bar_delim/info_type.csv' with (delimiter='|', header='false');<br/>
copy keyword from '/home/yizenov/origin_imdb_bar_delim/keyword.csv' with (delimiter='|', header='false');<br/>
copy kind_type from '/home/yizenov/origin_imdb_bar_delim/kind_type.csv' with (delimiter='|', header='false');<br/>
copy link_type from '/home/yizenov/origin_imdb_bar_delim/link_type.csv' with (delimiter='|', header='false');<br/>
copy movie_keyword from '/home/yizenov/origin_imdb_bar_delim/movie_keyword.csv' with (delimiter='|', header='false');<br/>
copy movie_link from '/home/yizenov/origin_imdb_bar_delim/movie_link.csv' with (delimiter='|', header='false');<br/>
copy name from '/home/yizenov/origin_imdb_bar_delim/name.csv' with (delimiter='|', header='false');<br/>
copy person_info from '/home/yizenov/origin_imdb_bar_delim/person_info.csv' with (delimiter='|', header='false');<br/>
copy title from '/home/yizenov/origin_imdb_bar_delim/title.csv' with (delimiter='|', header='false');<br/>
copy movie_companies from '/home/yizenov/origin_imdb_bar_delim/movie_companies.csv' with (delimiter='|', header='false');<br/>
copy role_type from '/home/yizenov/origin_imdb_bar_delim/role_type.csv' with (delimiter='|', header='false');<br/>
copy movie_info from '/home/yizenov/origin_imdb_bar_delim/movie_info.csv' with (delimiter='|', header='false');<br/>
copy movie_info_idx from '/home/yizenov/origin_imdb_bar_delim/movie_info_idx.csv' with (delimiter='|', header='false');<br/>
