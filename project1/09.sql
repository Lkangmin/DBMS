SELECT name, AVG(level) AS Average
FROM CatchedPokemon AS C JOIN (
  SELECT id,name
  FROM Trainer AS T1 JOIN Gym AS G ON T1.id = G.leader_id)
  AS T2 ON C.owner_id = T2.id
GROUP BY name 
ORDER BY name;