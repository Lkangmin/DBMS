SELECT SUM(level)
FROM CatchedPokemon AS C, Trainer AS T
WHERE T.name = 'Matis' AND T.id = C.owner_id;