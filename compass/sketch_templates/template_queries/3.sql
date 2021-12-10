SELECT COUNT(*)
FROM cast_info AS ci,
	role_type AS rt
WHERE ci.id > -1
	AND rt.id > -1
	AND ci.role_id = rt.id;
