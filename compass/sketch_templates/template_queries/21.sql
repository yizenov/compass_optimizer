SELECT COUNT(*)
FROM aka_title AS att,
	movie_companies AS mc
WHERE att.id > -1
	AND mc.id > -1
	AND att.movie_id = mc.movie_id;
