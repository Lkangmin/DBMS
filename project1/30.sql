SELECT P1.id, P1.name AS '1 Evolution', P2.name AS '2 Evolution', P3.name AS '3 Evolution'
FROM Pokemon AS P1, Pokemon AS P2, Pokemon AS P3, Evolution AS E1, Evolution AS E2
WHERE P1.id = E1.before_id AND E1.after_id = P2.id AND P2.id = E2.before_id AND E2.after_id = P3.id;