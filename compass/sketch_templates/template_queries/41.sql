SELECT COUNT(*)
FROM movie_link AS ml,
	movie_info AS mi
WHERE ml.id > -1
	AND mi.id > -1
	AND ml.movie_id = mi.movie_id;
