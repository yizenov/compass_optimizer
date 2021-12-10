SELECT COUNT(*)
FROM movie_info_idx AS mi_idx,
	movie_link AS ml
WHERE mi_idx.id > -1
	AND ml.id > -1
	AND mi_idx.movie_id = ml.movie_id;
