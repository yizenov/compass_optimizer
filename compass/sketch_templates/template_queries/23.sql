SELECT COUNT(*)
FROM movie_link AS ml,
	title AS t
WHERE ml.id > -1
	AND t.id > -1
	AND ml.linked_movie_id = t.id;
