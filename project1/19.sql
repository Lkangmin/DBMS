SELECT COUNT(DISTINCT type) AS '#Type'
FROM Pokemon AS P JOIN(
  SELECT pid
  FROM CatchedPokemon JOIN(
    SELECT leader_id
    FROM Gym
    WHERE city = 'Sangnok City')
  AS G ON G.leader_id = owner_id)
AS C ON C.pid = P.id;