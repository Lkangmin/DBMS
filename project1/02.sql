SELECT P.name
FROM Pokemon AS P,(
  SELECT COUNT(*) AS num
  FROM Pokemon
  GROUP BY type
  ORDER BY COUNT(*) DESC LIMIT 2) AS A,
 (SELECT COUNT(*) AS num, type
  FROM Pokemon
  GROUP BY type) AS B
WHERE A.num = B.num AND B.type = P.type
ORDER BY P.name;