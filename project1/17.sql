SELECT COUNT(DISTINCT name) AS 'Kind'
FROM Pokemon AS P JOIN(
  SELECT pid
  FROM CatchedPokemon JOIN(
    SELECT id
    FROM Trainer
    WHERE hometown = 'Sangnok City')
  AS T ON T.id = owner_id)
AS C ON C.pid = P.id;