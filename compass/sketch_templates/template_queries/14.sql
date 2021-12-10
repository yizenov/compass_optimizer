SELECT COUNT(*)
FROM cast_info AS ci,
	name AS n
WHERE ci.id > -1
	AND n.id > -1
	AND n.id = ci.person_id;
