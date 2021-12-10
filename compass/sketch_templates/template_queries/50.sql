SELECT COUNT(*)
FROM complete_cast AS cc,
	cast_info AS ci
WHERE cc.id > -1
	AND ci.id > -1
	AND cc.movie_id = ci.movie_id;
