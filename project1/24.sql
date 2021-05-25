SELECT hometown AS 'City', AVG(level) AS 'Average'
FROM CatchedPokemon AS C JOIN(
  SELECT id, hometown
  FROM Trainer)
AS T ON C.owner_id = T.id
GROUP BY hometown
ORDER BY AVG(level);