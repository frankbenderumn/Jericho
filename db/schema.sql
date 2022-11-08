CREATE TABLE "food" (
	id SERIAL PRIMARY KEY, 
	created_at BIGINT DEFAULT extract(epoch from now()) NOT NULL,
	updated_at BIGINT DEFAULT extract(epoch from now()) NOT NULL,
	"calories" INT NOT NULL,
	"name" VARCHAR(16) NOT NULL
);
CREATE TABLE "ohlc" (
	id SERIAL PRIMARY KEY, 
	created_at BIGINT DEFAULT extract(epoch from now()) NOT NULL,
	updated_at BIGINT DEFAULT extract(epoch from now()) NOT NULL,
	"close" DECIMAL NOT NULL,
	"from" VARCHAR(255) NOT NULL,
	"high" DECIMAL NOT NULL,
	"low" DECIMAL NOT NULL,
	"open" DECIMAL NOT NULL,
	"preMarket" DECIMAL NOT NULL,
	"symbol" VARCHAR(255) NOT NULL,
	"volume" BIGINT NOT NULL
);
CREATE TABLE "stock" (
	id SERIAL PRIMARY KEY, 
	created_at BIGINT DEFAULT extract(epoch from now()) NOT NULL,
	updated_at BIGINT DEFAULT extract(epoch from now()) NOT NULL,
	"cik" BIGINT NOT NULL,
	"composite_figi" VARCHAR(255) NOT NULL,
	"last_updated_at" VARCHAR(255) NOT NULL,
	"locale" VARCHAR(255) NOT NULL,
	"market" VARCHAR(255) NOT NULL,
	"name" VARCHAR(255) NOT NULL,
	"primary_exchange" VARCHAR(255) NOT NULL,
	"share_class_figi" VARCHAR(255) NOT NULL,
	"ticker" VARCHAR(255) NOT NULL,
	"type" VARCHAR(255) NOT NULL
);
CREATE TABLE "ticker" (
	ticker VARCHAR(255) PRIMARY KEY, 
	created_at BIGINT DEFAULT extract(epoch from now()) NOT NULL,
	updated_at BIGINT DEFAULT extract(epoch from now()) NOT NULL,
	"name" VARCHAR(255) NOT NULL
);
CREATE TABLE "user" (
	id SERIAL PRIMARY KEY, 
	created_at BIGINT DEFAULT extract(epoch from now()) NOT NULL,
	updated_at BIGINT DEFAULT extract(epoch from now()) NOT NULL,
	"activated" BOOL NOT NULL,
	"activated_at" BIGINT NOT NULL,
	"email" VARCHAR(50) NOT NULL,
	"fname" VARCHAR(50) NOT NULL,
	"lname" VARCHAR(50) NOT NULL,
	"password" VARCHAR(255) NOT NULL,
	"salt" VARCHAR(50) NOT NULL,
	"username" VARCHAR(15) NOT NULL
);
CREATE TABLE "ema" (
	id SERIAL PRIMARY KEY, 
	ticker_id VARCHAR(255),
	"timestamp" BIGINT NOT NULL,
	"value" DECIMAL NOT NULL,
	FOREIGN KEY (ticker_id) REFERENCES "ticker" (ticker)
);
CREATE TABLE "macd" (
	id SERIAL PRIMARY KEY, 
	ticker_id VARCHAR(255),
	"timestamp" BIGINT NOT NULL,
	"value" DECIMAL NOT NULL,
	FOREIGN KEY (ticker_id) REFERENCES "ticker" (ticker)
);
CREATE TABLE "meal" (
	id SERIAL PRIMARY KEY, 
	food_id INT NOT NULL,
	created_at BIGINT DEFAULT extract(epoch from now()) NOT NULL,
	updated_at BIGINT DEFAULT extract(epoch from now()) NOT NULL,
	"name" VARCHAR(16) NOT NULL,
	FOREIGN KEY (food_id) REFERENCES "food" (id)
);
CREATE TABLE "ohlc_bar" (
	id SERIAL PRIMARY KEY, 
	ticker_id VARCHAR(255),
	created_at BIGINT DEFAULT extract(epoch from now()) NOT NULL,
	updated_at BIGINT DEFAULT extract(epoch from now()) NOT NULL,
	"c" DECIMAL NOT NULL,
	"h" DECIMAL NOT NULL,
	"l" DECIMAL NOT NULL,
	"n" INT NOT NULL,
	"o" DECIMAL NOT NULL,
	"t" BIGINT NOT NULL,
	"v" BIGINT NOT NULL,
	"vw" DECIMAL NOT NULL,
	FOREIGN KEY (ticker_id) REFERENCES "ticker" (ticker)
);
CREATE TABLE "portfolio" (
	id SERIAL PRIMARY KEY, 
	user_id INT,
	created_at BIGINT DEFAULT extract(epoch from now()) NOT NULL,
	updated_at BIGINT DEFAULT extract(epoch from now()) NOT NULL,
	"name" VARCHAR(16) NOT NULL,
	FOREIGN KEY (user_id) REFERENCES "user" (id)
);
CREATE TABLE "rsi" (
	id SERIAL PRIMARY KEY, 
	ticker_id VARCHAR(255),
	"timestamp" BIGINT NOT NULL,
	"value" DECIMAL NOT NULL,
	FOREIGN KEY (ticker_id) REFERENCES "ticker" (ticker)
);
CREATE TABLE "sma" (
	id SERIAL PRIMARY KEY, 
	ticker_id VARCHAR(255),
	"timestamp" BIGINT NOT NULL,
	"value" DECIMAL NOT NULL,
	FOREIGN KEY (ticker_id) REFERENCES "ticker" (ticker)
);
CREATE TABLE "todo" (
	id SERIAL PRIMARY KEY, 
	user_id INT,
	created_at BIGINT DEFAULT extract(epoch from now()) NOT NULL,
	updated_at BIGINT DEFAULT extract(epoch from now()) NOT NULL,
	"name" VARCHAR(16) NOT NULL,
	FOREIGN KEY (user_id) REFERENCES "user" (id)
);
CREATE TABLE "transaction" (
	id SERIAL PRIMARY KEY, 
	user_id INT,
	created_at BIGINT DEFAULT extract(epoch from now()) NOT NULL,
	updated_at BIGINT DEFAULT extract(epoch from now()) NOT NULL,
	"name" VARCHAR(16) NOT NULL,
	"price" INT NOT NULL,
	FOREIGN KEY (user_id) REFERENCES "user" (id)
);
CREATE INDEX idx_transaction_price ON "transaction" (price);
CREATE TABLE "meal_food_map" (
	meal_id INT NOT NULL,
	food_id INT NOT NULL,
	PRIMARY KEY(meal_id, food_id),
	FOREIGN KEY(meal_id) REFERENCES "meal" (id),
	FOREIGN KEY(food_id) REFERENCES "food" (id)
);

