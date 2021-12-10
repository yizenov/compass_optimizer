SELECT COUNT(*)
FROM movie_keyword AS mk,
	movie_info AS mi
WHERE mk.id > -1
	AND mi.id > -1
	AND mk.movie_id = mi.movie_id;
