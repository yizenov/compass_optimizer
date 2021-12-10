SELECT COUNT(*)
FROM person_info AS pi,
	aka_name AS an
WHERE pi.id > -1
	AND an.id > -1
	AND an.person_id = pi.person_id;
