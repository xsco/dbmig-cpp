--
-- Install version 1.0.0 of the repo1 example database to describe humans.
--

create table human
(
    human_id serial,
    full_name varchar(255),
    gender char(1),
    
    primary key (human_id)
);

