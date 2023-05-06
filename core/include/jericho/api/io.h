#ifndef JERICHO_API_INTERNALS_H_
#define JERICHO_API_INTERNALS_H_

#include <fstream>

#include "api/api_helper.h"
#include "api/api.h"

std::string generate_handshake(Request* req, int status) {
    return req->protocol + " " + std::to_string(status) + " " + HttpStatus::status_to_name(status);
}

API(Handshake, {})
    BMAG("API::Internals::Handshake:\n");
    MAG("\t%-16s: %s\n", "Protocol", req->protocol.c_str());
    MAG("\t%-16s: %s\n", "Path", req->path.c_str());
    MAG("\t%-16s: %s\n", "Protocol", req->method.c_str());
    std::string hs = generate_handshake(req, 100);
    BMAG("\tHandshake return: %s\n", hs.c_str());
    return hs;
}

API(WriteFileRPC, {})
    BMAG("API::BifrostTest::WriteFileRPC: Running...\n");
    System* sys = router;
    std::string hostname = req->header("Host");
    SOCKET fd = req->client->socket;
    // MAG("\tAPI::BifrostTest::WriteFileRPC: Socket fd: %i\n", fd);
    std::string job_id = req->header("Job-Id");
    MessageJob* job = sys->bifrost()->job_id(job_id);
    if (job == nullptr) {
        BRED("BREAKPOINT: oooogalaboogala\n");
        // sanity checks on incoming request (check mem, size, etc.)
        job = new MessageJob(job_id, hostname, "write-file");
        std::string content = req->content;
        std::ofstream ofs(content, std::ios::out | std::ios::trunc);
        ofs.close();
        sys->bifrost()->job_id(hostname, job_id, job);
        req->lock();
        return "HTTP/1.1 100 Continue";
    } else {
        BGRE("Job already found... :(\n");
        // exit(1);
    }
    
    std::string jobId = req->header("Job-Id");
    std::string range = req->header("Content-Range");
    std::string disposition = req->header("Content-Disposition");
    std::string file;
    size_t p = disposition.find("filename=");
    if (p != std::string::npos) {
        file = disposition.substr(p+9, disposition.length() - p - 9);
    }
    std::string digest = req->header("Digest");
    std::string checksum;
    if (!digest.empty()) {
        p = digest.find("SHA-256=");
        if (p != std::string::npos) {
            checksum = digest.substr(p+8, digest.length() - p - 8);
            MAG("\tAPI::BifrostTest::WriteFileRPC: Checksum: %s\n", checksum.c_str());
        }
    }
    
    DEBUG("\tAPI::BifrostTest::WriteFileRPC: Content-Disposition: %s\n", disposition.c_str());
    DEBUG("\tAPI::BifrostTest::WriteFileRPC: Content-Range: %s\n", range.c_str());
    DEBUG("\tAPI::BifrostTest::WriteFileRPC: Content-Type: %s\n", req->header("Content-Type").c_str());
    DEBUG("\tAPI::BifrostTest::WriteFileRPC: Content-Length: %s\n", req->header("Content-Length").c_str());
    DEBUG("\tAPI::BifrostTest::WriteFileRPC: Transfer-Encoding: %s\n", req->header("Transfer-Encoding").c_str());
    DEBUG("\tAPI::BifrostTest::WriteFileRPC: Job-Id: %s\n", jobId.c_str());
    DEBUG("\tAPI::BifrostTest::WriteFileRPC: File: %s\n", file.c_str());
    std::string content = req->content;
    DEBUG("\tAPI::BifrostTest::WriteFileRPC: Content: (%li) %s\n", req->content.size(), req->content.c_str());
    if (range.find("/") == std::string::npos) {
        sys->bifrost()->clear_job_id(job_id);
        return "HTTP/1.1 500 Internal Server Error";
    }
    if (range.find("-") == std::string::npos) {
        sys->bifrost()->clear_job_id(job_id);
        return "HTTP/1.1 500 Internal Server Error";
    }
    if (range.find(" ") == std::string::npos) {
        sys->bifrost()->clear_job_id(job_id);
        return "HTTP/1.1 500 Internal Server Error";
    }
    p = range.find(" ");
    std::string range_type;
    range_type = range.substr(0, p);
    range = range.substr(p+1, range.size() - p - 1);
    p = range.find("/");
    std::string length = range.substr(p + 1, range.length() - p - 1);
    range = range.substr(0, p);
    size_t p2 = range.find("-");
    std::string start = range.substr(0, p2);
    std::string end = range.substr(p2+1, range.length() - 1);
    DEBUG("\tAPI::BifrostTest::WriteFileRPC: RangeType : %s\n", range_type.c_str());
    DEBUG("\tAPI::BifrostTest::WriteFileRPC: Start     : %s\n", start.c_str());
    DEBUG("\tAPI::BifrostTest::WriteFileRPC: End       : %s\n", end.c_str());
    DEBUG("\tAPI::BifrostTest::WriteFileRPC: Length    : %s\n", length.c_str());
    std::string response;
    size_t start_sz, end_sz;
    if (!parseSize(start_sz, start)) {
        sys->bifrost()->clear_job_id(job_id);
        return HttpStatus::response(416);
    }
    if (!parseSize(end_sz, end)) {
        sys->bifrost()->clear_job_id(job_id);
        return HttpStatus::response(416);
    }
    if (!JFS::writeOffset(file.c_str(), start_sz, content)) {
        sys->bifrost()->clear_job_id(job_id);
        return HttpStatus::response(503);
    }
    Message* msg = new Message;
    msg->setDirective(216);
    msg->headers["Content-Length"] = std::to_string(start_sz - end_sz);
    msg->headers["Content-Range"] = std::to_string(start_sz) + "-" + std::to_string(end_sz) + "/" + length;
    std::string result = msg->serialize_directive();
    delete msg;
    return result;
}

