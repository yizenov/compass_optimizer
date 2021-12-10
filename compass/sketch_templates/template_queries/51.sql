SELECT COUNT(*)
FROM complete_cast AS cc,
	movie_companies AS mc
WHERE cc.id > -1
	AND mc.id > -1
	AND cc.movie_id = mc.movie_id;
