# COMPASS: Online Sketch-based Query Optimization for In-Memory Databases

This work has been published at [SIGMOD 2021](https://dl.acm.org/doi/abs/10.1145/3448016.3452840).
The extended version is available in the [arXiv.org](https://arxiv.org/abs/2102.02440).

# Sections
[Section 1](https://github.com/yizenov/compass_optimizer/tree/master/compass). COMPASS has been implemented in [MapD System](https://github.com/omnisci/omniscidb), version 3.6.1 which later re-branded to [OmniSciDB](https://www.omnisci.com/). </br>

[Section 2](https://github.com/yizenov/compass_optimizer/tree/master/imdb_data). A snapshot of [Internet Movie Data Base (IMDB)](https://www.imdb.com/) from May 2013 has been used in the evaluation. </br>

[Section 3](https://github.com/yizenov/compass_optimizer/tree/master/job_workload). COMPASS and four other well-known database systems have been evaluated on [Join Order Benchmark(JOB)](http://www-db.in.tum.de/~leis/qo/job.tgz). The JOB consists of 113 queries including 33 query families that each family differ only in selection predicates. </br>

# Contacts
If you have questions, please contact:
- Yesdaulet Izenov [yizenov@ucmerced.edu], (https://yizenov.github.io/yizenov/)
- Asoke Datta [adatta2@ucmerced.edu], (https://asoke26.github.io/adatta2/)
- Florin Rusu [frusu@ucmerced.edu], (https://faculty.ucmerced.edu/frusu/)

# References:
1.  [Query optimization through the looking glass, and what we found running the Join Order Benchmark](https://doi.org/10.1007/s00778-017-0480-7)
2.  [Filter Push-Down extension for MapD](https://github.com/junhyungshin/mapd-core-fpd)
3.  [OmniSci (MapD)](https://www.omnisci.com)
4.  [PostgreSQL](https://www.postgresql.org)
