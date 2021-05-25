SELECT name
FROM Pokemon AS P JOIN CatchedPokemon AS C ON P.id = C.pid
WHERE nickname LIKE '% %'
ORDER BY name DESC;