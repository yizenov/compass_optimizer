SELECT COUNT(*)
FROM movie_link AS ml,
	link_type AS lt
WHERE ml.id > -1
	AND lt.id > -1
	AND ml.link_type_id = lt.id;
