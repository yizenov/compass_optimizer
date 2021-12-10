SELECT COUNT(*)
FROM movie_keyword AS mk,
	complete_cast AS cc
WHERE mk.id > -1
	AND cc.id > -1
	AND mk.movie_id = cc.movie_id;
