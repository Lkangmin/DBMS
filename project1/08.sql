SELECT AVG(level) AS Average
FROM Trainer AS T JOIN CatchedPokemon AS C ON T.id = C.owner_id
WHERE name = 'Red';