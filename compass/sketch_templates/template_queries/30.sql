SELECT COUNT(*)
FROM cast_info AS ci,
	title AS t
WHERE ci.id > -1
	AND t.id > -1
	AND ci.movie_id = t.id;
