SELECT B.name, MAX(B.sum) AS 'sum'
FROM (SELECT SUM(level) AS sum
      FROM CatchedPokemon AS C, Trainer AS T
      WHERE T.id = C.owner_id
      GROUP BY T.name
      ORDER BY SUM(level) DESC LIMIT 1) AS A,
     (SELECT T.name, SUM(level) AS sum
      FROM CatchedPokemon AS C, Trainer AS T
      WHERE T.id = C.owner_id
      GROUP BY T.name) AS B
WHERE B.sum = A.sum
ORDER BY B.name;