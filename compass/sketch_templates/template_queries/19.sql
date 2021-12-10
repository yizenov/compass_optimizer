SELECT COUNT(*)
FROM aka_title AS att,
	movie_info AS mi
WHERE att.id > -1
	AND mi.id > -1
	AND att.movie_id = mi.movie_id;
