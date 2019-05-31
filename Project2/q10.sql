DROP TABLE IF EXISTS qten CASCADE;
drop view if exists q11 cascade; 
CREATE TABLE qten AS SELECT extract(year from weekdate) AS yr, extract(quarter from weekdate) qtr, type, weeklysales FROM hw2.stores,hw2.sales WHERE stores.store = sales.store;

CREATE VIEW q11 AS SELECT yr,qtr,type,SUM(weeklysales) FROM qten GROUP BY type,qtr,yr ORDER BY yr,qtr,type;

SELECT DISTINCT q11.yr,q11.qtr,foo1.sum AS store_a_sales, foo2.sum AS store_b_sales, foo3.sum AS store_c_sales FROM q11,(SELECT yr,qtr,sum FROM q11 WHERE type = 'A')AS foo1,(SELECT yr,qtr,sum FROM q11 WHERE type = 'B')AS foo2,(SELECT yr,qtr,sum FROM q11 WHERE type = 'C')AS foo3 WHERE q11.yr=foo1.yr AND q11.qtr=foo1.qtr AND q11.yr=foo2.yr AND q11.qtr=foo2.qtr AND q11.yr=foo3.yr AND q11.qtr=foo3.qtr GROUP BY q11.qtr,q11.yr,store_a_sales,store_b_sales,store_c_sales ORDER BY q11.yr,q11.qtr, store_a_sales, store_b_sales, store_c_sales;

