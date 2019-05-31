Drop view if exists norm cascade;
drop table if exists non_norm_q9 cascade;
create view norm as 
select dept, (WeeklySales/size) as normsales 
from hw2.Sales,hw2.Stores where Stores.store=Sales.store;

create view ordered_q8 as select ROW_NUMBER() over (order by null) as row_num, * from (select dept,sum(normsales) from norm group by dept order by sum desc) as foo;


Create view q9 as 
select dept,sum as normsales from ordered_q8 where row_num<=10;

create table non_norm_q9 as 
select sales.dept, sum(weeklySales) 
from hw2.sales,hw2.stores,q9 
where sales.dept=q9.dept and stores.store=sales.store 
group by sales.dept;

create view q91 as select sales.dept, extract (year from Weekdate) as year, extract (month from Weekdate) as month,sum(weeklySales) as monthlySales from hw2.sales,q9 where q9.dept=sales.dept group by sales.dept,year,month order by dept,year,month;

Create view q92 as select q91.*,(monthlySales/sum*100) as contribution from q91,non_norm_q9 where non_norm_q9.dept=q91.dept;


Create view q93 as 
select *, sum(monthlysales) over (partition by dept order by dept,year,month) 
from q92;

select dept,year,month,monthlysales,round(CAST(contribution as numeric),2),round(CAST( sum as numeric),2)from q93;

