SELECT COUNT(*)
FROM complete_cast AS cc,
	movie_link AS ml
WHERE cc.id > -1
	AND ml.id > -1
	AND cc.movie_id = ml.movie_id;
