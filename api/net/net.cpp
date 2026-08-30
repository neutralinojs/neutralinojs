#include <string>
#include <vector>
#include <string>
#include <vector>
#include <regex>
#include <algorithm>
#include <cctype>

#include "helpers.h"
#include "errors.h"
#include "api/net/net.h"
#include "api/events/events.h"
#include "lib/json/json.hpp"
#include "lib/httplib/httplib.h"

using namespace std;
using json = nlohmann::json;

namespace net {

namespace controllers {

json request(const json &input) {
    json output;

    if(!helpers::hasRequiredFields(input, {"url"})) {
        output["error"] = errors::makeMissingArgErrorPayload("url");
        return output;
    }

    string url = input["url"].get<string>();
    string method = "GET";
    regex url_regex(R"(^(https?)://([^/]+)(/.*)?$)");
    smatch match;

    if(!regex_match(url, match, url_regex)) {
        output["error"] = errors::makeMissingArgErrorPayload("url");
        return output;
    }

    string protocol = match[1];
    string host = match[2];
    string path = match[3].matched ? match[3].str() : "/";

    httplib::Client cli(protocol + "://" + host);
    httplib::Params params;
    httplib::Headers headers;
    httplib::Result res;
    string body, contentType;

    if(protocol == "https") {
        cli.enable_system_ca(true);
    }
    if(helpers::hasField(input, "timeout")) {
        cli.set_max_timeout(input["timeout"].get<int>());
    }
    if(helpers::hasField(input, "params")) {
        for(auto& [key, value] : input["params"].items()) {
            if(value.is_string()) {
                params.emplace(key, value.get<string>());
            }
        }
    }
    if(helpers::hasField(input, "headers")) {
        for(auto& [key, value] : input["headers"].items()) {
            if(value.is_string()) {
                headers.emplace(key, value.get<string>());
            }
        }
    }
    if(helpers::hasField(input, "auth") && helpers::hasField(input["auth"], "username") && helpers::hasField(input["auth"], "password")) {
        cli.set_basic_auth(input["auth"]["username"].get<string>(), input["auth"]["password"].get<string>());
    }
    if(helpers::hasField(input, "body")) {
        body = input["body"].get<string>();
    }
    if(helpers::hasField(input, "allowRedirects")) {
        cli.set_follow_location(input["allowRedirects"].get<bool>());
        // disabled by default
    }
    if(helpers::hasField(input, "encodePath")) {
        cli.set_path_encode(input["encodePath"].get<bool>());
        // enabled by default
    }
    if(helpers::hasField(input, "keepAlive")) {
        cli.set_keep_alive(input["keepAlive"].get<bool>());
        // disabled by default
    }
    if(helpers::hasField(input, "contentType")) {
        contentType = input["contentType"].get<string>();
        contentType = contentType.empty() ? "application/json" : contentType;
    }
    if(contentType.empty()) {
        auto contentTypeHeader = headers.find("Content-Type");
        if(contentTypeHeader == headers.end()) {
            contentTypeHeader = headers.find("content-type");
        }
        if(contentTypeHeader != headers.end()) {
            contentType = contentTypeHeader->second;
        }
    }

    if(helpers::hasField(input, "method")) {
        method = input["method"].get<string>();
    }
    transform(method.begin(), method.end(), method.begin(), [](unsigned char c) {
        return toupper(c);
    });
    if(method == "GET") {
        res = cli.Get(path, params, headers);
    }
    else if(method == "POST") {
        if(!body.empty()) {
            res = cli.Post(path, headers, body, contentType);
        }
        else if(!params.empty()) {
            res = cli.Post(path, headers, params);
        }
        else{
            res = cli.Post(path, headers);
        }
    }
    else if(method == "HEAD") {
        res = cli.Head(path, headers);
    }
    else if(method == "OPTIONS") {
        res = cli.Options(path, headers);
    }
    else if(method == "PUT") {
        if(!body.empty()) {
            res = cli.Put(path, headers, body, contentType);
        }
        else {
            res = cli.Put(path, headers);
        }
    }
    else if(method == "PATCH") {
        if(!body.empty()) {
            res = cli.Patch(path, headers, body, contentType);
        }
        else {
            res = cli.Patch(path, headers);
        }
    }
    else if(method == "DELETE") {
        if(!body.empty()) {
            res = cli.Delete(path, headers, body, contentType);
        }
        else {
            res = cli.Delete(path, headers);
        }
    }

    if(!res) {
        const auto err = res.error();
        switch(err) {
            case httplib::Error::SSLConnection:
                output["error"] = errors::makeErrorPayload(errors::NE_NW_SSLCONN, to_string(res.ssl_error()));
                break;
            case httplib::Error::SSLLoadingCerts:
                output["error"] = errors::makeErrorPayload(errors::NE_NW_SSLLOAD, to_string(res.ssl_backend_error()));
                break;
            case httplib::Error::SSLServerVerification:
                output["error"] = errors::makeErrorPayload(errors::NE_NW_SSLVERI, to_string(res.ssl_backend_error()));
                break;
            case httplib::Error::SSLServerHostnameVerification:
                output["error"] = errors::makeErrorPayload(errors::NE_NW_SSLHOST, to_string(res.ssl_backend_error()));
                break;
            default:
                output["error"] = errors::makeErrorPayload(errors::NE_NW_HTTPERR, httplib::to_string(err));
        }
    }
    else {
        output["returnValue"] = {
            { "status", res->status },
            { "statusCode", res->status },
            { "statusText", res->reason },
            { "reason", res->reason },
            { "body", res->body },
            { "text", res->body },
            { "headers", res->headers },
            { "cookies", res->get_header_value("Set-Cookie") },
            { "contentType", res->get_header_value("Content-Type") },
            { "location", res->location },
            { "version", res->version }
        };
        output["success"] = true;
    }
    return output;
}

json __requestWithMethod(const json &input, const string &method) {
    json requestInput = input.is_object() ? input : json::object();
    requestInput["method"] = method;
    return request(requestInput);
}

json get(const json &input) {
    return __requestWithMethod(input, "GET");
}

json post(const json &input) {
    return __requestWithMethod(input, "POST");
}

json put(const json &input) {
    return __requestWithMethod(input, "PUT");
}

json del(const json &input) {
    return __requestWithMethod(input, "DELETE");
}

json patch(const json &input) {
    return __requestWithMethod(input, "PATCH");
}

json head(const json &input) {
    return __requestWithMethod(input, "HEAD");
}

json options(const json &input) {
    return __requestWithMethod(input, "OPTIONS");
}

} // namespace controllers
} // namespace net
