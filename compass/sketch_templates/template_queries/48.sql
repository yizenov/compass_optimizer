SELECT COUNT(*)
FROM movie_info_idx AS mi_idx,
	cast_info AS ci
WHERE mi_idx.id > -1
	AND ci.id > -1
	AND mi_idx.movie_id = ci.movie_id;
