DROP VIEW IF EXISTS q4 CASCADE;
DROP VIEW IF EXISTS typesum; 

CREATE VIEW q4 AS SELECT stores.store, type, weekdate,weeklysales FROM hw2.stores, hw2.sales WHERE stores.store = sales.store;
CREATE VIEW typesum AS SELECT type, sum(weeklySales) FROM q4 GROUP BY type ORDER BY type;
CREATE VIEW bymonth AS SELECT extract(month from weekdate) AS months, type, sum(weeklysales) FROM q4 GROUP BY months, type ORDER BY TYPE, months;
SELECT months, bymonth.type, bymonth.sum, ((bymonth.sum/typesum.sum)*100) AS "%contribution" FROM bymonth, typesum WHERE bymonth.type = typesum.type;

