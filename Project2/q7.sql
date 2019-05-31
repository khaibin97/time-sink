Drop view if exists q7 cascade;
create view q7 as select sales.store, dept,sum(weeklysales)as sales,store_sum 
from hw2.sales 
join (select store,sum(weeklysales) as store_sum from hw2.sales group by store) as foo on sales.store = foo.store 
	group by dept,sales.store,store_sum 
	order by sales.store, dept;
select foo.dept,avg(sales/store_sum) 
from q7,(select distinct dept from q7 where (sales/store_sum)*100 >= 5 group by dept having count(dept)>3)as foo where foo.dept = q7.dept group by foo.dept;

