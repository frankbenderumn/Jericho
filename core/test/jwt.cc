#include "crypt/jwt.h"
#include "util/clock.h"

// SHA-256 code
    // unsigned char hash[SHA256_DIGEST_LENGTH];
    // EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
    // EVP_DigestInit(mdctx, EVP_sha256());
    // EVP_DigestUpdate(mdctx, header_encode.c_str(), header_encode.size());
    // EVP_DigestFinal(mdctx, hash, NULL);
    // EVP_MD_CTX_free(mdctx);
    // byte_t sha_hash;
    // for (size_t i = 0; i < SHA256_DIGEST_LENGTH; i++) {
    //     sha_hash.push_back(hash[i]);
    // }

    // BBLU("SHA HASH: %s\n", (str_t)sha_hash.data());

namespace jericho {

    namespace jwt {
        std::string key_dir = "./config/keys/";
        std::string pub_suffix = ".pub";
        std::string priv_prefix = ".priv";

        // bool generate(std::string& jwt_token, std::string& payload, const std::string& session) {
            
        // }

    }

}

std::string PUBLIC_DIRECTORY = "./public/frontend/";

int main(int argc, char* argv[]) {
    PrizmInit("JWT Authentication");

    // policy implementation

    picojson::object o;
    o["username"] = picojson::value("jonsnow1234");
    o["role"] = picojson::value("user");
    auto now = Clock::now_chrono();
    now += Min(15);
    time_t time = Clock::sec(now);
    std::string time_str = std::to_string(time);
    o["expiration"] = picojson::value(time_str.c_str()); 
    std::string generated_payload = picojson::value(o).serialize();
    std::string jwt_token;

    // mechanism implementation
    std::string received_payload;

    TEST(JWT, GenericAsym) {

    if (!jcrypt::jwt::generate(jwt_token, generated_payload, "./config/keys/priv__pokemon.pem", JWT_RSA)) {
        BRED("Failed to generate JWT Token!\n");
        return 1;
    }

    // BYEL("Generated JWT Token is:\n%s\n", jwt_token.c_str());

    bool match = jcrypt::jwt::verify(jwt_token, received_payload, "./config/keys/pub__pokemon.pem", "./config/keys/priv__pokemon.pem", 1, JWT_RSA);

    // if (match) {
    //     BGRE("MATCH FOUND\n");
    // } else {
    //     BRED("MATCH NOT FOUND\n");
    // }

    // BMAG("Payload final:\n%s\n", received_payload.c_str());

    }

    TEST(JWT, GenericSym) {

    std::string secret = "I am a secret!";

    if (!jcrypt::jwt::generate(jwt_token, generated_payload, secret.c_str(), JWT_HMAC)) {
        BRED("Failed to generate JWT Token!\n");
        return 1;
    }

    // BYEL("Generated JWT Token is:\n%s\n", jwt_token.c_str());

    bool match = jcrypt::jwt::verify(jwt_token, received_payload, secret.c_str(), NULL, 1, JWT_HMAC);

    // if (match) {
    //     BGRE("MATCH FOUND\n");
    // } else {
    //     BRED("MATCH NOT FOUND\n");
    // }

    // BMAG("Payload final:\n%s\n", received_payload.c_str());

    }

    // policy implementation

    picojson::value payload_json;

    if (JFS::parseJson(payload_json, received_payload.c_str()) < 0) {
        BRED("Failed to parse payload json!\n");
    }

    std::string username = payload_json.get<picojson::object>()["username"].get<std::string>();
    std::string role = payload_json.get<picojson::object>()["role"].get<std::string>();
    std::string timestamp = payload_json.get<picojson::object>()["expiration"].get<std::string>();

    // BBLU("Username:    %s\n", username.c_str());
    // BBLU("Role:        %s\n", role.c_str());
    // BBLU("Timestamp:   %s\n", timestamp.c_str());

    long long time_conv = std::stoll(timestamp);
    std::string date = Clock::prettyDate(time_conv);

    // BBLU("Pretty Date: %s\n", date.c_str());

    // Prizm Updates needed after MVP
    // add a print statement after TEST macro to isolate console output information
    // add nested test for integrative testing
    // self-referential types, no separate executables
    TEST(JWT, GenerateAsym) {
        int i = 0;
        while (i < 5) {
            std::string jwt_token;
            picojson::object payload;
            payload["name"] = picojson::value("Franz");
            payload["role"] = picojson::value("user");
            std::pair<std::string, std::string> keys;
            if (!jcrypt::jwt::generate_asym(jwt_token, payload, keys, 15)){
                BRED("Failed to generate JWT token\n");
            }
            picojson::object extracted;
            std::string pub = keys.first;
            std::string session = prizm::tokenize(pub, "__")[0];
            bool verified = jcrypt::jwt::verify_asym(jwt_token, extracted, session);
            ASSERT(verified, true);
            i++;
        }
    }

    TEST(JWT, GenerateSym) {
        int i = 0;
        while (i < 100) {
            std::string jwt_token;
            picojson::object payload;
            payload["name"] = picojson::value("Jeff");
            payload["role"] = picojson::value("user");
            std::string secret;
            if (!jcrypt::jwt::generate_sym(jwt_token, payload, secret, 15)){
                BRED("Failed to generate JWT token\n");
            }
            picojson::object extracted;
            bool verified = jcrypt::jwt::verify_sym(jwt_token, extracted, secret);
            ASSERT(verified, true);
            verified = jcrypt::jwt::verify_sym(jwt_token, extracted, "Monster");
            REFUTE(verified, true);
            i++;
        }
    }


    PrizmResults(); 
    PrizmCleanup();
    return 0;
}