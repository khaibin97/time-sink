DROP VIEW IF EXISTS saledata cascade;
CREATE VIEW saledata AS SELECT Weekdate, SUM(weeklySales) FROM hw2.sales GROUP BY Weekdate;

CREATE VIEW sumsales AS SELECT saledata.WeekDate, sum FROM saledata,hw2.holidays WHERE holidays.weekdate=saledata.weekdate and holidays.isholiday='t';

CREATE VIEW sumavg AS SELECT avg(sum) FROM sumsales;

SELECT count(*) FROM saledata, sumavg,hw2.Holidays WHERE sum>avg AND saledata.weekdate=holidays.weekdate AND isholiday='f';

