SELECT COUNT(*)
FROM movie_keyword AS mk,
	movie_info_idx AS mi_idx
WHERE mk.id > -1
	AND mi_idx.id > -1
	AND mk.movie_id = mi_idx.movie_id;
