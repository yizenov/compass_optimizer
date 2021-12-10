# IMDB dataset details

The dataset that was used is [Internet Movie Data Base (IMDB)](https://www.imdb.com/). The data is [publicly availabe](ftp://ftp.fu-berlin.de/pub/misc/movies/database/) in text files and [open-source imdbpy package](https://bitbucket.org/alberanid/imdbpy/get/5.0.zip) was used to tranform .txt files to .csv files in [1]. See for more details in [here](https://github.com/gregrahn/join-order-benchmark). The 3.6 GB snapshot is from May 2013 and it can be downloaded [here](https://homepages.cwi.nl/~boncz/job/). The dataset includes 21 .csv files i.e. 21 relations in total. It also includes queries to create the necessary relations written in `schema.sql` or `schematext.sql` files.

The IMDB data snapshot initially shared in [here](https://homepages.cwi.nl/~boncz/job/). </br>
In case of issues accessing the dataset, we re-share the original version in [here](https://ucmerced.box.com/s/3hma15ta8h8qb0jvt70f34qyfirqlgn8).

There are two schema shared in public (one with the IMDB snapshot and one with the JOB benchmark).
However, there were differences which we list them here. JOB versus IMDB schemas:

1. aka_name
	- column `name text NOT NULL` changed to `character varying`
	- change in lengths in character varying
2. aka_title
	- column `title text NOT NULL` changed to `character varying`
	- column `note text` changed to `character varying()`
	- change in lengths in character varying
3. cast_info
	- column `note text` changed to `character varying`
4. char_name
	- column `name text NOT NULL` changed to `character varying NOT NULL`
	- change in lengths in character varying
5. company_name
	- column `name: text NOT NULL` changed to `character varying NOT NULL`
	- change in lengths in character varying
6. company_type
	- column `kind NOT NULL`: NOT NULL was removed
7. keyword
	- column `keyword text NOT NULL` changed to `character varying NOT NULL`
8. kind_type
	- column `kind NOT NULL`: NOT NULL was removed
9. movie_companies
	- column `note text` changed to `character varying`
10. movie_info
	- column `info text NOT NULL` changed to `character varying NOT NULL`
	- column `note text` changed to `character varying`
11. movie_info_idx
	- column `info text NOT NULL` changed to `character varying NOT NULL`
	- column `note text` changed to `character varying()`
12. name
	- column `name text NOT NULL` changed to `character varying NOT NULL`
	- change in lengths in character varying
13. person_info
	- column `info text NOT NULL` changed to `character varying NOT NULL`
	- column `note text` changed to `character varying`
14. title
	- column `title text NOT NULL` changed to `character varying NOT NULL`
	- change in lengths in character varying
	
We changed to schema to avoid errors caused in MapD:
- `character varying()` changed to `TEXT ENCODING DICT(32)`
- aka_name.name NOT NULL removed
- aka_title.title NOT NULL removed
- company_type.kind NOT NULL removed
- kind_type.kind NOT NULL removed
- cast_info.person_role_id added
- complete_cast.movie_id added
- comp_cast_type.kind `character varying()` changed to `TEXT`
- info_type.info `character varying()` changed to `TEXT`
- role_type.role_t `character varying()` changed to `TEXT`


We used the IMDB schema version with several changes to match the MapD syntax:
- `character varying()` changed to `TEXT ENCODING DICT(32)`
- `character varying` changed to `TEXT`
- `character varying()` changed to `TEXT` in columns:
  - aka_title.note
  - comp_cast_type.kind
  - info_type.info
  - link_type.link
  - movie_info_idx.note
  - role_type.role_t
- we omit all `PRIMARY KEY`
- table role_type: changed column name `role` to `role_t`. The word `role` is allocated by MapD.

We changed slightly the schema to avoid errors occurred in MapD:
- cast_info.person_role_id: `NOT NULL` added
- complete_cast.movie_id: `NOT NULL` added
There two caused the following error in MapD:
`Error: Check failed: f
E0827 05:13:55.682631 48594 MapDHandler.cpp:2606] Exception: Hash join failed, reason(s): For hash join, both sides must have the same nullability`

We also match the schema for PostgreSQL:
- change all `character varying(any length)` to `character varying(32)`
- change `character varying()` to `character varying` in columns:
  - aka_title.note
  - comp_cast_type.kind
  - info_type.info
  - link_type.link
  - movie_info_idx.note
  - role_type.role_t
- cast_info.person_role_id: `NOT NULL` added
- complete_cast.movie_id: `NOT NULL` added
- table role_type: changed column name `role` to `role_t`.
