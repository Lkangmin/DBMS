SELECT name, COUNT(*) AS '#Pokemon'
FROM CatchedPokemon AS C JOIN(
  SELECT id,name
  FROM Trainer JOIN Gym ON id = leader_id)  
AS T ON C.owner_id = T.id
GROUP BY name
ORDER BY name;