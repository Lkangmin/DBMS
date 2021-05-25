SELECT DISTINCT A.name
FROM (SELECT P.name
      FROM Pokemon AS P, CatchedPokemon AS C, Trainer AS T
      WHERE T.hometown = 'Sangnok City' AND C.owner_id = T.id AND C.pid = P.id) AS A,
      (SELECT P.name
      FROM Pokemon AS P, CatchedPokemon AS C, Trainer AS T
      WHERE T.hometown = 'Brown City' AND C.owner_id = T.id AND C.pid = P.id) AS B
WHERE A.name = B.name
ORDER BY A.name;