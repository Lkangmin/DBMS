SELECT DISTINCT name
FROM CatchedPokemon AS C JOIN(
  SELECT id,name
  FROM Trainer)
AS T ON C.owner_id = T.id
WHERE level <= 10
ORDER by name;
