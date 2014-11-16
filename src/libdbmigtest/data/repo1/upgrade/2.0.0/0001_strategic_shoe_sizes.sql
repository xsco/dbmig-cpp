-- Strategic solution to shoe sizes.

create table human_shoe_size
(
    human_shoe_size_id serial,
    human_id int,
    shoe_size_jurisdiction char(2), -- e.g. UK/EU/US
    numeric_shoe_size int,
    
    primary key(human_shoe_size_id),
    foreign key(human_id) references human(human_id)
);

--//@UNDO

drop table human_shoe_size;

