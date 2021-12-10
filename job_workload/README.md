# JOB workload details

The JOB workload initially shared in [here](http://www-db.in.tum.de/˜leis/qo/job.tgz). </br>
The link now has the access restriction but we re-share the original workload in [here](https://ucmerced.box.com/s/axn22onytx9u2ubkq9zmng5bcens3e6u).

There are 113 queries clustered into 33 families.
There are queries that have:
- cycles
- at most two instances of the same table used in a single query

We changed some queries in order to avoid the following issues:
- queries with empty result
- match the base table selection results between MapD and PostgreSQL
- syntax errors in MapD

Basic changes:
- projection: replaced all `MIN` aggregations to a single `COUNT` aggregation
- family 32: remove the duplicate join predicate between `mk.movie_id` and `t1.id`
- changed the attribute name `rt.role` to `rt.role_t` because of the schema change on `rt`:
  - affected queries: 8abcd, 9abcd, 10ab, 19abcd, 24ab, 29abc
- change in table alias for clarity and avoid MapD syntax errors:
  - 8ad: `an1` to `an`, `n1` to `n`
  - 8c: `a1` to `an`, `n1` to `n`
  - 13abcd: `it` to `it1`, `miidx` to `mi_idx`
  - 15abcd: `at` to `att`, `it1` to `it`
  - 23abc: `cct1` to `cct`, `it1` to `it`
  - 26abc: `it2` to `it`
  - 29abc: `it` to `it1`, `it3` to `it2`

Changes to match the selection results between MapD and PostgreSQL:
- attribute `mi_idx.info`: change operator `>` to `=` 
  - affected queries: 4abc, 12ac, 14abc, 18b, 22abcd, 26ab, 28abc, 33abc
- attribute `n.name`: change sign `'A%'` to `'%A%'`
  - affected queries: 7c, 17a
- attribute `an.name`: change sign `'A%'` to `'%A%'`
  - affected queries: 7c
- attribute `mi.info` like `'Japan:%2007%'` to `'%Japan:%2007%'`, and `'USA:%200%'` to `'%USA:%200%'`
  - affected queries: 19abc, 24ab, 29abc
- attribute `cn.country_code`: change operator `not like '[pl]'` to `not in ('[pl]')`
  - affected queries: 11abcd, 21abc, 27abc, 28ab

Changes due to the error and selection result mismatch:
- attribute `cn.country_code`: change operator `!=` to `<>` to `=`
  - affected queries: 11abcd, 21abc, 22abcd, 27abc, 28abc, 33c
- attribute `ct.kind`: change operator `!=` to `<>` to `=`
  - affected queries: 11cd
- attribute `t.title`: change operator `!=` to `<>`
  - affected queries: 13bc
- attribute `cct2.kind`: change operator `!=` to `<>` to `=`
  - affected queries: 28ab
The queries above had the following error: `Bang equal '!=' is not allowed under the current SQL conformance level`.

Changes due to the error `ERROR:  canceling statement due to statement timeout`:
- 11c_cn: remove cn.country_code = '[pl]'
- COMPASS long queries: 24a 15min, 29c 20min

Changes dur to the error: `codegen: Both operands to a binary operator are not of the same type!`
- 33ac

Other changes:
- attribute `cn.country_code`: change operator `= '[pl]'` to `not like '[pl]'`
  - affected queries: 11abcd, 21abc, 27abc, 28ab
- attribute `ct.kind`: change operator `= 'production companies'` to `not like 'production companies'`
  - affected queries: 11c
- attribute `cct2.kind`: change operator `= 'complete+verified'` to `not like 'complete+verified'`
  - affected queries: 28ab
