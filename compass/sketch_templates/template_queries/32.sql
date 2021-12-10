SELECT COUNT(*)
FROM movie_info AS mi,
	title AS t
WHERE mi.id > -1
	AND t.id > -1
	AND mi.movie_id = t.id;
