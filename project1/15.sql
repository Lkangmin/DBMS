SELECT A.owner_id, A.total
FROM (SELECT owner_id, count(*) AS total
      FROM CatchedPokemon
      GROUP BY owner_id) AS A,
     (SELECT COUNT(*) AS total
      FROM CatchedPokemon
      GROUP BY owner_id
      ORDER BY COUNT(*) DESC LIMIT 1) AS B
WHERE A.total = B.total
ORDER BY A.owner_id;