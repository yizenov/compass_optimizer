SELECT COUNT(*)
FROM movie_keyword AS mk,
	keyword AS k
WHERE mk.id > -1
	AND k.id > -1
	AND mk.keyword_id = k.id;
