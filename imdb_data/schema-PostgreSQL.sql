CREATE TABLE aka_name (
    id integer NOT NULL PRIMARY KEY,
    person_id integer NOT NULL,
    name character varying,
    imdb_index character varying(32),
    name_pcode_cf character varying(32),
    name_pcode_nf character varying(32),
    surname_pcode character varying(32),
    md5sum character varying(32)
);

CREATE TABLE aka_title (
    id integer NOT NULL PRIMARY KEY,
    movie_id integer NOT NULL,
    title character varying,
    imdb_index character varying(32),
    kind_id integer NOT NULL,
    production_year integer,
    phonetic_code character varying(32),
    episode_of_id integer,
    season_nr integer,
    episode_nr integer,
    note character varying,
    md5sum character varying(32)
);

CREATE TABLE cast_info (
    id integer NOT NULL PRIMARY KEY,
    person_id integer NOT NULL,
    movie_id integer NOT NULL,
    person_role_id integer NOT NULL,
    note character varying,
    nr_order integer,
    role_id integer NOT NULL
);

CREATE TABLE char_name (
    id integer NOT NULL PRIMARY KEY,
    name character varying NOT NULL,
    imdb_index character varying(32),
    imdb_id integer,
    name_pcode_nf character varying(32),
    surname_pcode character varying(32),
    md5sum character varying(32)
);

CREATE TABLE comp_cast_type (
    id integer NOT NULL PRIMARY KEY,
    kind character varying NOT NULL
);

CREATE TABLE company_name (
    id integer NOT NULL PRIMARY KEY,
    name character varying NOT NULL,
    country_code character varying(32),
    imdb_id integer,
    name_pcode_nf character varying(32),
    name_pcode_sf character varying(32),
    md5sum character varying(32)
);

CREATE TABLE company_type (
    id integer NOT NULL PRIMARY KEY,
    kind character varying(32)
);

CREATE TABLE complete_cast (
    id integer NOT NULL PRIMARY KEY,
    movie_id integer NOT NULL,
    subject_id integer NOT NULL,
    status_id integer NOT NULL
);

CREATE TABLE info_type (
    id integer NOT NULL PRIMARY KEY,
    info character varying NOT NULL
);

CREATE TABLE keyword (
    id integer NOT NULL PRIMARY KEY,
    keyword character varying NOT NULL,
    phonetic_code character varying(32)
);

CREATE TABLE kind_type (
    id integer NOT NULL PRIMARY KEY,
    kind character varying(32)
);

CREATE TABLE link_type (
    id integer NOT NULL PRIMARY KEY,
    link character varying NOT NULL
);

CREATE TABLE movie_companies (
    id integer NOT NULL PRIMARY KEY,
    movie_id integer NOT NULL,
    company_id integer NOT NULL,
    company_type_id integer NOT NULL,
    note character varying
);

CREATE TABLE movie_info (
    id integer NOT NULL PRIMARY KEY,
    movie_id integer NOT NULL,
    info_type_id integer NOT NULL,
    info character varying NOT NULL,
    note character varying
);

CREATE TABLE movie_info_idx (
    id integer NOT NULL PRIMARY KEY,
    movie_id integer NOT NULL,
    info_type_id integer NOT NULL,
    info character varying NOT NULL,
    note character varying
);

CREATE TABLE movie_keyword (
    id integer NOT NULL PRIMARY KEY,
    movie_id integer NOT NULL,
    keyword_id integer NOT NULL
);

CREATE TABLE movie_link (
    id integer NOT NULL PRIMARY KEY,
    movie_id integer NOT NULL,
    linked_movie_id integer NOT NULL,
    link_type_id integer NOT NULL
);

CREATE TABLE name (
    id integer NOT NULL PRIMARY KEY,
    name character varying NOT NULL,
    imdb_index character varying(32),
    imdb_id integer,
    gender character varying(32),
    name_pcode_cf character varying(32),
    name_pcode_nf character varying(32),
    surname_pcode character varying(32),
    md5sum character varying(32)
);

CREATE TABLE person_info (
    id integer NOT NULL PRIMARY KEY,
    person_id integer NOT NULL,
    info_type_id integer NOT NULL,
    info character varying NOT NULL,
    note character varying
);

CREATE TABLE role_type (
    id integer NOT NULL PRIMARY KEY,
    role_t character varying NOT NULL
);

CREATE TABLE title (
    id integer NOT NULL PRIMARY KEY,
    title character varying NOT NULL,
    imdb_index character varying(32),
    kind_id integer NOT NULL,
    production_year integer,
    imdb_id integer,
    phonetic_code character varying(32),
    episode_of_id integer,
    season_nr integer,
    episode_nr integer,
    series_years character varying(32),
    md5sum character varying(32)
);
