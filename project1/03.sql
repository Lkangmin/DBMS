SELECT AVG(level)
FROM Pokemon AS P, CatchedPokemon AS C INNER JOIN (
  SELECT id
  FROM Trainer
  WHERE hometown = 'Sangnok City'
  ) AS T ON T.id = C.owner_id
WHERE P.type = 'Electric' AND C.pid = P.id;