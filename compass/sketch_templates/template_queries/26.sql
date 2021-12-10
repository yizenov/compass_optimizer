SELECT COUNT(*)
FROM movie_link AS ml,
	movie_info_idx AS mi_idx
WHERE ml.id > -1
	AND mi_idx.id > -1
	AND ml.linked_movie_id = mi_idx.movie_id;
