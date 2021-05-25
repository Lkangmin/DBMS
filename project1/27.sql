SELECT name, MAX(level)
FROM CatchedPokemon AS C JOIN Trainer AS T ON C.owner_id = T.id
GROUP BY owner_id
HAVING COUNT(*) >= 4
ORDER BY name;