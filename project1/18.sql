SELECT AVG(level) AS 'Average'
FROM CatchedPokemon AS C JOIN Gym AS G ON C.owner_id = G.leader_id;