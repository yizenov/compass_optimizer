SELECT COUNT(*)
FROM aka_title AS att,
	movie_keyword AS mk
WHERE att.id > -1
	AND mk.id > -1
	AND att.movie_id = mk.movie_id;
