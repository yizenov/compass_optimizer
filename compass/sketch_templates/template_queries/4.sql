SELECT COUNT(*)
FROM cast_info AS ci,
	char_name AS chn
WHERE ci.id > -1
	AND chn.id > -1
	AND ci.person_role_id = chn.id;
