SELECT COUNT(*)
FROM info_type AS it,
	movie_info_idx AS mi_idx
WHERE it.id > -1
	AND mi_idx.id > -1
	AND mi_idx.info_type_id = it.id;
