SELECT COUNT(*)
FROM movie_companies AS mc,
	company_name AS cn
WHERE mc.id > -1
	AND cn.id > -1
	AND mc.company_id = cn.id;
