SELECT P.name, C.level, C.nickname
FROM CatchedPokemon AS C, Gym AS G, Pokemon AS P
WHERE C.owner_id = G.leader_id AND C.nickname LIKE 'A%' AND C.pid = P.id
ORDER BY P.name DESC;