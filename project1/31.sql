SELECT P.type
FROM Pokemon AS P, Evolution AS E
WHERE P.id = E.before_id
GROUP BY P.type
HAVING COUNT(*) >= 3
ORDER BY P.type DESC;