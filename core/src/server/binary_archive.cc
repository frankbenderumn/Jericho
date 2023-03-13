/** STUB: do send before receive for fault tolerance? */
// int bytes_received = 0;
// int content_bytes = 0;
// long bytes_needed = 0;
bool all_read = false;
// bool headers_found = false;
int r = 0;

Request* req;
// receives bytes from client and asserts against request limit
// while (!all_read) { 
    // r = SSL_read(client->ssl, client->request + client->received, MAX_REQUEST_SIZE - client->received); 
    // if (r > 0) {


// my attempt		
    BBLU("Ah fuck. Here we go again!\n");
    req = new Request(client, MAX_REQUEST_SIZE);

// 	BYEL("LETTS GET IT STARTED!\n");
// 	BYEL("%s\n", req->request.c_str());
// 	BYEL("===============================\n");

r = req->client->received;

// 	} else {
// 		all_read = true;
// 	}
// }
// 	bytes_received += r;

// 	if (r > 0) {
// 	BWHI("REWQQQQQQ\n");
// 	BWHI("Request: %s\n", client->request);

// 	if (!headers_found) {
// 		char* p = strstr(client->request, "\r\n\r\n");
// 		if (p != NULL) {
// 			size_t len = p - &client->request[0];
// 			YEL("HEADERS LEN IS: %li\n", len + 4);
// 			size_t orig = len + 4;
// 			size_t beast = client->received;
// 			YEL("CLIENT RECEIVED: %li\n", (long)client->received);
// 			char buf[len + 5];
// 			strncpy(buf, client->request, len + 4);
// 			buf[len + 5] = 0;
// 			char view[200];
// 			strncpy(view, client->request, 199);
// 			view[200] = '\0';
// 			// YEL("BUF IS: %s\n", buf);
// 			// YEL("VIEW IS: %s\n", view);
// 			char* p2 = strstr(buf, "Content-Length: ");
// 			if (p2 != NULL) {
// 				p2 = p2 + strlen("Content-Length: ");
// 				char* lenEnd = strstr(p2, "\r\n");
// 				if (lenEnd == NULL) { BRED("\\r\\n not found\n"); }
// 				size_t sz = lenEnd - p2;
// 				YEL("Content length has %li digits\n", sz);
// 				char clen[sz + 1];
// 				std::string k(p2);
// 				std::string n;
// 				int i = 0;
// 				while (i < sz) {
// 					n += k[i];
// 					i++;
// 				}
// 				YEL("5\n");
// 				strncpy(clen, p2, sz);
// 				clen[sz + 1] = '\0';
// 				YEL("LEN IS: %s\n", clen);
// 				char* pEnd;
// 				long length = strtol(clen, &pEnd, 10);
// 				long length2 = std::stol(n);
// 				YEL("LEN IS: %li\n", length);
// 				long total = length + len;
// 				YEL("BYTE TOTAL SHOULD BE: %li\n", total);
// 				headers_found = true;
// 				bytes_needed = length2;
// 				content_bytes = beast - orig;
// 				BLU("server.cc: Bytes received: %li, Bytes needed: %li\n", (long)content_bytes, bytes_needed);
// 			} else {
// 				BRED("Content-Length not specified\n");
// 				headers_found = true;
// 				break;
// 			}						
// 		} else {
// 			// BRED("Not an HTTP Protocol (\\r\\n\\r\\n not found)\n");
// 			// BRED("%200.s\n", client->request);
// 			// r = 0;
// 			// break;
// 		}
// 	} else {
// 		content_bytes += r;
// 	}

// 	}
    
// 	if ((long)content_bytes >= bytes_needed) {
// 		all_read = true;
// 	}
// 	if (bytes_needed == 0) {
// 		r = 0;
// 		break;
// 	}
    
// 	// BMAG("Bytes received: %li\n", (long)bytes_received);
// }