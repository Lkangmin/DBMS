SELECT A.hometown AS 'City', A.nickname
FROM (SELECT T.hometown, C.nickname, C.level
      FROM Trainer As T, CatchedPokemon AS C
      WHERE T.id = C.owner_id) AS A,
     (SELECT T.hometown, MAX(C.level) AS level
      FROM Trainer AS T, CatchedPokemon AS C
      WHERE T.id = C.owner_id
      GROUP BY T.hometown) AS B
WHERE A.hometown = B.hometown AND A.level = B.level
ORDER BY A.hometown;