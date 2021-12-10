SELECT COUNT(*)
FROM movie_companies AS mc,
	cast_info AS ci
WHERE mc.id > -1
	AND ci.id > -1
	AND mc.movie_id = ci.movie_id;
