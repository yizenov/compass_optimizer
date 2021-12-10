SELECT COUNT(*)
FROM aka_title AS att,
	title AS t
WHERE att.id > -1
	AND t.id > -1
	AND att.movie_id = t.id;
