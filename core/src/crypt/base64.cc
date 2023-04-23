#include "crypt/base64.h"

namespace jcrypt {

namespace base64 {

void _to_url(std::string& output) {
	std::replace(output.begin(), output.end(), '+', '-');
	std::replace(output.begin(), output.end(), '/', '_');
	output.erase(std::remove(output.begin(), output.end(), '='), output.end());
}

void _from_url(std::string& output) {
	// Convert URL-safe Base64 to regular Base64
    size_t pos = output.find_first_of("-_");
    while (pos != std::string::npos) {
        output[pos] = pos == '-' ? '+' : '/';
        pos = output.find_first_of("-_", pos + 1);
    }

    // Pad the string with '=' characters if needed
    if (output.length() % 4 != 0) {
        size_t padding = 4 - output.length() % 4;
        output.append(padding, '=');
    }
}

std::string decode_url(const std::string& input) {
	std::string copy = input;
	_from_url(copy);
	return decode(copy);
}

std::string encode_url(const std::string& input) {
	std::string output = encode(input);
	_to_url(output);
	return output;
}

std::string encode(const std::string& input) {
	// Create a memory buffer BIO
	BIO* bio = BIO_new(BIO_s_mem());

	// Create a base64 encoding BIO and push it onto the memory buffer BIO
	BIO* b64 = BIO_new(BIO_f_base64());
	BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
	bio = BIO_push(b64, bio);

	// Write the input data to the base64 encoding BIO
	BIO_write(bio, input.data(), input.length());

	// Flush the base64 encoding BIO to get the encoded data as a string
	BIO_flush(bio);
	char* base64_data;
	long base64_length = BIO_get_mem_data(bio, &base64_data);
	std::string output(base64_data, base64_length);

	// Clean up the BIOs
	BIO_free_all(bio);

	return output;
}

std::string decode(const std::string& input) {
    // Decode the Base64 string
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

    BIO* bmem = BIO_new_mem_buf(input.c_str(), input.length());
    bmem = BIO_push(b64, bmem);

    std::string output(input.length(), '\0');
    int len = BIO_read(bmem, &output[0], output.length());

    output.resize(len);
    BIO_free_all(bmem);

    return output;
}

}

}
