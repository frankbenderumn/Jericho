CREATE TABLE "food" (
	id SERIAL PRIMARY KEY, 
	created_at BIGINT NOT NULL,
	updated_at BIGINT NOT NULL,
	calories INT NOT NULL,
	name VARCHAR(16) NOT NULL
);
CREATE TABLE "meal" (
	id SERIAL PRIMARY KEY, 
	created_at BIGINT NOT NULL,
	updated_at BIGINT NOT NULL,
	name VARCHAR(16) NOT NULL
);
CREATE TABLE "portfolio" (
	id SERIAL PRIMARY KEY, 
	created_at BIGINT NOT NULL,
	updated_at BIGINT NOT NULL,
	name VARCHAR(16) NOT NULL
);
CREATE TABLE "todo" (
	id SERIAL PRIMARY KEY, 
	created_at BIGINT NOT NULL,
	updated_at BIGINT NOT NULL,
	name VARCHAR(16) NOT NULL
);
CREATE TABLE "transaction" (
	id SERIAL PRIMARY KEY, 
	created_at BIGINT NOT NULL,
	updated_at BIGINT NOT NULL,
	name VARCHAR(16) NOT NULL
);
CREATE TABLE "user" (
	user_id SERIAL PRIMARY KEY, 
	created_at BIGINT NOT NULL,
	updated_at BIGINT NOT NULL,
	activated BOOL NOT NULL,
	email VARCHAR(50) NOT NULL,
	logged_in BOOL NOT NULL,
	name VARCHAR(50) NOT NULL,
	password UNDEFINED NOT NULL,
	username VARCHAR(15) NOT NULL
);

