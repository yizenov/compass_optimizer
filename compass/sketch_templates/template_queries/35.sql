SELECT COUNT(*)
FROM movie_keyword AS mk,
	movie_link AS ml
WHERE mk.id > -1
	AND ml.id > -1
	AND mk.movie_id = ml.movie_id;
