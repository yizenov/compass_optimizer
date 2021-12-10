SELECT COUNT(*)
FROM movie_info_idx AS mi_idx,
	title AS t
WHERE mi_idx.id > -1
	AND t.id > -1
	AND mi_idx.movie_id = t.id;
