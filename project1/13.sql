SELECT name,id
FROM Pokemon AS P JOIN(
  SELECT owner_id, pid
  FROM CatchedPokemon
  JOIN(
    SELECT id
    FROM Trainer
    WHERE hometown = 'Sangnok City')
  AS T 
  ON T.id = owner_id)
 AS C 
 ON C.pid = P.id
ORDER BY id;