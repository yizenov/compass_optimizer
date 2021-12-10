SELECT COUNT(*)
FROM name AS n,
	person_info AS pi
WHERE n.id > -1
	AND pi.id > -1
	AND pi.person_id = n.id;
