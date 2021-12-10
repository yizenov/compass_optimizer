SELECT COUNT(*)
FROM info_type AS it,
	movie_info AS mi
WHERE it.id > -1
	AND mi.id > -1
	AND mi.info_type_id = it.id;
