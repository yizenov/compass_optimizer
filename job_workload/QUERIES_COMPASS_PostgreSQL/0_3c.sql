SELECT COUNT(*)
FROM keyword AS k,
    movie_info AS mi,
    movie_keyword AS mk,
    title AS t
WHERE k.keyword like '%sequel%'
    AND mi.info in ('Sweden', 'Norway', 'Germany', 'Denmark', 'Swedish', 'Denish', 'Norwegian', 'German', 'USA', 'American')
    AND t.production_year > 1990

    AND t.id = mi.movie_id
    AND t.id = mk.movie_id
    AND mk.movie_id = mi.movie_id
    AND k.id = mk.keyword_id;
