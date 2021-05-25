SELECT T.name
FROM CatchedPokemon AS C, Trainer AS T
WHERE T.id = C.owner_id
GROUP BY owner_id
HAVING COUNT(*) > 2
ORDER BY COUNT(pid) DESC;