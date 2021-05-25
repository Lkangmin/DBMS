SELECT P.name
FROM Evolution AS E, Pokemon AS P
WHERE P.id = E.after_id AND NOT EXISTS(
  SELECT before_id
  FROM Evolution AS E1
  WHERE E1.before_id = P.id)
ORDER BY P.name;