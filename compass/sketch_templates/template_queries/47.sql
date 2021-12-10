SELECT COUNT(*)
FROM movie_info_idx AS mi_idx,
	complete_cast AS cc
WHERE mi_idx.id > -1
	AND cc.id > -1
	AND mi_idx.movie_id = cc.movie_id;
