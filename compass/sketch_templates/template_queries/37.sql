SELECT COUNT(*)
FROM movie_keyword AS mk,
	cast_info AS ci
WHERE mk.id > -1
	AND ci.id > -1
	AND mk.movie_id = ci.movie_id;
