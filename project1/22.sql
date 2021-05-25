SELECT type,COUNT(*) AS '#Pokemon'
FROM Pokemon
GROUP BY type
ORDER BY COUNT(*),type;