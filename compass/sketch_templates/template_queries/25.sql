SELECT COUNT(*)
FROM movie_link AS ml,
	cast_info AS ci
WHERE ml.id > -1
	AND ci.id > -1
	AND ml.linked_movie_id = ci.movie_id;
