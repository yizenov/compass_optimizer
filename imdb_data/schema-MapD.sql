CREATE TABLE aka_name (
    id integer NOT NULL PRIMARY KEY,
    person_id integer NOT NULL,
    name TEXT,
    imdb_index TEXT ENCODING DICT(32),
    name_pcode_cf TEXT ENCODING DICT(32),
    name_pcode_nf TEXT ENCODING DICT(32),
    surname_pcode TEXT ENCODING DICT(32),
    md5sum TEXT ENCODING DICT(32)
);

CREATE TABLE aka_title (
    id integer NOT NULL PRIMARY KEY,
    movie_id integer NOT NULL,
    title TEXT,
    imdb_index TEXT ENCODING DICT(32),
    kind_id integer NOT NULL,
    production_year integer,
    phonetic_code TEXT ENCODING DICT(32),
    episode_of_id integer,
    season_nr integer,
    episode_nr integer,
    note TEXT,
    md5sum TEXT ENCODING DICT(32)
);

CREATE TABLE cast_info (
    id integer NOT NULL PRIMARY KEY,
    person_id integer NOT NULL,
    movie_id integer NOT NULL,
    person_role_id integer NOT NULL,
    note TEXT,
    nr_order integer,
    role_id integer NOT NULL
);

CREATE TABLE char_name (
    id integer NOT NULL PRIMARY KEY,
    name TEXT NOT NULL,
    imdb_index TEXT ENCODING DICT(32),
    imdb_id integer,
    name_pcode_nf TEXT ENCODING DICT(32),
    surname_pcode TEXT ENCODING DICT(32),
    md5sum TEXT ENCODING DICT(32)
);

CREATE TABLE comp_cast_type (
    id integer NOT NULL PRIMARY KEY,
    kind TEXT NOT NULL
);

CREATE TABLE company_name (
    id integer NOT NULL PRIMARY KEY,
    name TEXT NOT NULL,
    country_code TEXT ENCODING DICT(32),
    imdb_id integer,
    name_pcode_nf TEXT ENCODING DICT(32),
    name_pcode_sf TEXT ENCODING DICT(32),
    md5sum TEXT ENCODING DICT(32)
);

CREATE TABLE company_type (
    id integer NOT NULL PRIMARY KEY,
    kind TEXT ENCODING DICT(32)
);

CREATE TABLE complete_cast (
    id integer NOT NULL PRIMARY KEY,
    movie_id integer NOT NULL,
    subject_id integer NOT NULL,
    status_id integer NOT NULL
);

CREATE TABLE info_type (
    id integer NOT NULL PRIMARY KEY,
    info TEXT NOT NULL
);

CREATE TABLE keyword (
    id integer NOT NULL PRIMARY KEY,
    keyword TEXT NOT NULL,
    phonetic_code TEXT ENCODING DICT(32)
);

CREATE TABLE kind_type (
    id integer NOT NULL PRIMARY KEY,
    kind TEXT ENCODING DICT(32)
);

CREATE TABLE link_type (
    id integer NOT NULL PRIMARY KEY,
    link TEXT NOT NULL
);

CREATE TABLE movie_companies (
    id integer NOT NULL PRIMARY KEY,
    movie_id integer NOT NULL,
    company_id integer NOT NULL,
    company_type_id integer NOT NULL,
    note TEXT
);

CREATE TABLE movie_info (
    id integer NOT NULL PRIMARY KEY,
    movie_id integer NOT NULL,
    info_type_id integer NOT NULL,
    info TEXT NOT NULL,
    note TEXT
);

CREATE TABLE movie_info_idx (
    id integer NOT NULL PRIMARY KEY,
    movie_id integer NOT NULL,
    info_type_id integer NOT NULL,
    info TEXT NOT NULL,
    note TEXT
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
    name TEXT NOT NULL,
    imdb_index TEXT ENCODING DICT(32),
    imdb_id integer,
    gender TEXT ENCODING DICT(32),
    name_pcode_cf TEXT ENCODING DICT(32),
    name_pcode_nf TEXT ENCODING DICT(32),
    surname_pcode TEXT ENCODING DICT(32),
    md5sum TEXT ENCODING DICT(32)
);

CREATE TABLE person_info (
    id integer NOT NULL PRIMARY KEY,
    person_id integer NOT NULL,
    info_type_id integer NOT NULL,
    info TEXT NOT NULL,
    note TEXT
);

CREATE TABLE role_type (
    id integer NOT NULL PRIMARY KEY,
    role_t TEXT NOT NULL
);

CREATE TABLE title (
    id integer NOT NULL PRIMARY KEY,
    title TEXT NOT NULL,
    imdb_index TEXT ENCODING DICT(32),
    kind_id integer NOT NULL,
    production_year integer,
    imdb_id integer,
    phonetic_code TEXT ENCODING DICT(32),
    episode_of_id integer,
    season_nr integer,
    episode_nr integer,
    series_years TEXT ENCODING DICT(32),
    md5sum TEXT ENCODING DICT(32)
);
