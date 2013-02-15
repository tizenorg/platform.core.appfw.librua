PRAGMA journal_mode = PERSIST;

CREATE TABLE IF NOT EXISTS rua_history (
	id INTEGER PRIMARY KEY,
	pkg_name TEXT,
	app_path TEXT,
	arg TEXT,
	launch_time INTEGER
);
