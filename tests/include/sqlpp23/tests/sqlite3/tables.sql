DROP TABLE IF EXISTS tab_foo;

CREATE TABLE tab_foo
(
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  text_nn_d varchar(255) NOT NULL DEFAULT '',
  int_n bigint,
  double_n double precision,
  u_int_n bigint UNSIGNED,
  bool_n bool,
  blob_n blob
);

DROP TABLE IF EXISTS tab_bar;

CREATE TABLE tab_bar
(
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  text_n varchar(255) NULL,
  bool_nn bool NOT NULL DEFAULT false,
  int_n int
);

DROP TABLE IF EXISTS tab_date_time;

CREATE TABLE tab_date_time (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  day_point_n date,
  time_point_n datetime(3),
  date_time_point_n_d datetime DEFAULT CURRENT_TIMESTAMP,
  time_of_day_n time(3)
)

