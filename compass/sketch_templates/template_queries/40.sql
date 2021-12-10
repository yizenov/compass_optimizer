SELECT COUNT(*)
FROM movie_companies AS mc,
	movie_info AS mi
WHERE mc.id > -1
	AND mi.id > -1
	AND mc.movie_id = mi.movie_id;
