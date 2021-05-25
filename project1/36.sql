SELECT T.name
FROM Evolution AS E1, Trainer AS T, CatchedPokemon AS C 
WHERE C.pid = E1.after_id AND T.id = C.owner_id AND NOT EXISTS(
   SELECT before_id
   FROM Evolution AS E2
   WHERE C.pid = E2.before_id)
ORDER BY T.name;