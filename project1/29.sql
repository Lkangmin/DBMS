SELECT P.type, COUNT(*) AS '#Pokemon'
FROM Pokemon AS P, CatchedPokemon AS C
WHERE P.id = C.pid
GROUP BY P.type
ORDER BY P.type;