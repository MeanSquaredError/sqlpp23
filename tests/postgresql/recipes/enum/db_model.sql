
DROP TYPE IF EXISTS shape_type CASCADE;

CREATE TYPE shape_type AS ENUM ('circle', 'square', 'triangle');

DROP TABLE IF EXISTS tab_enums;

CREATE TABLE tab_enums (
  animal bigint,
  -- cpp_type:shape_type
  shape shape_type
);
