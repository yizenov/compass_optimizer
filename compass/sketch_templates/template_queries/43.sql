SELECT COUNT(*)
FROM cast_info AS ci,
	movie_info AS mi
WHERE ci.id > -1
	AND mi.id > -1
	AND ci.movie_id = mi.movie_id;
