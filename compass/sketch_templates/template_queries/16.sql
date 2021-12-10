SELECT COUNT(*)
FROM name AS n,
	aka_name AS an
WHERE n.id > -1
	AND an.id > -1
	AND an.person_id = n.id;
