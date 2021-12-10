SELECT COUNT(*)
FROM movie_keyword AS mk,
	title AS t
WHERE mk.id > -1
	AND t.id > -1
	AND mk.movie_id = t.id;
