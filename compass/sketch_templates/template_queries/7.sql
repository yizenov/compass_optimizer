SELECT COUNT(*)
FROM movie_companies AS mc,
	company_type AS ct
WHERE mc.id > -1
	AND ct.id > -1
	AND mc.company_type_id = ct.id;