API(VerifyWriteRPC, {})
    System* sys = router;
    SOCKET fd = req->client->socket;
    // buf->headers["Content-Disposition"] = "attachment; filename=" + moniker;
    // buf->headers["Digest"] = "SHA-256=" + JFS::sha256_checksum(file_path);
    std::string disposition = req->header("Content-Disposition");
    std::string digest = req->header("Digest");
    std::string job_id = req->header("Job-Id");
    size_t p;
    std::string fpath, checksum;
    if ((p = disposition.find("filename=")) != std::string::npos) {
        fpath = disposition.substr(p+9, disposition.size()-p-9);
    } else {
        BRED("API::IO::VerifyWriteRPC: File disposition not found\n");
        sys->bifrost()->clear_job_id(job_id);
        return HttpStatus::response(500);
    }
    if ((p = digest.find("SHA-256=")) != std::string::npos) {
        checksum = digest.substr(p+8, digest.size()-p-8);
    } else {
        BRED("API::IO::VerifyWriteRPC: SHA256 Digest not found\n");
        sys->bifrost()->clear_job_id(job_id);
        return HttpStatus::response(500);
    }

    std::string verification = JFS::sha256_checksum(fpath);

    std::string result = HttpStatus::response(501);
    if (checksum == verification) {
        result = HttpStatus::response(200);
    }

    sys->bifrost()->clear_job_id(job_id);
    return result;
}

API(ReadFileRPC, {})
    size_t chunk_size = 2048;
    BMAG("API::BifrostTest::ReadFileRPC: Running...\n");
    System* sys = router;
    std::string hostname = req->header("Host");
    SOCKET fd = req->client->socket;
    MAG("\tAPI::BifrostTest::ReadFileRPC: Socket fd: %i\n", fd);
    std::string job_id = req->header("Job-Id");
    MessageJob* job = sys->bifrost()->job_id(job_id);
    if (job == nullptr) {
        // sanity checks on incoming request (check mem, size, etc.)
        if (req->path.find("..") != std::string::npos) {
            return HttpStatus::response(404);
        }
        if (req->path.size() > 100) {
            return HttpStatus::response(413);
        }
        if (!JFS::exists(req->content)) {
            return HttpStatus::response(404);
        }
        job = new MessageJob(job_id, hostname, "write-file");
        sys->bifrost()->job_id(hostname, job_id, job);
        Message* msg = new Message;
        msg->setDirective(100);
        size_t sz;
        if (!JFS::size(sz, req->content.c_str())) {
            BRED("ReadFileRPC::JobCreation::Failed to get size of file '%s'\n", req->content.c_str());
            return HttpStatus::response(500);
        }
        msg->headers["Content-Length"] = std::to_string(sz);
        std::string val = msg->serialize_directive();
        BLU("%s\n", val.c_str());
        delete msg;
        return val;
    }

    std::string range_header = req->header("Range");
    std::string type, start_str, end_str, range;
    size_t p;
    if ((p = range_header.find("=")) != std::string::npos) {
        type = range_header.substr(0, p);
        range = range_header.substr(p+1, range_header.size() - p - 1);
    } else {
        sys->bifrost()->clear_job_id(job_id);
        return HttpStatus::response(416);
    }

    if ((p = range.find("-")) != std::string::npos) {
        start_str = range.substr(0, p);
        end_str = range.substr(p+1, range.size() - p - 1);
    } else {
        sys->bifrost()->clear_job_id(job_id);
        return HttpStatus::response(416);
    }

    int start;
    int end;

    if (!parseInt(start, start_str)) {
        sys->bifrost()->clear_job_id(job_id);
        return HttpStatus::response(500);
    }

    if (!parseInt(end, end_str)) {
        sys->bifrost()->clear_job_id(job_id);
        return HttpStatus::response(500);
    }

    std::ifstream file(req->content, std::ios::binary);
    if (!file.is_open()) {
        BRED("\tBifrost::send_file: Error opening file '%s'!\n", req->content.c_str());
        sys->bifrost()->clear_job_id(job_id);
        return "HTTP/1.1 500 Internal Server Error";
    }

    size_t file_sz;
    if (!JFS::size(file_sz, req->content.c_str())) {
        BRED("ReadRPC: Failed to get size of request file '%s'\n", req->content.c_str());
        return HttpStatus::response(500);
    }
    int num_chunks = file_sz / chunk_size;
    Message* msg = new Message;
    msg->setDirective(206);
    msg->headers["Content-Type"] = "application/octet-stream";
    msg->headers["Content-Range"] = "bytes " + start_str + "-" + end_str + "/" + std::to_string(file_sz);
    msg->sent = JFS::readOffset(req->content.c_str(), (size_t)start, (size_t)end);
    msg->headers["Content-Length"] = std::to_string(msg->sent.size());
    std::string result = msg->serialize_directive();
    delete msg;
    return result;
}

API(UpdateFileRPC, {})
    return "COMPLETE";
}

#endif