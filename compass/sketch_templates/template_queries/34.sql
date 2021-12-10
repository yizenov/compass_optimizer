SELECT COUNT(*)
FROM movie_keyword AS mk,
	movie_companies AS mc
WHERE mk.id > -1
	AND mc.id > -1
	AND mk.movie_id = mc.movie_id;
