SELECT COUNT(*)
FROM complete_cast AS cc,
	movie_info AS mi
WHERE cc.id > -1
	AND mi.id > -1
	AND cc.movie_id = mi.movie_id;
