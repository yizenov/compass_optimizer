SELECT COUNT(*)
FROM movie_companies AS mc,
	title AS t
WHERE mc.id > -1
	AND t.id > -1
	AND mc.movie_id = t.id;
