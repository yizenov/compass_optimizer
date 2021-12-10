SELECT COUNT(*)
FROM movie_info_idx AS mi_idx,
	movie_info AS mi
WHERE mi_idx.id > -1
	AND mi.id > -1
	AND mi_idx.movie_id = mi.movie_id;
