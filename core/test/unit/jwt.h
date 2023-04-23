#include "prizm2/prizm.h"
#include "crypt/jwt.h"

class JWT : public ITest {
    MTRACE(JWT);
  protected:
    void precondition() {
      payload["name"] = picojson::value("Jeff");
      payload["role"] = picojson::value("user");
    }

    void postcondition() {
    }

    picojson::object payload;
    int a = 0;
    int b = 2;
};

PTEST(JWT, Demo) {
    PASSERT(a, 0);
    PBENCH(Okay);
    PSTOP(Okay);
}

PTEST(JWT, Asymmetric) {
    int i = 0;
    while (i < 5) {
        std::string jwt_token;
        std::pair<std::string, std::string> keys;
        if (!jcrypt::jwt::generate_asym(jwt_token, payload, keys, 15)){
            BRED("Failed to generate JWT token\n");
        }
        picojson::object extracted;
        std::string pub = keys.first;
        std::string session = prizm::tokenize(pub, "__")[0];
        bool verified = jcrypt::jwt::verify_asym(jwt_token, extracted, session);
        PASSERT(verified, true);
        i++;
    }
}

PTEST(JWT, Symmetric) {
  int i = 0;
  while (i < 100) {
    std::string jwt_token;
    std::string secret;
    if (!jcrypt::jwt::generate_sym(jwt_token, payload, secret, 15)){
        BRED("Failed to generate JWT token\n");
    }
    // BRED("============================================\n");
    // BRED("SECRET: %s\n", secret.c_str());
    // BRED("============================================\n");
    picojson::object extracted;
    bool verified = jcrypt::jwt::verify_sym(jwt_token, extracted, secret);
    PASSERT(verified, true);
    verified = jcrypt::jwt::verify_sym(jwt_token, extracted, "Monster");
    PREFUTE(verified, true);
    i++;
  }
}