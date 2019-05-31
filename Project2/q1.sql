Drop view if exists holsales cascade;
CREATE VIEW holsales AS SELECT Store, WeeklySales FROM hw2.sales, hw2.holidays WHERE (hw2.Sales.WeekDate=hw2.Holidays.WeekDate and Holidays.IsHoliday='t');

CREATE VIEW holsumsales AS SELECT Store, SUM(WeeklySales) AS hol_sales FROM holsales GROUP BY Store;

SELECT store, hol_sales FROM holsumsales WHERE hol_sales=(select max(hol_sales) FROM holsumsales ) UNION SELECT store, hol_sales FROM holsumsales WHERE hol_sales=(SELECT MIN(hol_sales) FROM holsumsales );

