SELECT P.name
FROM Evolution AS E, Pokemon AS P
WHERE E.before_id = P.id AND P.type = 'Grass';