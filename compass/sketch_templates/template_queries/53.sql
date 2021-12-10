SELECT COUNT(*)
FROM movie_link AS ml,
	movie_companies AS mc
WHERE ml.id > -1
	AND mc.id > -1
	AND ml.movie_id = mc.movie_id;
