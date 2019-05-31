DROP VIEW IF EXISTS q5 CASCADE;
DROP VIEW IF EXISTS checkyear; 
CREATE VIEW q5 AS SELECT store,dept, EXTRACT(month from WeekDate) as month, EXTRACT(year from Weekdate) as year,SUM(weeklysales) FROM hw2.sales GROUP BY store,dept,year,month ORDER BY year,month,store,dept;

create view checkyear as select distinct extract(year from weekdate) as year,store from hw2.sales;


CREATE VIEW maxdept AS SELECT store, MAX(dept) AS maxd FROM q5 GROUP BY store ORDER BY store;

CREATE VIEW newtable AS SELECT q5.store,dept,month,q5.year FROM q5,checkyear WHERE q5.store = checkyear.store and q5.year = checkyear.year and sum >0 order by store, dept, month;
CREATE view exceptdept as select store,dept,count(month) from q5 group by store,dept;  

SELECT DISTINCT exceptdept.store FROM exceptdept,(select store,count(distinct dept), AVG(count) FROM exceptdept group by store) as month, maxdept where month.avg = 12 and month.count = maxdept.maxd

