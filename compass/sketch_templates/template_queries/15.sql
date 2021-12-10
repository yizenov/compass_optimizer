SELECT COUNT(*)
FROM cast_info AS ci,
	person_info AS pi
WHERE ci.id > -1
	AND pi.id > -1
	AND pi.person_id = ci.person_id;
