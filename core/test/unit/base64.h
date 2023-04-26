#include "prizm2/prizm.h"
#include "crypt/base64.h"
#include "crypt/base64_c.h"

class B64Unit : public ITest {
    MTRACE(B64Unit);
  protected:
    void precondition() {
        const char* data = "eseweflkwjelkjlk\0ssjkl\0sldfjlsdkjf\0\0\0\0\0\0\0\0\0\0\0\0\0I AM GROOT";
        const char* data2 = "eseweflkwjelkjlk\0ssjkl\0sldfjlsdkjf\0\0\0\0\0\0\0\0\0\0\0\0\0I AM BINARY BAD BITCH";
        size_t length = 57;
        size_t length2 = 68;
        bin_msg.append(data, length);
        bin_msg2.append(data2, length2);
    }

    void postcondition() {
    }

    std::string msg = "helloworld"; 
    std::string bin_msg;
    std::string bin_msg2;
    std::string enc = "aGVsbG93b3JsZA==";
    std::string enc_url = "aGVsbG93b3JsZA";
    std::string bin_enc = "ZXNld2VmbGt3amVsa2psawBzc2prbABzbGRmamxzZGtqZgAAAAAAAAAAAAAAAABJIEFNIEdST09U";
    std::string bin2_enc_url = "ZXNld2VmbGt3amVsa2psawBzc2prbABzbGRmamxzZGtqZgAAAAAAAAAAAAAAAABJIEFNIEJJTkFSWSBCQUQgQklUQ0g";
};

PTEST(B64Unit, encrypt_str) {
    PBENCH(Encode);
    std::string _enc = jcrypt::base64::encode(msg);
    PSTOP(Encode);
    PASSERT(_enc, enc);
}

PTEST(B64Unit, decrypt_str) {
    PBENCH(Decode);
    std::string dec = jcrypt::base64::decode(enc);
    PASSERT(dec, msg);
    PSTOP(Decode);
}

PTEST(B64Unit, url_encrypt_str) {
    std::string _enc = jcrypt::base64::encode_url(msg);
    PASSERT(_enc, enc_url);
}

PTEST(B64Unit, url_decrypt_str) {
    std::string dec = jcrypt::base64::decode_url(enc_url);
    PASSERT(dec, msg);    
}

PTEST(B64Unit, encrypt_bytes) {
    unsigned char* dest = base64_encode((unsigned char*)msg.data(), msg.length(), NULL);
    int len = strlen((char*)dest);
    BYEL("Len: %i\n", len);
    std::string s((char*)dest, len - 1);
    PASSERT(len - 1, enc.length());
    PASSERT(s.size(), enc.length());
    PASSERT(s, enc);
}

PTEST(B64Unit, size) {
    size_t sz = jcrypt::base64::size(msg);
    PASSERT(sz, enc.size());
}

PTEST(B64Unit, binary_encrypt) {
    std::string _bin_enc = jcrypt::base64::encode(bin_msg);
    PASSERT(_bin_enc, bin_enc);
}

PTEST(B64Unit, binary_decrypt) {
    std::string bin_dec = jcrypt::base64::decode(bin_enc);
    PASSERT(bin_dec, bin_msg);
}

PTEST(B64Unit, binary_url_decrypt) {
    std::string bin_dec = jcrypt::base64::decode_url(bin2_enc_url);
    PASSERT(bin_dec.size(), bin_msg2.size());
    bool bin_match = true;
    if (bin_dec.size() == bin_msg2.size()) {
        for (int i = 0; i < bin_dec.size(); i++) {
            char c = bin_dec[i];
            char d = bin_msg2[i];
            if (c == '\0') { printf("\\0"); } else {
                printf("%c", c);
            }
            printf("\n");
            if (c != d) {
                BRED("Character mismatch on idx: %i\n", i);
                bin_match = false;
            }
        }
    }
    PASSERT(bin_match, true);
}