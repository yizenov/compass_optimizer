SELECT COUNT(*)
FROM complete_cast AS cc,
	title AS t
WHERE cc.id > -1
	AND t.id > -1
	AND cc.movie_id = t.id;
