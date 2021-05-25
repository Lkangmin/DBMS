SELECT DISTINCT name, type
FROM CatchedPokemon AS C JOIN Pokemon AS P ON C.pid = P.id 
WHERE level >= 30
ORDER BY name;