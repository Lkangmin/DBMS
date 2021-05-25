SELECT name, COUNT(*) AS '#Pokemon'
FROM CatchedPokemon AS C JOIN(
  SELECT id, name
  FROM Trainer
  WHERE hometown = 'Sangnok City')
AS T ON C.owner_id = T.id
GROUP BY name
ORDER BY COUNT(*);