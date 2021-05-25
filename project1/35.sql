SELECT P.name
FROM Pokemon AS P,Evolution AS E
WHERE P.id = E.before_id AND E.before_id > E.after_id
ORDER BY P.name;