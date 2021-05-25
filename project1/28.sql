SELECT T.name, AVG(level) AS Average
FROM Pokemon AS P, CatchedPokemon AS C, Trainer AS T
WHERE T.id = C.owner_id AND C.pid = P.id AND (P.type = 'Normal' OR P.type = 'Electric')
GROUP BY name
ORDER BY AVG(level);