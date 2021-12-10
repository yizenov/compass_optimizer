SELECT COUNT(*)
FROM info_type AS it,
	person_info AS pi
WHERE it.id > -1
	AND pi.id > -1
	AND pi.info_type_id = it.id;
