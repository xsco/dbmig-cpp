-- Remediate the tactical solution to shoe sizes.

insert into human_shoe_size (
       human_id, shoe_size_jurisdiction, numeric_shoe_size)
select human_id, 'uk', uk_shoe_size
from human;

alter table human drop uk_shoe_size;

--//@UNDO

alter table human
add uk_shoe_size int null;

-- TODO - maybe restore from human_shoe_size table?

