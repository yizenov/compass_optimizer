SELECT COUNT(*)
FROM movie_info_idx AS mi_idx,
	movie_companies AS mc
WHERE mi_idx.id > -1
	AND mc.id > -1
	AND mi_idx.movie_id = mc.movie_id;
