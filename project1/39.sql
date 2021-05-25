SELECT DISTINCT T.name
FROM CatchedPokemon AS C1,CatchedPokemon AS C2, Trainer AS T
WHERE C1.pid = C2.pid AND C1.owner_id = C2.owner_id AND C1.id <> C2.id
      AND C1.owner_id = T.id
ORDER BY T.name;