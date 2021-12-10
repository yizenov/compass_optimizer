SELECT COUNT(*)
FROM comp_cast_type AS cct,
	complete_cast AS cc
WHERE cct.id > -1
	AND cc.id > -1
	AND cct.id = cc.status_id;
