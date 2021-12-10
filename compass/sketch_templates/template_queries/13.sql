SELECT COUNT(*)
FROM cast_info AS ci,
	aka_name AS an
WHERE ci.id > -1
	AND an.id > -1
	AND an.person_id = ci.person_id;
