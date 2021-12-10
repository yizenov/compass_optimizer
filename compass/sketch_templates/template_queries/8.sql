SELECT COUNT(*)
FROM title AS t,
	kind_type AS kt
WHERE t.id > -1
	AND kt.id > -1
	AND t.kind_id = kt.id;
